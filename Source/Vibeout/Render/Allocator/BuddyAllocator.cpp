// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "BuddyAllocator.h"

enum AllocatorBlockState
{
	BLOCK_NONE = 0,
	BLOCK_FREE,
	BLOCK_SPLIT,
	BLOCK_ALLOCATED
};

class AllocatorFreeListItem
{
public:
	uint32 block_index;
	AllocatorFreeListItem* next;
};

class BuddyAllocator
{
public:
	uint32 block_size;
	uint32 level_num;
	AllocatorFreeListItem** free_block_lists;
	uint8* block_states;
	AllocatorFreeListItem* free_items;
};

static inline size_t _align(size_t value, size_t alignment);
static inline uint64 div_ceil(uint64 a, uint64 b);
static inline int32 uint_log2(uint64 x);
static inline int32 uint_log2_ceil(uint64_t x);
static inline AllocatorFreeListItem* allocate_list_item(BuddyAllocator* allocator);
static inline void free_list_item(BuddyAllocator* allocator, AllocatorFreeListItem* item);
static inline void write_free_block_to_list(BuddyAllocator* allocator, uint32 level, uint32 block_index);
static inline uint32 get_level_offset(BuddyAllocator* allocator, uint32 level);
static inline uint32 get_next_level_offset(BuddyAllocator* allocator, uint32 level, uint32 offset);

void subdivide_block(BuddyAllocator* allocator, uint32 src_level, uint32 dst_level);
bool merge_blocks(BuddyAllocator* allocator, uint32 level, uint32 block_index);
void remove_block_from_free_list(BuddyAllocator* allocator, uint32 level, uint32 block_index);


BuddyAllocator* create_buddy_allocator(uint64 capacity, uint64 block_size)
{
	// Capacity must be a multiple of block_size
	VO_ASSERT((capacity % block_size) == 0);
	const uint32 level_num = uint_log2(capacity / block_size) + 1;

	// Capacity must be a *power-of-2* multiple of block size
	VO_ASSERT(capacity == (block_size << (level_num - 1)));

	uint32 block_num = 0;
	for (uint32 i = 0; i < level_num; i++)
		block_num += 1 << ((level_num - 1) - i);

	const size_t alignment = 16;
	const size_t allocator_size = _align(sizeof(BuddyAllocator), alignment);
	const size_t free_list_array_size = _align(level_num * sizeof(AllocatorFreeListItem*), alignment);
	const size_t free_item_buffer_size = _align(block_num * sizeof(AllocatorFreeListItem), alignment);
	const size_t block_state_size = _align(block_num * sizeof(uint8), alignment);
	const size_t total_size = allocator_size + free_list_array_size + free_item_buffer_size + block_state_size;
	char* memory = (char*)_aligned_malloc(total_size, alignment);
	memset(memory, 0, total_size);

	BuddyAllocator* allocator = (BuddyAllocator*)memory;
	allocator->block_size = (uint32)block_size;
	allocator->level_num = level_num;
	allocator->free_block_lists = (AllocatorFreeListItem**)(memory + allocator_size);
	allocator->free_items = (AllocatorFreeListItem*)(memory + allocator_size + free_list_array_size);
	allocator->block_states = (uint8*)(memory + allocator_size + free_list_array_size + free_item_buffer_size);

	for (uint32 i = 0; i < block_num; i++)
		allocator->free_items[i].next = allocator->free_items + i + 1;
	allocator->free_items[block_num - 1].next = nullptr;

	allocator->block_states[block_num - 1] = BLOCK_FREE;
	write_free_block_to_list(allocator, level_num - 1, 0);

	return allocator;
}

BAResult buddy_allocator_allocate(BuddyAllocator* allocator, uint64 size, uint64 alignment, uint64* offset)
{
	const uint32_t level = uint_log2_ceil(div_ceil(size, allocator->block_size));

	// The requested size exceeds the allocator _capacity
	if (level >= allocator->level_num)
		return BA_NOT_ENOUGH_MEMORY;

	// Every block is aligned to its size
	const uint64_t block_size = (uint64_t)(1ull << level) * allocator->block_size;
	const uint64_t alignment_size = block_size % alignment;

	if (alignment_size != 0)
		return BA_INVALID_ALIGNMENT;

	uint32_t i = level;
	for (; i < allocator->level_num && allocator->free_block_lists[i] == NULL; i++)
	{
	}

	if (i == allocator->level_num)
		return BA_NOT_ENOUGH_MEMORY;

	if (i > level)
		subdivide_block(allocator, i, level);

	AllocatorFreeListItem* item = allocator->free_block_lists[level];
	allocator->free_block_lists[level] = item->next;

	const uint32_t level_block_offset = get_level_offset(allocator, level);
	allocator->block_states[level_block_offset + item->block_index] = BLOCK_ALLOCATED;

	*offset = item->block_index * block_size;
	free_list_item(allocator, item);

	return BA_SUCCESS;
}

void buddy_allocator_free(BuddyAllocator* allocator, uint64_t offset, uint64_t size)
{
	const uint32_t level = uint_log2_ceil(div_ceil(size, allocator->block_size));
	const uint64_t block_size = (uint64_t)(1ull << level) * allocator->block_size;
	const uint32_t block_index = (uint32_t)(offset / block_size);

	const uint32_t level_block_offset = get_level_offset(allocator, level);
	VO_ASSERT(allocator->block_states[level_block_offset + block_index] == BLOCK_ALLOCATED);
	allocator->block_states[level_block_offset + block_index] = BLOCK_FREE;

	if (!merge_blocks(allocator, level, block_index))
		write_free_block_to_list(allocator, level, block_index);
}

void destroy_buddy_allocator(BuddyAllocator* allocator)
{
	_aligned_free(allocator);
}

void subdivide_block(BuddyAllocator* allocator, uint32_t src_level, uint32_t dst_level)
{
	VO_ASSERT(dst_level < src_level);

	{
		// Task free block from the list
		AllocatorFreeListItem* item = allocator->free_block_lists[src_level];
		allocator->free_block_lists[src_level] = item->next;

		// Find offsets for states
		const uint32_t previous_level_block_offset = get_level_offset(allocator, src_level - 1);
		const uint32_t current_level_block_offset = get_next_level_offset(allocator, src_level - 1, previous_level_block_offset);

		const uint32_t block_index = item->block_index;

		// Change state of the blocks
		VO_ASSERT(allocator->block_states[current_level_block_offset + block_index] == BLOCK_FREE);
		allocator->block_states[current_level_block_offset + block_index] = BLOCK_SPLIT;
		allocator->block_states[previous_level_block_offset + block_index * 2] = BLOCK_FREE;
		allocator->block_states[previous_level_block_offset + block_index * 2 + 1] = BLOCK_FREE;

		// Add blocks to free list
		AllocatorFreeListItem* item0 = allocate_list_item(allocator);
		AllocatorFreeListItem* item1 = allocate_list_item(allocator);
		item0->block_index = block_index * 2;
		item0->next = item1;
		item1->block_index = block_index * 2 + 1;
		item1->next = allocator->free_block_lists[src_level - 1];
		allocator->free_block_lists[src_level - 1] = item0;
	}

	if (src_level - 1 != dst_level)
		subdivide_block(allocator, src_level - 1, dst_level);
}

bool merge_blocks(BuddyAllocator* allocator, uint32_t level, uint32_t block_index)
{
	if (level == allocator->level_num - 1)
	{
		write_free_block_to_list(allocator, level, block_index);
		return true;
	}

	const uint32_t level_block_offset = get_level_offset(allocator, level);
	const uint32_t buddy_block_index = (block_index % 2) == 0 ? block_index + 1 : block_index - 1;

	if (allocator->block_states[level_block_offset + buddy_block_index] != BLOCK_FREE)
		return false;

	remove_block_from_free_list(allocator, level, buddy_block_index);

	const uint32_t next_level_block_offset = get_level_offset(allocator, level + 1);
	const uint32_t next_level_block_index = block_index / 2;
	VO_ASSERT(allocator->block_states[next_level_block_offset + next_level_block_index] == BLOCK_SPLIT);
	allocator->block_states[next_level_block_offset + next_level_block_index] = BLOCK_FREE;

	if (!merge_blocks(allocator, level + 1, next_level_block_index))
		write_free_block_to_list(allocator, level + 1, next_level_block_index);

	return true;
}

void remove_block_from_free_list(BuddyAllocator* allocator, uint32_t level, uint32_t block_index)
{
	AllocatorFreeListItem** prev_next = &allocator->free_block_lists[level];
	AllocatorFreeListItem* item = allocator->free_block_lists[level];

	VO_ASSERT(item != NULL);
	while (item->block_index != block_index)
	{
		prev_next = &item->next;
		item = item->next;
		VO_ASSERT(item != NULL);
	}

	*prev_next = item->next;
	free_list_item(allocator, item);
}

static inline size_t _align(size_t value, size_t alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

static inline uint64_t div_ceil(uint64_t a, uint64_t b)
{
	return (a + b - 1) / b;
}

static inline int32_t uint_log2(uint64_t x)
{
	uint32_t result = 0;
	while (x >>= 1)
		result++;
	return result;
}

static inline int32_t uint_log2_ceil(uint64_t x)
{
	int32_t log_x = uint_log2(x);

	if (x > (1ull << log_x))
		log_x += 1;

	return log_x;
}

static inline AllocatorFreeListItem* allocate_list_item(BuddyAllocator* allocator)
{
	AllocatorFreeListItem* item = allocator->free_items;
	allocator->free_items = item->next;
	return item;
}

static inline void free_list_item(BuddyAllocator* allocator, AllocatorFreeListItem* item)
{
	item->next = allocator->free_items;
	allocator->free_items = item;
}

static inline void write_free_block_to_list(BuddyAllocator* allocator, uint32_t level, uint32_t block_index)
{
	AllocatorFreeListItem* item = allocate_list_item(allocator);
	item->block_index = block_index;
	item->next = allocator->free_block_lists[level];
	allocator->free_block_lists[level] = item;
}

static inline uint32_t get_level_offset(BuddyAllocator* allocator, uint32_t level)
{
	uint32_t offset = 0;
	for (int32_t i = 0; i < (int32_t)level; i++)
		offset += 1 << ((allocator->level_num - 1) - i);

	return offset;
}

static inline uint32_t get_next_level_offset(BuddyAllocator* allocator, uint32_t prev_level, uint32_t offset)
{
	return offset + (1 << ((allocator->level_num - 1) - prev_level));
}
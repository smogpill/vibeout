// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "DeviceAllocator.h"
#include "BuddyAllocator.h"

// Block size optimized for placement of texture mip chains with up to 4 KB alignment
#define ALLOCATOR_BLOCK_SIZE (44 * 1024)

// Power-of-2 blocks, which is necessary for correct operation of the buddy allocator.
// The _capacity is chosen to allow placement of up to 4096x4096 RGBA8 textures with mip chains.
#define ALLOCATOR_CAPACITY (ALLOCATOR_BLOCK_SIZE * 2048)

class SubAllocator
{
public:
	VkDeviceMemory memory = nullptr;
	BuddyAllocator* buddy_allocator = nullptr;
	size_t memory_used = 0;
	SubAllocator* next = nullptr;
};

class DeviceMemoryAllocator
{
public:
	SubAllocator* sub_allocators[VK_MAX_MEMORY_TYPES] = {};
	VkDevice device = nullptr;
	size_t total_memory_allocated = 0;
	size_t total_memory_used = 0;
};

int create_sub_allocator(DeviceMemoryAllocator* allocator, uint32_t memory_type, uint32_t alignment);

DeviceMemoryAllocator* create_device_memory_allocator(VkDevice device)
{
	DeviceMemoryAllocator* allocator = new DeviceMemoryAllocator();
	allocator->device = device;
	return allocator;
}

DMAResult allocate_device_memory(DeviceMemoryAllocator* allocator, DeviceMemory* device_memory)
{
	const uint32_t memory_type = device_memory->memory_type;
	if (allocator->sub_allocators[memory_type] == NULL)
	{
		if (!create_sub_allocator(allocator, memory_type, (uint32)device_memory->alignment))
			return DMA_NOT_ENOUGH_MEMORY;
	}

	VO_ASSERT(device_memory->size <= ALLOCATOR_CAPACITY);

	BAResult result = BA_NOT_ENOUGH_MEMORY;
	SubAllocator* sub_allocator = allocator->sub_allocators[memory_type];

	while (result != BA_SUCCESS)
	{
		result = buddy_allocator_allocate(sub_allocator->buddy_allocator, device_memory->size,
			device_memory->alignment, &device_memory->memory_offset);

		if (result != BA_SUCCESS)
		{
			if (sub_allocator->next != NULL)
			{
				sub_allocator = sub_allocator->next;
			}
			else
			{
				if (!create_sub_allocator(allocator, memory_type, (uint32)device_memory->alignment))
				{
					device_memory->memory = VK_NULL_HANDLE;
					return DMA_NOT_ENOUGH_MEMORY;
				}

				sub_allocator = allocator->sub_allocators[memory_type];
				//Com_DDPrintf("Created sub-allocator at 0x%p\n", sub_allocator);
			}
		}
	}

	sub_allocator->memory_used += device_memory->size;
	device_memory->memory = sub_allocator->memory;

	//Com_DDPrintf("Allocated %.2f MB, total usage is %.2f in sub-allocator at 0x%p\n", 
	//	(float)device_memory->size / 1048576.f, (float)sub_allocator->memory_usage / 1048576.f , sub_allocator);
	allocator->total_memory_used += device_memory->size;

	return DMA_SUCCESS;
}

void free_device_memory(DeviceMemoryAllocator* allocator, const DeviceMemory* device_memory)
{
	SubAllocator* sub_allocator = allocator->sub_allocators[device_memory->memory_type];

	while (sub_allocator->memory != device_memory->memory)
		sub_allocator = sub_allocator->next;

	buddy_allocator_free(sub_allocator->buddy_allocator, device_memory->memory_offset, device_memory->size);

	sub_allocator->memory_used -= device_memory->size;
	allocator->total_memory_used -= device_memory->size;
}

void destroy_device_memory_allocator(DeviceMemoryAllocator* allocator)
{
	VO_ASSERT(allocator->total_memory_used == 0);

	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
	{
		SubAllocator* sub_allocator = allocator->sub_allocators[i];
		while (sub_allocator != NULL)
		{
			vkFreeMemory(allocator->device, sub_allocator->memory, NULL);
			SubAllocator* next = sub_allocator->next;
			free(sub_allocator);
			sub_allocator = next;
		}
	}

	free(allocator);
}

int create_sub_allocator(DeviceMemoryAllocator* allocator, uint32_t memory_type, uint32_t alignment)
{
	SubAllocator* sub_allocator = (SubAllocator*)malloc(sizeof(SubAllocator));

	uint32_t block_size = ALLOCATOR_BLOCK_SIZE;
	uint32_t capacity = ALLOCATOR_CAPACITY;

	if (alignment > block_size)
	{
		block_size = alignment;
		capacity = 1;

		while (capacity < ALLOCATOR_CAPACITY)
			capacity *= 2;
	}

	VkMemoryAllocateInfo memory_allocate_info = 
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = capacity,
		.memoryTypeIndex = memory_type
	};

	const VkResult result = vkAllocateMemory(allocator->device, &memory_allocate_info, NULL, &sub_allocator->memory);
	if (result != VK_SUCCESS)
		return 0;

	sub_allocator->buddy_allocator = create_buddy_allocator(capacity, block_size);
	sub_allocator->next = allocator->sub_allocators[memory_type];
	allocator->sub_allocators[memory_type] = sub_allocator;

	allocator->total_memory_allocated += capacity;

	return 1;
}

void get_device_malloc_stats(DeviceMemoryAllocator* allocator, size_t* memory_allocated, size_t* memory_used)
{
	if (memory_allocated) *memory_allocated = allocator->total_memory_allocated;
	if (memory_used) *memory_used = allocator->total_memory_used;
}
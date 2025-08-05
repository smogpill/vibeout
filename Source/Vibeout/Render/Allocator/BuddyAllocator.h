// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

enum BAResult
{
	BA_SUCCESS,
	BA_NOT_ENOUGH_MEMORY,
	BA_INVALID_ALIGNMENT
};

class BuddyAllocator;

BuddyAllocator* create_buddy_allocator(uint64_t capacity, uint64_t block_size);
BAResult buddy_allocator_allocate(BuddyAllocator* allocator, uint64_t size, uint64_t alignment, uint64_t* offset);
void buddy_allocator_free(BuddyAllocator* allocator, uint64_t offset, uint64_t size);
void destroy_buddy_allocator(BuddyAllocator* allocator);

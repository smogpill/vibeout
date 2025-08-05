// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

enum DMAResult
{
	DMA_SUCCESS,
	DMA_NOT_ENOUGH_MEMORY
};

struct DeviceMemory
{
	VkDeviceMemory memory;
	uint64_t memory_offset;
	uint64_t size;
	uint64_t alignment;
	uint32_t memory_type;
};

class DeviceMemoryAllocator;

DeviceMemoryAllocator* create_device_memory_allocator(VkDevice device);
DMAResult allocate_device_memory(DeviceMemoryAllocator* allocator, DeviceMemory* device_memory);
void free_device_memory(DeviceMemoryAllocator* allocator, const DeviceMemory* device_memory);
void destroy_device_memory_allocator(DeviceMemoryAllocator* allocator);
void get_device_malloc_stats(DeviceMemoryAllocator* allocator, size_t* memory_allocated, size_t* memory_used);

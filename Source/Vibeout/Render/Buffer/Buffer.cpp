// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Buffer.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Renderer.h"

Buffer::Buffer(const Renderer& renderer, const Setup& setup, bool& result)
	: _renderer(renderer)
{
	result = Init(setup);
}

Buffer::~Buffer()
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	VO_ASSERT(!_mapped);
	if (_buffer)
		vkDestroyBuffer(device, _buffer, nullptr);
	if (_memory)
		vkFreeMemory(device, _memory, nullptr);
}

void* Buffer::Map()
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	VO_ASSERT(!_mapped);
	_mapped = true;
	void* ret = nullptr;
	VO_ASSERT(_memory);
	VO_ASSERT(_size > 0);
	VO_CHECK_VK(vkMapMemory(device, _memory, 0 /*offset*/, _size, 0 /*flags*/, &ret));
	return ret;
}

void Buffer::Unmap()
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	VO_ASSERT(_mapped);
	_mapped = false;
	vkUnmapMemory(device, _memory);
}

VkDeviceAddress Buffer::GetDeviceAddress() const
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	VkBufferDeviceAddressInfo addressInfo = 
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = _buffer
	};

	return vkGetBufferDeviceAddress(device, &addressInfo);
}

void Buffer::Write(const void* buffer, uint32 size)
{
	VO_ASSERT(size <= _size);
	void* ptr = Map();
	VO_ASSERT(ptr);
	memcpy(ptr, buffer, size);
	Unmap();
}

bool Buffer::Init(const Setup& setup)
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	VO_ASSERT(setup._size > 0);
	VkResult result = VK_SUCCESS;

	VkBufferCreateInfo buf_create_info =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = setup._size,
		.usage = setup._usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr
	};

	_size = setup._size;

	VO_TRY_VK(vkCreateBuffer(device, &buf_create_info, nullptr, &_buffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, _buffer, &memReqs);

	uint32 memoryType;
	VO_TRY(_renderer.GetMemoryType(memReqs.memoryTypeBits, setup._memProps, memoryType));

	VkMemoryAllocateInfo mem_alloc_info = 
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = memoryType
	};

	VkMemoryAllocateFlagsInfo mem_alloc_flags =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = (setup._usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) ? VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT : VkMemoryAllocateFlags(0),
		.deviceMask = 0
	};

	mem_alloc_info.pNext = &mem_alloc_flags;

	VO_TRY_VK(vkAllocateMemory(device, &mem_alloc_info, nullptr, &_memory));
	VO_ASSERT(_memory);

	VO_TRY_VK(vkBindBufferMemory(device, _buffer, _memory, 0));

	if (setup._usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		VkBufferDeviceAddressInfo address_info = 
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = _buffer
		};

		_address = vkGetBufferDeviceAddress(device, &address_info);
		VO_ASSERT(_address);
	}
	else
	{
		_address = 0;
	}

	return true;
}

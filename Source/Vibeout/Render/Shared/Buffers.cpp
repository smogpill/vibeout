// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Buffers.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Buffer/Buffer.h"
#include "Vibeout/Render/Shared/Utils.h"

Buffers::Buffers(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

Buffers::~Buffers()
{
	const VkDevice device = _renderer.GetDevice();
	if (_descPool)
		vkDestroyDescriptorPool(device, _descPool, nullptr);
	if (_descSetLayout)
		vkDestroyDescriptorSetLayout(device, _descSetLayout, nullptr);

	for (BufferGroup& buffer : _bufferGroups)
	{
		for (Buffer*& b : buffer._hostBuffers)
			delete b;
		delete buffer._deviceBuffer;
	}
}

bool Buffers::Init()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	const VkPhysicalDevice physicalDevice = _renderer._physicalDevice;
	VO_TRY(physicalDevice);

	VkDescriptorPoolSize pool_sizes[(int)BufferID::END] = {};
	VkDescriptorSetLayoutBinding ubo_layout_bindings[(int)BufferID::END] = {};

	ubo_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_bindings[0].descriptorCount = 1;
	ubo_layout_bindings[0].binding = GLOBAL_UBO_BINDING_IDX;
	ubo_layout_bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = 1;

	ubo_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ubo_layout_bindings[1].descriptorCount = 1;
	ubo_layout_bindings[1].binding = GLOBAL_TLAS_BINDING_IDX;
	ubo_layout_bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_sizes[1].descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo layout_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = std::size(ubo_layout_bindings),
		.pBindings = ubo_layout_bindings,
	};

	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &_descSetLayout));

	const VkMemoryPropertyFlags host_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	const VkMemoryPropertyFlags device_memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	/*
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	size_t alignment = properties.limits.minUniformBufferOffsetAlignment;
	*/

	_bufferGroups[(int)BufferID::UNIFORM]._size = sizeof(GlobalUniformBuffer);
	_bufferGroups[(int)BufferID::TLAS]._size = 1024; // TODO

	for (BufferGroup& bufferGroup : _bufferGroups)
	{
		for (int i = 0; i < maxFramesInFlight; ++i)
		{
			Buffer::Setup setup;
			setup._size = bufferGroup._size;
			setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			setup._memProps = host_memory_flags;
			bool result;
			bufferGroup._hostBuffers[i] = new Buffer(_renderer, setup, result);
			VO_TRY(result);
		}

		{
			Buffer::Setup setup;
			setup._size = bufferGroup._size;
			setup._usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			setup._memProps = device_memory_flags;
			bool result;
			bufferGroup._deviceBuffer = new Buffer(_renderer, setup, result);
			VO_TRY(result);
		}
	}

	VkDescriptorPoolCreateInfo pool_info = {};
	{
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		pool_info.maxSets = 1;
	};

	VO_TRY_VK(vkCreateDescriptorPool(device, &pool_info, nullptr, &_descPool));

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = _descPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &_descSetLayout,
	};

	VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, &_descSet));

	VkDescriptorBufferInfo uniformBufInfo =
	{
		.buffer = _bufferGroups[(int)BufferID::UNIFORM]._deviceBuffer->GetBuffer(),
		.offset = 0,
		.range = _bufferGroups[(int)BufferID::UNIFORM]._size,
	};

	VkDescriptorBufferInfo tlasBufInfo =
	{
		.buffer = _bufferGroups[(int)BufferID::TLAS]._deviceBuffer->GetBuffer(),
		.offset = 0,
		.range = _bufferGroups[(int)BufferID::TLAS]._size,
	};

	VkWriteDescriptorSet writes[(int)BufferID::END] = {};

	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = _descSet;
	writes[0].dstBinding = GLOBAL_UBO_BINDING_IDX;
	writes[0].dstArrayElement = 0;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].descriptorCount = 1;
	writes[0].pBufferInfo = &uniformBufInfo;

	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstSet = _descSet;
	writes[1].dstBinding = GLOBAL_TLAS_BINDING_IDX;
	writes[1].dstArrayElement = 0;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writes[1].descriptorCount = 1;
	writes[1].pBufferInfo = &tlasBufInfo;

	vkUpdateDescriptorSets(device, std::size(writes), writes, 0, nullptr);

	return true;
}

void Buffers::CopyFromStaging(VkCommandBuffer commandBuffer, BufferID bufferID, int nbRegions, const VkBufferCopy* regions)
{
	VO_SCOPE_VK_CMD_LABEL(commandBuffer, "CopyFromStaging");
	BufferGroup& bufferGroup = _bufferGroups[(int)bufferID];
	Buffer* ubo = bufferGroup._hostBuffers[_renderer._currentFrameInFlight];

	vkCmdCopyBuffer(commandBuffer, ubo->GetBuffer(), bufferGroup._deviceBuffer->GetBuffer(), nbRegions, regions);

	for (int regionIdx = 0; regionIdx < nbRegions; ++regionIdx)
	{
		const VkBufferCopy& copy = regions[regionIdx];
		VkBufferMemoryBarrier barrier =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer = bufferGroup._deviceBuffer->GetBuffer(),
			.offset = copy.dstOffset,
			.size = copy.size
		};

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0, 0, nullptr, 1, &barrier, 0, nullptr);
	}
}

void* Buffers::Map(BufferID bufferID)
{
	BufferGroup& bufferGroup = _bufferGroups[(int)bufferID];
	Buffer* hostBuffer = bufferGroup._hostBuffers[_renderer._currentFrameInFlight];
	if (!hostBuffer)
		return nullptr;

	return hostBuffer->Map();
}

void Buffers::Unmap(BufferID bufferID)
{
	BufferGroup& bufferGroup = _bufferGroups[(int)bufferID];
	Buffer* hostBuffer = bufferGroup._hostBuffers[_renderer._currentFrameInFlight];
	if (hostBuffer)
		hostBuffer->Unmap();
}

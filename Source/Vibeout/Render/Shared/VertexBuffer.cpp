// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "VertexBuffer.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Buffer/Buffer.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Shared/Buffers.h"

VertexBuffer::VertexBuffer(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

VertexBuffer::~VertexBuffer()
{
	VkDevice device = _renderer.GetDevice();
	if (desc_pool_vertex_buffer)
		vkDestroyDescriptorPool(device, desc_pool_vertex_buffer, nullptr);
	if (_descSetLayout)
		vkDestroyDescriptorSetLayout(device, _descSetLayout, nullptr);

	delete null_buffer;
	delete _readbackBuffer;
	for (Buffer* buffer : _stagingReadbackBuffers)
		delete buffer;
	delete _toneMapBuffer;
}

bool VertexBuffer::Init()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkDescriptorSetLayoutBinding vbo_layout_bindings[] =
	{
		{
			.binding = READBACK_BUFFER_BINDING_IDX,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = TONE_MAPPING_BUFFER_BINDING_IDX,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		}
	};

	VkDescriptorSetLayoutCreateInfo layout_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = std::size(vbo_layout_bindings),
		.pBindings = vbo_layout_bindings,
	};

	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &_descSetLayout));

	{
		Buffer::Setup setup;
		setup._size = sizeof(ReadbackBuffer);
		setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		_readbackBuffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
	}

	{
		Buffer::Setup setup;
		setup._size = sizeof(ToneMappingBuffer);
		setup._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		_toneMapBuffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
	}

	int stagingReadbackIdx = 0;
	for (Buffer*& buffer : _stagingReadbackBuffers)
	{
		Buffer::Setup setup;
		setup._size = sizeof(ReadbackBuffer);
		setup._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		bool result;
		buffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
		_renderer.SetObjectName(buffer->GetBuffer(), std::format("StagingReadback{}", stagingReadbackIdx++).c_str());
	}

	{
		Buffer::Setup setup;
		setup._size = 4;
		setup._usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		null_buffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
	}

	VkDescriptorPoolSize pool_size =
	{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = std::size(vbo_layout_bindings) + 128,
	};

	VkDescriptorPoolCreateInfo pool_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 2,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
	};

	VO_TRY_VK(vkCreateDescriptorPool(device, &pool_info, nullptr, &desc_pool_vertex_buffer));

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = desc_pool_vertex_buffer,
		.descriptorSetCount = 1,
		.pSetLayouts = &_descSetLayout,
	};

	VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, &_descSet));

	VkDescriptorBufferInfo buf_info =
	{
		.buffer = null_buffer->GetBuffer(),
		.offset = 0,
		.range = null_buffer->GetSize(),
	};

	VkWriteDescriptorSet output_buf_write =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descSet,
		.dstArrayElement = VERTEX_BUFFER_WORLD,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &buf_info,
	};

	output_buf_write.dstBinding = READBACK_BUFFER_BINDING_IDX;
	buf_info.buffer = _readbackBuffer->GetBuffer();
	buf_info.range = sizeof(ReadbackBuffer);
	vkUpdateDescriptorSets(device, 1, &output_buf_write, 0, nullptr);

	output_buf_write.dstBinding = TONE_MAPPING_BUFFER_BINDING_IDX;
	buf_info.buffer = _toneMapBuffer->GetBuffer();
	buf_info.range = sizeof(ToneMappingBuffer);
	vkUpdateDescriptorSets(device, 1, &output_buf_write, 0, nullptr);

	return true;
}

bool VertexBuffer::Readback(ReadbackBuffer& dst)
{
	Buffer* buffer = _stagingReadbackBuffers[_renderer._currentFrameInFlight];
	void* mapped = buffer->Map();
	VO_TRY(mapped);
	memcpy(&dst, mapped, sizeof(ReadbackBuffer));
	buffer->Unmap();
	return true;
}

bool VertexBuffer::InitPipelines()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	VkDescriptorSetLayout descSetLayoutUBO = buffers->GetDescriptorSetLayout();
	VO_TRY(descSetLayoutUBO);
	return true;
}

void VertexBuffer::ShutdownPipelines()
{
}

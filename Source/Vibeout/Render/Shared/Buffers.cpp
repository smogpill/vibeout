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
	if (_mappedStaging)
		_mappedStaging->Unmap();
	const VkDevice device = _renderer.GetDevice();
	if (_descPool)
		vkDestroyDescriptorPool(device, _descPool, nullptr);
	if (_descSetLayout)
		vkDestroyDescriptorSetLayout(device, _descSetLayout, nullptr);

	for (Buffer*& b : _stagingBuffers)
		delete b;
	for (Buffer*& b : _deviceBuffers)
		delete b;
}

bool Buffers::Init()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	const VkPhysicalDevice physicalDevice = _renderer._physicalDevice;
	VO_TRY(physicalDevice);

	VkDescriptorType descriptorTypes[std::size(_deviceBuffers)];
	for (uint i = 0; i < std::size(descriptorTypes); ++i)
		descriptorTypes[i] = i == 0 ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	VkDescriptorSetLayoutBinding layoutBindings[std::size(_deviceBuffers)] = {};
	VkDescriptorPoolSize poolSizes[std::size(_deviceBuffers)] = {};

	for (int bufferIdx = 0; bufferIdx < std::size(_deviceBuffers); ++bufferIdx)
	{
		VkDescriptorSetLayoutBinding& binding = layoutBindings[bufferIdx];
		binding.descriptorType = descriptorTypes[bufferIdx];
		binding.descriptorCount = 1;
		binding.binding = bufferIdx;
		binding.stageFlags = VK_SHADER_STAGE_ALL;

		VkDescriptorPoolSize& poolSize = poolSizes[bufferIdx];
		poolSize.type = binding.descriptorType;
		poolSize.descriptorCount = 1;
	}

	// Layout
	{
		VkDescriptorSetLayoutCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = std::size(layoutBindings),
			.pBindings = layoutBindings,
		};

		VO_TRY_VK(vkCreateDescriptorSetLayout(device, &info, nullptr, &_descSetLayout));
	}

	const VkMemoryPropertyFlags device_memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	// Device buffers
	{
		Buffer::Setup setup;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		// Uniform
		{
			setup._size = sizeof(GlobalUniformBuffer);
			setup._usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bool result;
			_deviceBuffers[(int)BufferID::UNIFORM] = new Buffer(_renderer, setup, result);
			VO_TRY(result);
		}

		// TLAS
		{
			setup._size = 1024; // TODO
			setup._usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bool result;
			_deviceBuffers[(int)BufferID::TLAS] = new Buffer(_renderer, setup, result);
			VO_TRY(result);
		}
	}

	// Staging buffers
	{
		size_t stagingBufferSize = 0;
		for (const Buffer* deviceBuffer : _deviceBuffers)
			stagingBufferSize += deviceBuffer->GetSize();

		Buffer::Setup setup;
		setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		setup._size = stagingBufferSize;

		for (Buffer*& buffer : _stagingBuffers)
		{
			bool result;
			buffer = new Buffer(_renderer, setup, result);
			VO_TRY(result);
		}
	}

	// Pool
	{
		VkDescriptorPoolCreateInfo info = {};
		{
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			info.poolSizeCount = std::size(poolSizes);
			info.pPoolSizes = poolSizes;
			info.maxSets = 1;
		};

		VO_TRY_VK(vkCreateDescriptorPool(device, &info, nullptr, &_descPool));
	}

	// Desc set
	{
		VkDescriptorSetAllocateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = _descPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &_descSetLayout,
		};

		VO_TRY_VK(vkAllocateDescriptorSets(device, &info, &_descSet));
	}

	// Update descriptor sets
	{
		VkDescriptorBufferInfo bufferInfos[(int)BufferID::END] = {};
		for (int bufferIdx = 0; bufferIdx < (int)BufferID::END; ++bufferIdx)
		{
			VkDescriptorBufferInfo& bufferInfo = bufferInfos[bufferIdx];
			const Buffer* buffer = _deviceBuffers[bufferIdx];
			bufferInfo.buffer = buffer->GetBuffer();
			bufferInfo.range = buffer->GetSize();
		}

		VkWriteDescriptorSet writes[(int)BufferID::END] = {};

		for (uint bufferIdx = 0; bufferIdx < std::size(_deviceBuffers); ++bufferIdx)
		{
			writes[bufferIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[bufferIdx].dstSet = _descSet;
			writes[bufferIdx].dstBinding = bufferIdx;
			writes[bufferIdx].dstArrayElement = 0;
			writes[bufferIdx].descriptorType = descriptorTypes[bufferIdx];
			writes[bufferIdx].descriptorCount = 1;
			writes[bufferIdx].pBufferInfo = &bufferInfos[bufferIdx];
		}

		vkUpdateDescriptorSets(device, std::size(writes), writes, 0, nullptr);
	}

	return true;
}

void Buffers::UnmapStagingIfMapped()
{
	if (_mappedStaging)
	{
		_mappedStaging->Unmap();
		_mappedStagingPtr = nullptr;
		_mappedStaging = nullptr;
		_allocatedInStaging = 0;
	}
}

void Buffers::CopyFromStaging(VkCommandBuffer commands)
{
	VO_SCOPE_VK_CMD_LABEL(commands, "CopyFromStaging");
	if (_mappedStaging)
	{
		_mappedStaging->Unmap();

		std::vector<VkBufferCopy> copies;
		copies.reserve(_uploadRequests.size());

		for (int bufferID = 0; bufferID < (int)BufferID::END; ++bufferID)
		{
			copies.clear();
			for (const UploadRequest& request : _uploadRequests)
			{
				if ((int)request._bufferID == bufferID)
				{
					copies.push_back(request._copyVK);
				}
			}

			if (!copies.empty())
			{
				Buffer* deviceBuffer = _deviceBuffers[(int)bufferID];
				vkCmdCopyBuffer(commands, _mappedStaging->GetBuffer(), deviceBuffer->GetBuffer(), (uint32)copies.size(), copies.data());

				for (const VkBufferCopy& copy : copies)
				{
					VkBufferMemoryBarrier barrier =
					{
						.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
						.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.buffer = deviceBuffer->GetBuffer(),
						.offset = copy.dstOffset,
						.size = copy.size
					};

					vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						0, 0, nullptr, 1, &barrier, 0, nullptr);
				}
			}
		}

		_uploadRequests.clear();
		_mappedStagingPtr = nullptr;
		_mappedStaging = nullptr;
		_allocatedInStaging = 0;
	}
}

void* Buffers::AllocateInStaging(BufferID bufferID, uint32 offset, uint32 size)
{
	if (_mappedStaging == nullptr)
	{
		_mappedStaging = _stagingBuffers[_renderer._currentFrameInFlight];
		_mappedStagingPtr = _mappedStaging->Map();
	}
	if (_allocatedInStaging + size < _mappedStaging->GetSize())
	{
		UploadRequest request;
		request._bufferID = bufferID;
		request._copyVK.srcOffset = _allocatedInStaging;
		request._copyVK.size = size;
		request._copyVK.dstOffset = 0;
		_uploadRequests.push_back(request);
		void* buffer = (void*)((intptr_t)_mappedStagingPtr + _allocatedInStaging);
		_allocatedInStaging += size;
		return buffer;
	}
	else
	{
		VO_ASSERT(false);
		return nullptr;
	}
}

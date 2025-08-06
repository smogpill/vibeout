// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "PathTracer.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Shared/Buffers.h"
#include "Vibeout/Render/Shared/Textures.h"
#include "Vibeout/Render/Shared/Shaders.h"
#include "Vibeout/Render/Buffer/Buffer.h"

PathTracer::PathTracer(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

bool PathTracer::Init()
{
	const Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	const Textures* textures = _renderer._textures;
	VO_TRY(textures);
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	/*
	// create descriptor set layout
	VkDescriptorSetLayoutBinding bindings[] = {};

	VkDescriptorSetLayoutCreateInfo layout_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = std::size(bindings),
		.pBindings = bindings
	};
	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &_descriptorSetLayout));
	ATTACH_LABEL_VARIABLE(_descriptorSetLayout, DESCRIPTOR_SET_LAYOUT);

	*/


	VkDescriptorSetLayout desc_set_layouts[] =
	{
		buffers->GetDescriptorSetLayout(),
		textures->GetDescSetLayout()
	};

	/* create pipeline */
	VkPushConstantRange push_constant_range =
	{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = sizeof(PushConstants),
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = std::size(desc_set_layouts),
		.pSetLayouts = desc_set_layouts,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range,
	};

	VO_TRY_VK(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &_pipelineLayout));

	/*
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, s_maxFramesInFlight * std::size(bindings) }
	};

	VkDescriptorPoolCreateInfo pool_create_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = s_maxFramesInFlight,
		.poolSizeCount = std::size(pool_sizes),
		.pPoolSizes = pool_sizes
	};

	VO_TRY_VK(vkCreateDescriptorPool(device, &pool_create_info, nullptr, &_descriptorPool));
	ATTACH_LABEL_VARIABLE(_descriptorPool, DESCRIPTOR_POOL);

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = _descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &_descriptorSetLayout,
	};

	for (int i = 0; i < s_maxFramesInFlight; i++)
	{
		VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, &_descriptorSet[i]));
		ATTACH_LABEL_VARIABLE(_descriptorSet[i], DESCRIPTOR_SET);
	}
	*/
	return true;
}

bool PathTracer::InitPipelines()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkSpecializationMapEntry specEntry =
	{
		.constantID = 0,
		.offset = 0,
		.size = sizeof(uint32_t),
	};

	uint32 numbers[2] = { 0, 1 };

	VkSpecializationInfo specInfo[2] =
	{
		{
			.mapEntryCount = 1,
			.pMapEntries = &specEntry,
			.dataSize = sizeof(uint32),
			.pData = &numbers[0],
		},
		{
			.mapEntryCount = 1,
			.pMapEntries = &specEntry,
			.dataSize = sizeof(uint32),
			.pData = &numbers[1],
		}
	};

	VkComputePipelineCreateInfo computeInfos[(int)PipelineID::END] = {};
	for (VkComputePipelineCreateInfo& info : computeInfos)
	{
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.layout = _pipelineLayout;

		VkPipelineShaderStageCreateInfo& shaderStage = info.stage;
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStage.pName = "main";
	}

	const Shaders* shaders = _renderer._shaders;
	VO_TRY(shaders);

	{
		VkComputePipelineCreateInfo& info = computeInfos[(int)PipelineID::PRIMARY_RAYS];
		info.stage.module = shaders->GetShaderModule("PrimaryRays.comp");
	}
	{
		VkComputePipelineCreateInfo& info = computeInfos[(int)PipelineID::DIRECT_LIGHTING];
		info.stage.module = shaders->GetShaderModule("DirectLighting.comp");
		info.stage.pSpecializationInfo = &specInfo[0];
	}
	{
		VkComputePipelineCreateInfo& info = computeInfos[(int)PipelineID::INDIRECT_LIGHTING_0];
		info.stage.module = shaders->GetShaderModule("IndirectLighting.comp");
		info.stage.pSpecializationInfo = &specInfo[0];
	}
	{
		VkComputePipelineCreateInfo& info = computeInfos[(int)PipelineID::INDIRECT_LIGHTING_1];
		info.stage.module = shaders->GetShaderModule("IndirectLighting.comp");
		info.stage.pSpecializationInfo = &specInfo[1];
	}
	static_assert(std::size(computeInfos) == std::size(_pipelines));
	VO_TRY_VK(vkCreateComputePipelines(device, nullptr, std::size(computeInfos), computeInfos, nullptr, _pipelines));

	return true;
}

void PathTracer::ShutdownPipelines()
{
	VkDevice device = _renderer.GetDevice();
	for (VkPipeline& pipeline : _pipelines)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		pipeline = nullptr;
	}
}

void PathTracer::SetupPipeline(VkCommandBuffer cmd_buf, VkPipelineBindPoint bind_point, PipelineID pipelineID)
{
	VO_SCOPE_VK_CMD_LABEL(cmd_buf, "PT:SetupPipeline");
	const Buffers* buffers = _renderer._buffers;
	VO_ASSERT(buffers);
	const Textures* textures = _renderer._textures;
	VO_ASSERT(textures);
	vkCmdBindPipeline(cmd_buf, bind_point, _pipelines[(int)pipelineID]);
	vkCmdBindDescriptorSets(cmd_buf, bind_point, _pipelineLayout, 0, 1, &buffers->GetDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(cmd_buf, bind_point, _pipelineLayout, 1, 1, &textures->GetCurrentDescSet(), 0, nullptr);
}

void PathTracer::DispatchRays(VkCommandBuffer cmd_buf, PipelineID pipelineID, PushConstants push, uint32_t width, uint32_t height, uint32_t depth)
{
	VO_SCOPE_VK_CMD_LABEL(cmd_buf, "DispatchRays");
	SetupPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineID);
	vkCmdPushConstants(cmd_buf, _pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
	vkCmdDispatch(cmd_buf, (width + 7) / 8, (height + 7) / 8, depth);
}

void PathTracer::TracePrimaryRays(VkCommandBuffer commandBuffer)
{
	VO_SCOPE_VK_CMD_LABEL(commandBuffer, "TracePrimaryRays");
	Textures* textures = _renderer._textures;
	VO_ASSERT(textures);
	const Buffers* buffers = _renderer._buffers;
	VO_ASSERT(buffers);

	// Unsure if required
	{
		Buffer* buffer = buffers->GetDeviceBuffer(Buffers::BufferID::TLAS);
		VkBufferMemoryBarrier barrier = BUFFER_BARRIER();
		barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.buffer = buffer->GetBuffer();
		barrier.size = VK_WHOLE_SIZE;
		QUEUE_BUFFER_BARRIER(commandBuffer, barrier);
	}

	const int frame_idx = _renderer._frameCounter & 1;

	/*
	VkBufferMemoryBarrier barrier = BUFFER_BARRIER();
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.buffer = qvk.buf_readback.buffer;
	barrier.size = VK_WHOLE_SIZE;
	QUEUE_BUFFER_BARRIER(commandBuffer, barrier);
	*/

	//BEGIN_PERF_MARKER(cmd_buf, PROFILER_PRIMARY_RAYS);

	PushConstants push;
	push._bounce = 0;

	DispatchRays(commandBuffer, PipelineID::PRIMARY_RAYS, push, _renderer._extentRender.width / 2, _renderer._extentRender.height, 2);

	//END_PERF_MARKER(cmd_buf, PROFILER_PRIMARY_RAYS);

	textures->QueueImageBarrier(commandBuffer, (ImageID)((int)ImageID::PT_VISBUF_PRIM_A + frame_idx), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, (ImageID)((int)ImageID::PT_VISBUF_BARY_A + frame_idx), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_TRANSPARENT, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_MOTION, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_SHADING_POSITION, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_VIEW_DIRECTION, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_THROUGHPUT, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_BOUNCE_THROUGHPUT, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_GODRAYS_THROUGHPUT_DIST, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, (ImageID)((int)ImageID::PT_BASE_COLOR_A + frame_idx), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, (ImageID)((int)ImageID::PT_METALLIC_A + frame_idx), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, (ImageID)((int)ImageID::PT_CLUSTER_A + frame_idx), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, (ImageID)((int)ImageID::PT_VIEW_DEPTH_A + frame_idx), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, (ImageID)((int)ImageID::PT_NORMAL_A + frame_idx), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, (ImageID)((int)ImageID::ASVGF_RNG_SEED_A + frame_idx), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
}

void PathTracer::TraceLighting(VkCommandBuffer commandBuffer, int nbBounces)
{
	VO_SCOPE_VK_CMD_LABEL(commandBuffer, "TraceLighting");
	Textures* textures = _renderer._textures;
	VO_ASSERT(textures);

	// ???
	if (false)
	{
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_LF_SH, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_LF_COCG, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_HF, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_SPEC, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	}

	{
		VO_SCOPE_VK_CMD_LABEL(commandBuffer, "DirectLighting");
		//pipeline_index_t pipeline = (cvar_pt_caustics->value != 0) ? PIPELINE_DIRECT_LIGHTING_CAUSTICS : PIPELINE_DIRECT_LIGHTING;
		const PipelineID pipeline = PipelineID::DIRECT_LIGHTING;

		PushConstants push;
		push._bounce = 0;

		DispatchRays(commandBuffer, pipeline, push, _renderer._extentRender.width / 2, _renderer._extentRender.height, 2);
	}

	textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_LF_SH, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_LF_COCG, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_HF, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_SPEC, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);

#if 0
	{
		VkBufferMemoryBarrier barrier = BUFFER_BARRIER();
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.buffer = qvk.buf_readback.buffer;
		barrier.size = VK_WHOLE_SIZE;
		QUEUE_BUFFER_BARRIER(commandBuffer, barrier);
	}
#endif


	VO_ASSERT(nbBounces <= 2);
	for (int bounceIdx = 0; bounceIdx < nbBounces; ++bounceIdx)
	{
		{
			VO_SCOPE_VK_CMD_LABEL(commandBuffer, std::format("IndirectLighting{}", bounceIdx).c_str());
			PipelineID pipeline = (PipelineID)((int)PipelineID::INDIRECT_LIGHTING_0 + bounceIdx);

			PushConstants push;
			push._bounce = 0;

			DispatchRays(commandBuffer, pipeline, push, _renderer._extentRender.width / 2, _renderer._extentRender.height, 2);
		}

		textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_LF_SH, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_LF_COCG, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_HF, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_COLOR_SPEC, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_BOUNCE_THROUGHPUT, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
		textures->QueueImageBarrier(commandBuffer, ImageID::PT_SHADING_POSITION, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
	}
}

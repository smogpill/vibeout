// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Denoiser.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Shared/Buffers.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Shared/Textures.h"
#include "Vibeout/Render/Shared/Shaders.h"

#define BARRIER_COMPUTE(cmd_buf, img) \
	do { \
		VkImageSubresourceRange subresource_range = { \
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT, \
			.baseMipLevel   = 0, \
			.levelCount     = 1, \
			.baseArrayLayer = 0, \
			.layerCount     = 1 \
		}; \
		VkImageMemoryBarrier barrier = IMAGE_BARRIER(); \
		barrier.image = img; \
		barrier.subresourceRange = subresource_range; \
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; \
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; \
		barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL; \
		barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; \
		QUEUE_IMAGE_BARRIER(cmd_buf, barrier); \
	} while(0)

Denoiser::Denoiser(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

Denoiser::~Denoiser()
{
	VkDevice device = _renderer.GetDevice();
	if (_pipeline_layout_atrous)
		vkDestroyPipelineLayout(device, _pipeline_layout_atrous, nullptr);
	if (_pipeline_layout_general)
		vkDestroyPipelineLayout(device, _pipeline_layout_general, nullptr);
	if (_pipeline_layout_taa)
		vkDestroyPipelineLayout(device, _pipeline_layout_taa, nullptr);
}

bool Denoiser::InitPipelines()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkSpecializationMapEntry specEntries[] =
	{
		{.constantID = 0, .offset = 0, .size = sizeof(uint32) }
	};

	uint32 spec_data[] =
	{
		0,
		1,
		2,
		3
	};

	VkSpecializationInfo specInfo[] =
	{
		{.mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32), .pData = &spec_data[0] },
		{.mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32), .pData = &spec_data[1] },
		{.mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32), .pData = &spec_data[2] },
		{.mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32), .pData = &spec_data[3] },
	};

	VkComputePipelineCreateInfo pipeline_info[(int)PipelineID::END] = {};

	for (VkComputePipelineCreateInfo& info : pipeline_info)
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::GRADIENT_IMAGE];
		info.stage = SHADER_STAGE("AsvgfGradientImg.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _pipeline_layout_general;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::GRADIENT_ATROUS];
		info.stage = SHADER_STAGE("AsvgfGradientAtrous.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _pipeline_layout_atrous;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::GRADIENT_REPROJECT];
		info.stage = SHADER_STAGE("AsvgfGradientReproject.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _pipeline_layout_general;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::TEMPORAL];
		info.stage = SHADER_STAGE("AsvgfTemporal.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _pipeline_layout_general;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::ATROUS_LF];
		info.stage = SHADER_STAGE("AsvgfLf.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _pipeline_layout_atrous;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::ATROUS_ITER_0];
		info.stage = SHADER_STAGE_SPEC("AsvgfAtrous.comp", VK_SHADER_STAGE_COMPUTE_BIT, &specInfo[0]);
		info.layout = _pipeline_layout_general;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::ATROUS_ITER_1];
		info.stage = SHADER_STAGE_SPEC("AsvgfAtrous.comp", VK_SHADER_STAGE_COMPUTE_BIT, &specInfo[1]);
		info.layout = _pipeline_layout_general;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::ATROUS_ITER_2];
		info.stage = SHADER_STAGE_SPEC("AsvgfAtrous.comp", VK_SHADER_STAGE_COMPUTE_BIT, &specInfo[2]);
		info.layout = _pipeline_layout_general;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::ATROUS_ITER_3];
		info.stage = SHADER_STAGE_SPEC("AsvgfAtrous.comp", VK_SHADER_STAGE_COMPUTE_BIT, &specInfo[3]);
		info.layout = _pipeline_layout_general;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::CHECKERBOARD_INTERLEAVE];
		info.stage = SHADER_STAGE("CheckerboardInterleave.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _pipeline_layout_general;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::TAAU];
		info.stage = SHADER_STAGE("AsvgfTaau.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _pipeline_layout_taa;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::COMPOSITING];
		info.stage = SHADER_STAGE("Compositing.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _pipeline_layout_general;
	}

	VO_TRY_VK(vkCreateComputePipelines(device, 0, std::size(pipeline_info), pipeline_info, 0, _pipelines));

	return true;
}

void Denoiser::ShutdownPipelines()
{
	VkDevice device = _renderer.GetDevice();
	for (VkPipeline& pipeline : _pipelines)
		if (pipeline)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
			pipeline = nullptr;
		}
}

bool Denoiser::Init()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	const Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	const Textures* textures = _renderer._textures;
	VO_TRY(textures);
	VkDescriptorSetLayout desc_set_layouts[] =
	{
		buffers->GetDescriptorSetLayout(),
		textures->GetDescSetLayout()
	};

	VkPushConstantRange push_constant_range_atrous =
	{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = sizeof(uint32)
	};

	// Pipeline layout: Atrous
	{
		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = std::size(desc_set_layouts);
		info.pSetLayouts = desc_set_layouts;
		info.pushConstantRangeCount = 1;
		info.pPushConstantRanges = &push_constant_range_atrous;
		VO_TRY_VK(vkCreatePipelineLayout(device, &info, nullptr, &_pipeline_layout_atrous));
		_renderer.SetObjectName(_pipeline_layout_atrous, "Atrous");
	}

	// Pipeline layout: General
	{
		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = std::size(desc_set_layouts);
		info.pSetLayouts = desc_set_layouts;
		VO_TRY_VK(vkCreatePipelineLayout(device, &info, nullptr, &_pipeline_layout_general));
		_renderer.SetObjectName(_pipeline_layout_general, "General");
	}

	// Pipeline layout: TAA
	{
		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = std::size(desc_set_layouts);
		info.pSetLayouts = desc_set_layouts;
		VO_TRY_VK(vkCreatePipelineLayout(device, &info, nullptr, &_pipeline_layout_taa));
		_renderer.SetObjectName(_pipeline_layout_taa, "TAA");
	}

	return true;
}

bool Denoiser::ASVGF_GradientReproject(VkCommandBuffer commands)
{
	VO_SCOPE_VK_CMD_LABEL(commands, "RenderDenoiser::ASVGF_GradientReproject");
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);

	VkDescriptorSet desc_sets[] =
	{
		buffers->GetDescriptorSet(),
		textures->GetCurrentDescSet()
	};

	int current_sample_pos_image = (int)ImageID::ASVGF_GRAD_SMPL_POS_A + (_renderer._frameCounter & 1);

	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::GRADIENT_REPROJECT]);
	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
		_pipeline_layout_general, 0, std::size(desc_sets), desc_sets, 0, 0);

	uint32 group_size_pixels = 24; // matches GROUP_SIZE_PIXELS in asvgf_gradient_reproject.comp
	vkCmdDispatch(commands,
		(_renderer._extentRender.width + group_size_pixels - 1) / group_size_pixels,
		(_renderer._extentRender.height + group_size_pixels - 1) / group_size_pixels,
		1);

	BARRIER_COMPUTE(commands, textures->GetImage((ImageID)((int)ImageID::ASVGF_RNG_SEED_A + (_renderer._frameCounter & 1))));
	BARRIER_COMPUTE(commands, textures->GetImage((ImageID)current_sample_pos_image));

	return true;
}

bool Denoiser::ASVGF_Filter(VkCommandBuffer commands, bool enable_lf)
{
	VO_SCOPE_VK_CMD_LABEL(commands, "RenderDenoiser::ASVGF_Filter");
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);

	VkDescriptorSet desc_sets[] =
	{
		buffers->GetDescriptorSet(),
		textures->GetCurrentDescSet()
	};
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::PT_COLOR_LF_SH));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::PT_COLOR_LF_COCG));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::PT_COLOR_HF));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::PT_COLOR_SPEC));

	/* create gradient image */
	{
		VO_SCOPE_VK_CMD_LABEL(commands, "GradientImage");
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::GRADIENT_IMAGE]);
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
			_pipeline_layout_general, 0, std::size(desc_sets), desc_sets, 0, 0);
		vkCmdDispatch(commands,
			(_renderer._extentRender.width / GRAD_DWN + 15) / 16,
			(_renderer._extentRender.height / GRAD_DWN + 15) / 16,
			1);
	}

	// XXX BARRIERS!!!
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_GRAD_LF_PING));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_GRAD_HF_SPEC_PING));

	{
		VO_SCOPE_VK_CMD_LABEL(commands, "GradientAtrous");
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::GRADIENT_ATROUS]);
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
			_pipeline_layout_atrous, 0, std::size(desc_sets), desc_sets, 0, 0);

		/* reconstruct gradient image */
		const int num_atrous_iterations_gradient = 7;
		for (int i = 0; i < num_atrous_iterations_gradient; i++)
		{
			uint32 push_constants[1] = { (uint32)i };

			vkCmdPushConstants(commands, _pipeline_layout_atrous,
				VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push_constants), push_constants);

			vkCmdDispatch(commands,
				(_renderer._extentRender.width / GRAD_DWN + 15) / 16,
				(_renderer._extentRender.height / GRAD_DWN + 15) / 16,
				1);
			BARRIER_COMPUTE(commands, textures->GetImage((ImageID)((int)ImageID::ASVGF_GRAD_LF_PING + !(i & 1))));
			BARRIER_COMPUTE(commands, textures->GetImage((ImageID)((int)ImageID::ASVGF_GRAD_HF_SPEC_PING + !(i & 1))));
		}
	}

	/* temporal accumulation / filtering */
	{
		VO_SCOPE_VK_CMD_LABEL(commands, "Temporal");
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::TEMPORAL]);
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
			_pipeline_layout_general, 0, std::size(desc_sets), desc_sets, 0, 0);
		vkCmdDispatch(commands,
			(_renderer._extentRender.width + 14) / 15,
			(_renderer._extentRender.height + 14) / 15,
			1);
	}

	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_LF_SH));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_LF_COCG));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_HF));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_MOMENTS));
	BARRIER_COMPUTE(commands, textures->GetImage((ImageID)((int)ImageID::ASVGF_FILTERED_SPEC_A + (_renderer._frameCounter & 1))));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_HIST_MOMENTS_HF_A));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_HIST_MOMENTS_HF_B));

	/* spatial reconstruction filtering */
	const int num_atrous_iterations = 4;
	for (int i = 0; i < num_atrous_iterations; i++)
	{
		if (enable_lf)
		{
			VO_SCOPE_VK_CMD_LABEL(commands, "AtrousLF");
			uint32 push_constants[1] = { (uint32_t)i };

			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::ATROUS_LF]);

			vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
				_pipeline_layout_atrous, 0, std::size(desc_sets), desc_sets, 0, 0);

			vkCmdPushConstants(commands, _pipeline_layout_atrous,
				VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push_constants), push_constants);

			vkCmdDispatch(commands,
				(_renderer._extentRender.width / GRAD_DWN + 15) / 16,
				(_renderer._extentRender.height / GRAD_DWN + 15) / 16,
				1);

			if (i == num_atrous_iterations - 1)
			{
				BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_LF_SH));
				BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_LF_COCG));
			}
		}

		{
			VO_SCOPE_VK_CMD_LABEL(commands, "AtrousIter");

			int specialization = (int)PipelineID::ATROUS_ITER_0 + i;

			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[specialization]);

			vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
				_pipeline_layout_general, 0, std::size(desc_sets), desc_sets, 0, 0);

			vkCmdDispatch(commands,
				(_renderer._extentRender.width + 15) / 16,
				(_renderer._extentRender.height + 15) / 16,
				1);
		}

		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_LF_SH));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_LF_COCG));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_HF));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PING_MOMENTS));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PONG_LF_SH));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PONG_LF_COCG));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PONG_HF));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_ATROUS_PONG_MOMENTS));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_HIST_COLOR_HF));
		BARRIER_COMPUTE(commands, textures->GetImage((ImageID)((int)ImageID::ASVGF_ATROUS_PING_LF_SH + !(i & 1))));
		BARRIER_COMPUTE(commands, textures->GetImage((ImageID)((int)ImageID::ASVGF_ATROUS_PING_LF_COCG + !(i & 1))));
		BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_COLOR));
	}

	return true;
}

bool Denoiser::Compositing(VkCommandBuffer commands)
{
	VO_SCOPE_VK_CMD_LABEL(commands, "RenderDenoiser::Compositing");
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);

	VkDescriptorSet desc_sets[] = {
		buffers->GetDescriptorSet(),
		textures->GetCurrentDescSet()
	};
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::PT_COLOR_LF_SH));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::PT_COLOR_LF_COCG));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::PT_COLOR_HF));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::PT_COLOR_SPEC));

	/* create gradient image */
	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::COMPOSITING]);
	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
		_pipeline_layout_general, 0, std::size(desc_sets), desc_sets, 0, 0);

	vkCmdDispatch(commands,
		(_renderer._extentRender.width + 15) / 16,
		(_renderer._extentRender.height + 15) / 16,
		1);

	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_COLOR));

	return true;
}

bool Denoiser::Interleave(VkCommandBuffer commands)
{
	VO_SCOPE_VK_CMD_LABEL(commands, "RenderDenoiser::Interleave");
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);

	VkDescriptorSet desc_sets[] =
	{
		buffers->GetDescriptorSet(),
		textures->GetCurrentDescSet()
	};

	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::CHECKERBOARD_INTERLEAVE]);
	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
		_pipeline_layout_general, 0, std::size(desc_sets), desc_sets, 0, 0);

	// dispatch using the image dimensions, not render dimensions - to clear the unused area with black color
	vkCmdDispatch(commands,
		(_renderer._extentScreenImages.width + 15) / 16,
		(_renderer._extentScreenImages.height + 15) / 16,
		1);

	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::FLAT_COLOR));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::FLAT_MOTION));

	return true;
}

bool Denoiser::TAA(VkCommandBuffer commands)
{
	VO_SCOPE_VK_CMD_LABEL(commands, "RenderDenoiser::TAA");
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);

	VkDescriptorSet desc_sets[] =
	{
		buffers->GetDescriptorSet(),
		textures->GetCurrentDescSet()
	};

	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::TAAU]);

	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
		_pipeline_layout_taa, 0, std::size(desc_sets), desc_sets, 0, 0);

	VkExtent2D dispatchSize = _renderer._extentTAAOutput;

	if (dispatchSize.width < _renderer._extentTAAImages.width)
		dispatchSize.width += 8;

	if (dispatchSize.height < _renderer._extentTAAImages.height)
		dispatchSize.height += 8;

	vkCmdDispatch(commands,
		(dispatchSize.width + 15) / 16,
		(dispatchSize.height + 15) / 16,
		1);
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_TAA_A));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_TAA_B));
	BARRIER_COMPUTE(commands, textures->GetImage(ImageID::ASVGF_TAA_B));

	return true;
}

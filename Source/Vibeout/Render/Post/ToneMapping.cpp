// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "ToneMapping.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Shared/Buffers.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Shared/Textures.h"
#include "Vibeout/Render/Shared/Shaders.h"
#include "Vibeout/Render/Shared/VertexBuffer.h"
#include "Vibeout/Render/Buffer/Buffer.h"

ToneMapping::ToneMapping(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

ToneMapping::~ToneMapping()
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	if (pipeline_layout_tone_mapping_histogram)
		vkDestroyPipelineLayout(device, pipeline_layout_tone_mapping_histogram, nullptr);
	if (pipeline_layout_tone_mapping_curve)
		vkDestroyPipelineLayout(device, pipeline_layout_tone_mapping_curve, nullptr);
	if (pipeline_layout_tone_mapping_apply)
		vkDestroyPipelineLayout(device, pipeline_layout_tone_mapping_apply, nullptr);
}

bool ToneMapping::Init()
{
	//VO_TRY(InitDescriptorSetLayouts());
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);
	VertexBuffer* vertexBuffer = _renderer._vertexBuffer;
	VO_TRY(vertexBuffer);
	Bloom* bloom = _renderer._bloom;
	VO_TRY(bloom);

	VkDescriptorSetLayout desc_set_layouts[] =
	{
		buffers->GetDescriptorSetLayout(),
		textures->GetDescSetLayout(),
		vertexBuffer->GetDescSetLayout()
	};

	VkPushConstantRange push_constant_range_curve =
	{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = 16 * sizeof(float)
	};

	VkPushConstantRange push_constant_range_apply =
	{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = 3 * sizeof(float)
	};

	CREATE_PIPELINE_LAYOUT(device, &pipeline_layout_tone_mapping_histogram,
		.setLayoutCount = std::size(desc_set_layouts),
		.pSetLayouts = desc_set_layouts,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr
	);
	_renderer.SetObjectName(pipeline_layout_tone_mapping_histogram, "ToneMappingHistogram");

	CREATE_PIPELINE_LAYOUT(device, &pipeline_layout_tone_mapping_curve,
		.setLayoutCount = std::size(desc_set_layouts),
		.pSetLayouts = desc_set_layouts,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range_curve
	);
	_renderer.SetObjectName(pipeline_layout_tone_mapping_curve, "ToneMappingCurve");

	CREATE_PIPELINE_LAYOUT(device, &pipeline_layout_tone_mapping_apply,
		.setLayoutCount = std::size(desc_set_layouts),
		.pSetLayouts = desc_set_layouts,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range_apply
	);
	_renderer.SetObjectName(pipeline_layout_tone_mapping_apply, "ToneMappingApply");

	return true;
}

void ToneMapping::RequestReset()
{
	reset_required = 1;
}

bool ToneMapping::InitPipelines()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkSpecializationMapEntry specEntries[] =
	{
		{.constantID = 0, .offset = 0, .size = sizeof(uint32_t) }
	};

	// "HDR tone mapping" flag
	uint32_t spec_data[] =
	{
		0,
		1,
	};

	VkSpecializationInfo specInfo_SDR = { .mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32_t), .pData = &spec_data[0] };
	VkSpecializationInfo specInfo_HDR = { .mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32_t), .pData = &spec_data[1] };

	VkComputePipelineCreateInfo pipeline_info[(int)PipelineID::END] = {};
	for (VkComputePipelineCreateInfo& info : pipeline_info)
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::TONE_MAPPING_HISTOGRAM];
		info.stage = SHADER_STAGE("ToneMappingHistogram.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = pipeline_layout_tone_mapping_histogram;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::TONE_MAPPING_CURVE];
		info.stage = SHADER_STAGE("ToneMappingCurve.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = pipeline_layout_tone_mapping_curve;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::TONE_MAPPING_APPLY_SDR];
		info.stage = SHADER_STAGE_SPEC("ToneMappingApply.comp", VK_SHADER_STAGE_COMPUTE_BIT, &specInfo_SDR);
		info.layout = pipeline_layout_tone_mapping_apply;
	}

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::TONE_MAPPING_APPLY_HDR];
		info.stage = SHADER_STAGE_SPEC("ToneMappingApply.comp", VK_SHADER_STAGE_COMPUTE_BIT, &specInfo_HDR);
		info.layout = pipeline_layout_tone_mapping_apply;
	}

	VO_TRY_VK(vkCreateComputePipelines(device, 0, std::size(pipeline_info), pipeline_info, 0, _pipelines));

	RequestReset();

	return true;
}

void ToneMapping::ShutdownPipelines()
{
	VkDevice device = _renderer.GetDevice();
	VO_CHECK(device);
	for (VkPipeline& pipeline : _pipelines)
	{
		if (pipeline)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
			pipeline = nullptr;
		}
	}
}

bool ToneMapping::RecordCommandBuffer(VkCommandBuffer cmd_buf, float frame_time)
{
	VO_SCOPE_VK_CMD_LABEL(cmd_buf, "ToneMapping");
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);
	VertexBuffer* vertexBuffer = _renderer._vertexBuffer;
	VO_TRY(vertexBuffer);
	Buffer* buffer = vertexBuffer->GetToneMapBuffer();
	VO_TRY(buffer);
	VkBuffer bufferVK = buffer->GetBuffer();
	VO_TRY(bufferVK);

	if (reset_required)
	{
		// Clear the histogram image.
		VO_CHECK(Reset(cmd_buf));
	}

	VkDescriptorSet desc_sets[] =
	{
		buffers->GetDescriptorSet(),
		textures->GetCurrentDescSet(),
		vertexBuffer->GetDescSet()
	};

	// Histogram
	{
		VO_SCOPE_VK_CMD_LABEL(cmd_buf, "Histogram");
		QueueImageBarrier(cmd_buf, textures->GetImage(ImageID::TAA_OUTPUT), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

		// Record instructions to run the compute shader that updates the histogram.
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::TONE_MAPPING_HISTOGRAM]);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline_layout_tone_mapping_histogram, 0, std::size(desc_sets), desc_sets, 0, 0);

		vkCmdDispatch(cmd_buf,
			(_renderer._extentTAAOutput.width + 15) / 16,
			(_renderer._extentTAAOutput.height + 15) / 16,
			1);
	}

	// Curve
	{
		VO_SCOPE_VK_CMD_LABEL(cmd_buf, "Curve");
		{
			VkBufferMemoryBarrier barrier = BUFFER_BARRIER();
			barrier.buffer = bufferVK;
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			QUEUE_BUFFER_BARRIER(cmd_buf, barrier);
		}

		// Record instructions to run the compute shader that computes the tone
		// curve and autoexposure constants from the histogram.
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::TONE_MAPPING_CURVE]);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline_layout_tone_mapping_curve, 0, std::size(desc_sets), desc_sets, 0, 0);

		// Compute the push constants for the tone curve generation shader:
		// whether to ignore previous tone curve results, how much to blend this
		// frame's tone curve with the previous frame's tone curve, and the kernel
		// used to filter the tone curve's slopes.
		// This is one of the things we added to Eilertsen's tone mapper, and helps
		// prevent artifacts that occur when the tone curve's slopes become flat or
		// change too suddenly (much like when you specify a curve in an image 
		// processing application that is too intense). This especially helps on
		// shadow edges in some scenes.
		// In addition, we assume the kernel is symmetric; this allows us to only
		// specify half of it in our push constant buffer.

		float push_constants_tm2_curve[16] =
		{
			 reset_required ? 1.0f : 0.0f, // 1 means reset the histogram
			 frame_time, // Frame time
			 0.0f, 0.0f, 0.0f, 0.0f, // Slope kernel filter
			 0.0f, 0.0f, 0.0f, 0.0f,
			 0.0f, 0.0f, 0.0f, 0.0f,
			 0.0f, 0.0f
		};

		// Compute Gaussian curve and sum, taking symmetry into account.
		float gaussian_sum = 0.0f;
		for (int i = 0; i < 14; ++i)
		{
			float kernel_value = exp(-i * i / (2.0f * _slopeBlurSigma * _slopeBlurSigma));
			gaussian_sum += kernel_value * (i == 0 ? 1 : 2);
			push_constants_tm2_curve[i + 2] = kernel_value;
		}
		// Normalize the result (since even with an analytic normalization factor,
		// the results may not sum to one).
		for (int i = 0; i < 14; ++i) {
			push_constants_tm2_curve[i + 2] /= gaussian_sum;
		}

		vkCmdPushConstants(cmd_buf, pipeline_layout_tone_mapping_curve,
			VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push_constants_tm2_curve), push_constants_tm2_curve);

		vkCmdDispatch(cmd_buf, 1, 1, 1);
	}

	// Apply
	{
		VO_SCOPE_VK_CMD_LABEL(cmd_buf, "Apply");
		{
			VkBufferMemoryBarrier barrier = BUFFER_BARRIER();
			barrier.buffer = bufferVK;
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			QUEUE_BUFFER_BARRIER(cmd_buf, barrier);
		}

		QueueImageBarrier(cmd_buf, textures->GetImage(ImageID::BLOOM), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

		// Record instructions to apply our tone curve to the final image, apply
		// the autoexposure tone mapper to the final image, and blend the results
		// of the two techniques.
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)(_renderer._surfaceIsHDR ? PipelineID::TONE_MAPPING_APPLY_HDR : PipelineID::TONE_MAPPING_APPLY_SDR)]);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline_layout_tone_mapping_apply, 0, std::size(desc_sets), desc_sets, 0, 0);

		// At the end of the hue-preserving tone mapper, the luminance of every
		// pixel is mapped to the range [0,1]. However, because this tone
		// mapper adjusts luminance while preserving hue and saturation, the values
		// of some RGB channels may lie outside [0,1]. To finish off the tone
		// mapping pipeline and since we want the brightest colors in the scene to
		// be desaturated a bit for display, we apply a subtle user-configurable
		// Reinhard tone mapping curve to the brighest values in the image at this
		// point, preserving pixels with luminance below tm_knee_start.
		//
		// If we wanted to support an arbitrary SDR color grading pipeline here or
		// implement an additional filmic tone mapping pass, for instance, this is
		// roughly around where it would be applied. For applications that need to
		// output both SDR and HDR images but for which creating custom grades
		// for each format is impractical, one common approach is to
		// (roughly) use the HDR->SDR transformation to map an SDR color grading
		// function back to an HDR color grading function.

		// We modify Reinhard to smoothly blend with the identity transform up to tm_knee_start.
		// We need to find w, a, and b such that in y(x) = (wx+a)/(x+b),
		// * y(knee_start) = tm_knee_start
		// * dy/dx(knee_start) = 1
		// * y(knee_white_point) = tm_white_point.
		// The solution is as follows:
		float knee_w = (_kneeStart * (_kneeStart - 2.0f) + _kneeWhitePoint) / (_kneeWhitePoint - 1.0f);
		float knee_a = -_kneeStart * _kneeStart;
		float knee_b = knee_w - 2.0f * _kneeStart;

		float push_constants_tm2_apply[3] = {
			knee_w, // knee_w in piecewise knee adjustment
			knee_a, // knee_a in piecewise knee adjustment
			knee_b, // knee_b in piecewise knee adjustment
		};

		vkCmdPushConstants(cmd_buf, pipeline_layout_tone_mapping_apply,
			VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push_constants_tm2_apply), push_constants_tm2_apply);

		vkCmdDispatch(cmd_buf,
			(_renderer._extentTAAOutput.width + 15) / 16,
			(_renderer._extentTAAOutput.height + 15) / 16,
			1);

		// Because VKPT_IMG_TAA_OUTPUT changed, we make sure to wait for the image
		// to be written before continuing. This could be ensured in several
		// other ways as well.
		QueueImageBarrier(cmd_buf, textures->GetImage(ImageID::TAA_OUTPUT), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	}

	reset_required = 0;

	return true;
}

void ToneMapping::FillUBO(GlobalUniformBuffer& ubo)
{
	ubo.tm_slope_blur_sigma = _slopeBlurSigma;
	ubo.tm_knee_start = _kneeStart;
}

bool ToneMapping::Reset(VkCommandBuffer commands)
{
	VertexBuffer* vertexBuffer = _renderer._vertexBuffer;
	VO_TRY(vertexBuffer);
	Buffer* toneMapBuffer = vertexBuffer->GetToneMapBuffer();
	VO_TRY(toneMapBuffer);
	VkBuffer buffer = toneMapBuffer->GetBuffer();
	VO_TRY(buffer);

	{
		VkBufferMemoryBarrier barrier = BUFFER_BARRIER();
		barrier.buffer = buffer;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;
		QUEUE_BUFFER_BARRIER(commands, barrier);
	}


	vkCmdFillBuffer(commands, buffer, 0, VK_WHOLE_SIZE, 0);

	{
		VkBufferMemoryBarrier barrier = BUFFER_BARRIER();
		barrier.buffer = buffer;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		QUEUE_BUFFER_BARRIER(commands, barrier);
	}

	return true;
}
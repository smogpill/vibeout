// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Draw.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Shared/Buffers.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Shared/Textures.h"
#include "Vibeout/Render/Shared/Shaders.h"
#include "Vibeout/Render/Buffer/Buffer.h"

Draw::Draw(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

Draw::~Draw()
{
	for (Buffer*& buffer : buf_ubo)
		delete buffer;
	for (Buffer*& buffer : buf_stretch_pic_queue)
		delete buffer;
}

bool Draw::Init()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VO_TRY(InitRenderPass());
	for (int i = 0; i < maxFramesInFlight; ++i)
	{
		{
			Buffer::Setup setup;
			setup._size = sizeof(StretchPic) * MAX_STRETCH_PICS;
			setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			setup._usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			bool result;
			buf_stretch_pic_queue[i] = new Buffer(_renderer, setup, result);
			VO_TRY(result);
		}

		{
			Buffer::Setup setup;
			setup._size = sizeof(StretchPic_UBO);
			setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			setup._usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			bool result;
			buf_ubo[i] = new Buffer(_renderer, setup, result);
			VO_TRY(result);
		}
	}

	VkDescriptorSetLayoutBinding layout_bindings[] =
	{
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		},
	};

	VkDescriptorSetLayoutCreateInfo layout_info = 
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = std::size(layout_bindings),
		.pBindings = layout_bindings,
	};

	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &desc_set_layout_sbo));

	VkDescriptorPoolSize pool_size = 
	{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = maxFramesInFlight,
	};

	VkDescriptorPoolCreateInfo pool_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = maxFramesInFlight,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
	};

	VO_TRY_VK(vkCreateDescriptorPool(device, &pool_info, nullptr, &desc_pool_sbo));

	VkDescriptorSetLayoutBinding layout_bindings_ubo[] =
	{
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
	};

	VkDescriptorSetLayoutCreateInfo layout_info_ubo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = std::size(layout_bindings_ubo),
		.pBindings = layout_bindings_ubo,
	};

	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layout_info_ubo, nullptr, &desc_set_layout_ubo));
	_renderer.SetObjectName(desc_set_layout_ubo, "UBO_DeskSetLayout");

	VkDescriptorPoolSize pool_size_ubo =
	{
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = maxFramesInFlight,
	};

	VkDescriptorPoolCreateInfo pool_info_ubo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = maxFramesInFlight,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size_ubo,
	};

	VO_TRY_VK(vkCreateDescriptorPool(device, &pool_info_ubo, nullptr, &desc_pool_ubo));
	_renderer.SetObjectName(desc_pool_ubo, "UBO_DescPool");

	VkDescriptorSetLayoutBinding layout_binding_final_blit =
	{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};

	VkDescriptorSetLayoutCreateInfo layout_info_final_blit =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &layout_binding_final_blit,
	};

	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layout_info_final_blit, nullptr, &desc_set_layout_final_blit));
	_renderer.SetObjectName(desc_set_layout_final_blit, "FinalBlit_DescSetLayout");

	VkDescriptorPoolSize pool_size_final_blit =
	{
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = maxFramesInFlight,
	};

	VkDescriptorPoolCreateInfo pool_info_final_blit =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = maxFramesInFlight,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size_final_blit,
	};

	VO_TRY_VK(vkCreateDescriptorPool(device, &pool_info_final_blit, nullptr, &desc_pool_final_blit));
	_renderer.SetObjectName(desc_pool_final_blit, "FinalBlit_DescPool");

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = desc_pool_sbo,
		.descriptorSetCount = 1,
		.pSetLayouts = &desc_set_layout_sbo,
	};

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info_ubo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = desc_pool_ubo,
		.descriptorSetCount = 1,
		.pSetLayouts = &desc_set_layout_ubo,
	};

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info_final_blit =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = desc_pool_final_blit,
		.descriptorSetCount = 1,
		.pSetLayouts = &desc_set_layout_final_blit,
	};

	for (int i = 0; i < maxFramesInFlight; ++i)
	{
		VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, desc_set_sbo + i));
		Buffer* sbo = buf_stretch_pic_queue[i];

		VkDescriptorBufferInfo buf_info =
		{
			.buffer = sbo->GetBuffer(),
			.offset = 0,
			.range = sizeof(stretch_pic_queue),
		};

		VkWriteDescriptorSet output_buf_write =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = desc_set_sbo[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &buf_info,
		};

		vkUpdateDescriptorSets(device, 1, &output_buf_write, 0, nullptr);

		VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info_ubo, desc_set_ubo + i));
		Buffer* ubo = buf_ubo[i];

		VkDescriptorBufferInfo buf_info_ubo =
		{
			.buffer = ubo->GetBuffer(),
			.offset = 0,
			.range = sizeof(StretchPic_UBO),
		};

		VkWriteDescriptorSet output_buf_write_ubo =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = desc_set_ubo[i],
			.dstBinding = 2,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &buf_info_ubo,
		};

		vkUpdateDescriptorSets(device, 1, &output_buf_write_ubo, 0, nullptr);

		VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info_final_blit, desc_set_final_blit + i));
	}
	return true;
}

bool Draw::InitRenderPass()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkAttachmentDescription color_attachment =
	{
		.format = _renderer._surfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
		//.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		//.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
		.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference color_attachment_ref =
	{
		.attachment = 0, /* index in fragment shader */
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass =
	{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
	};

	VkSubpassDependency dependencies[] =
	{
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0, /* index for own subpass */
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0, /* XXX verify */
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},
	};

	VkRenderPassCreateInfo render_pass_info =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &color_attachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = std::size(dependencies),
		.pDependencies = dependencies,
	};

	VO_TRY_VK(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass_stretch_pic));
	_renderer.SetObjectName(render_pass_stretch_pic, "StretchPic");
	return true;
}

bool Draw::InitPipelines()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);
	const Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);

	const VkExtent2D drawExtents = _renderer._extentUnscaled;

	VO_ASSERT(desc_set_layout_sbo);
	VkDescriptorSetLayout desc_set_layouts[] =
	{
		desc_set_layout_sbo, textures->desc_set_layout_textures, desc_set_layout_ubo
	};
	CREATE_PIPELINE_LAYOUT(device, &pipeline_layout_stretch_pic,
		.setLayoutCount = std::size(desc_set_layouts),
		.pSetLayouts = desc_set_layouts
	);

	VkPushConstantRange push_constant_range_final_blit =
	{
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof(FinalBlitPushConstants)
	};

	VkDescriptorSetLayout desc_set_layouts_final_blit[] =
	{
		buffers->GetDescriptorSetLayout(), desc_set_layout_final_blit
	};
	CREATE_PIPELINE_LAYOUT(device, &pipeline_layout_final_blit,
		.setLayoutCount = std::size(desc_set_layouts_final_blit),
		.pSetLayouts = desc_set_layouts_final_blit,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range_final_blit
	);

	VkSpecializationMapEntry specEntries[] =
	{
		{.constantID = 0, .offset = 0, .size = sizeof(uint32) }
	};

	// "HDR display" flag
	uint32 spec_data[] =
	{
		0,
		1,
	};

	VkSpecializationInfo specInfo_SDR = { .mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32), .pData = &spec_data[0] };
	VkSpecializationInfo specInfo_HDR = { .mapEntryCount = 1, .pMapEntries = specEntries, .dataSize = sizeof(uint32), .pData = &spec_data[1] };

	VkPipelineShaderStageCreateInfo shader_info_SDR[] =
	{
		SHADER_STAGE("StretchPic.vert", VK_SHADER_STAGE_VERTEX_BIT),
		SHADER_STAGE_SPEC("StretchPic.frag", VK_SHADER_STAGE_FRAGMENT_BIT, &specInfo_SDR)
	};

	VkPipelineShaderStageCreateInfo shader_info_HDR[] =
	{
		SHADER_STAGE("StretchPic.vert", VK_SHADER_STAGE_VERTEX_BIT),
		SHADER_STAGE_SPEC("StretchPic.frag", VK_SHADER_STAGE_FRAGMENT_BIT, &specInfo_HDR)
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkViewport viewport =
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)drawExtents.width,
		.height = (float)drawExtents.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor =
	{
		.offset = { 0, 0 },
		.extent = drawExtents,
	};

	VkPipelineViewportStateCreateInfo viewport_state =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	VkPipelineRasterizationStateCreateInfo rasterizer_state =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE, /* skip rasterizer */
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisample_state =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment =
	{
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
							 | VK_COLOR_COMPONENT_G_BIT
							 | VK_COLOR_COMPONENT_B_BIT
							 | VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
		.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
	};

	VkGraphicsPipelineCreateInfo pipeline_info =
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = std::size(shader_info_SDR),

		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly_info,
		.pViewportState = &viewport_state,
		.pRasterizationState = &rasterizer_state,
		.pMultisampleState = &multisample_state,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &color_blend_state,
		.pDynamicState = nullptr,

		.layout = pipeline_layout_stretch_pic,
		.renderPass = render_pass_stretch_pic,
		.subpass = 0,

		.basePipelineHandle = nullptr,
		.basePipelineIndex = -1,
	};

	pipeline_info.pStages = shader_info_SDR;
	VO_TRY_VK(vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_info, nullptr, &pipeline_stretch_pic[STRETCH_PIC_SDR]));
	_renderer.SetObjectName(pipeline_stretch_pic[STRETCH_PIC_SDR], "StretchPicSDR");

	pipeline_info.pStages = shader_info_HDR;
	VO_TRY_VK(vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_info, nullptr, &pipeline_stretch_pic[STRETCH_PIC_HDR]));
	_renderer.SetObjectName(pipeline_stretch_pic[STRETCH_PIC_HDR], "StretchPicHDR");

	VkSpecializationMapEntry final_blit_spec_entries[] =
	{
		{.constantID = 0, .offset = 0, .size = sizeof(uint32) },
		{.constantID = 1, .offset = 4, .size = sizeof(uint32) }
	};

	pipeline_info.layout = pipeline_layout_final_blit;
	for (uint32 i = 0; i < FINAL_BLIT_NUM_PIPELINES; i++)
	{
		uint32 final_blit_spec_data[2] = { (i & FINAL_BLIT_FILTERED) ? 1u : 0u, (i & FINAL_BLIT_WARPED) ? 1u : 0u };
		VkSpecializationInfo spec_info_final_blit = { .mapEntryCount = std::size(final_blit_spec_entries), .pMapEntries = final_blit_spec_entries, .dataSize = sizeof(uint32) * 2, .pData = final_blit_spec_data };

		VkPipelineShaderStageCreateInfo shader_info_final_blit[] =
		{
			SHADER_STAGE("FinalBlit.vert", VK_SHADER_STAGE_VERTEX_BIT),
			SHADER_STAGE_SPEC("FinalBlit.frag", VK_SHADER_STAGE_FRAGMENT_BIT, &spec_info_final_blit)
		};

		pipeline_info.pStages = shader_info_final_blit;
		VO_TRY_VK(vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_info, nullptr, &pipeline_final_blit[i]));
		_renderer.SetObjectName(pipeline_final_blit[i], "FinalBlit");
	}

	const size_t nbSwapChainImages = _renderer._swapChainImages.size();
	framebuffer_stretch_pic.resize(nbSwapChainImages, nullptr);
	for (int i = 0; i < nbSwapChainImages; ++i)
	{
		VkImageView attachments[] =
		{
			_renderer._swapChainImageViews[i]
		};

		VkFramebufferCreateInfo fb_create_info =
		{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = render_pass_stretch_pic,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = drawExtents.width,
			.height = drawExtents.height,
			.layers = 1,
		};

		VO_TRY_VK(vkCreateFramebuffer(device, &fb_create_info, nullptr, &framebuffer_stretch_pic[i]));
		_renderer.SetObjectName(framebuffer_stretch_pic[i], "FrameBufferStretchPic");
	}

	return true;
}

void Draw::ShutdownPipelines()
{
	VkDevice device = _renderer.GetDevice();

	if (device)
	{
		for (VkPipeline pipeline : pipeline_stretch_pic)
			vkDestroyPipeline(device, pipeline, nullptr);
		for (VkPipeline pipeline : pipeline_final_blit)
			vkDestroyPipeline(device, pipeline, nullptr);
		if (pipeline_layout_stretch_pic)
			vkDestroyPipelineLayout(device, pipeline_layout_stretch_pic, nullptr);
		if (pipeline_layout_final_blit)
			vkDestroyPipelineLayout(device, pipeline_layout_final_blit, nullptr);
		for (VkFramebuffer framebuffer : framebuffer_stretch_pic)
			vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	pipeline_layout_stretch_pic = nullptr;
	pipeline_layout_final_blit = nullptr;
	for (VkPipeline& pipeline : pipeline_stretch_pic)
		pipeline = nullptr;
	for (VkPipeline& pipeline : pipeline_final_blit)
		pipeline = nullptr;
	for (VkFramebuffer& framebuffer : framebuffer_stretch_pic)
		framebuffer = nullptr;
	framebuffer_stretch_pic.clear();
}

bool Draw::ClearStretchPics()
{
	num_stretch_pics = 0;
	return true;
}

bool Draw::SubmitStretchPics(VkCommandBuffer commands)
{
	/*
	if (num_stretch_pics == 0)
		return true;

	const uint32 currentFrameInFlight = _renderer.GetCurrentFrameInFlight();

	RenderBuffer* buf_spq = buf_stretch_pic_queue[currentFrameInFlight];
	StretchPic* spq_dev = (StretchPic*)buf_spq->Map();
	memcpy(spq_dev, stretch_pic_queue, sizeof(StretchPic) * num_stretch_pics);
	buf_spq->Unmap();
	spq_dev = nullptr;

	RenderBuffer* ubo_res = buf_ubo[currentFrameInFlight];
	StretchPic_UBO* ubo = (StretchPic_UBO*)ubo_res->Map();
	ubo->ui_hdr_nits = cvar_ui_hdr_nits->value;
	ubo->tm_hdr_saturation_scale = cvar_tm_hdr_saturation_scale->value;
	ubo_res->Unmap();
	ubo = nullptr;

	VkRenderPassBeginInfo render_pass_info =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass_stretch_pic,
		.framebuffer = framebuffer_stretch_pic[_renderer.GetCurrentSwapChainImageIdx()],
		.renderArea.offset = { 0, 0 },
		.renderArea.extent = _renderer.GetUnscaledExtents()
	};

	VkDescriptorSet desc_sets[] =
	{
		desc_set_sbo[currentFrameInFlight],
		qvk_get_current_desc_set_textures(),
		desc_set_ubo[currentFrameInFlight],
	};

	vkCmdBeginRenderPass(commands, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout_stretch_pic, 0, std::size(desc_sets), desc_sets, 0, 0);
	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_stretch_pic[_renderer.IsSurfaceHDR() ? STRETCH_PIC_HDR : STRETCH_PIC_SDR]);
	vkCmdDraw(commands, 4, num_stretch_pics, 0, 0);
	vkCmdEndRenderPass(commands);

	*/
	num_stretch_pics = 0;
	return true;
}

bool Draw::FinalBlit(VkCommandBuffer commands, ImageID imageID, VkExtent2D extent, bool filtered, bool warped)
{
	VO_SCOPE_VK_CMD_LABEL(commands, "FinalBlit");
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);

	VkDescriptorImageInfo img_info =
	{
		.sampler = textures->tex_sampler,
		.imageView = textures->GetImageView(imageID),
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	};
	VkWriteDescriptorSet elem_image =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = desc_set_final_blit[_renderer._currentFrameInFlight],
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &img_info,
	};

	vkUpdateDescriptorSets(device, 1, &elem_image, 0, nullptr);

	VkRenderPassBeginInfo render_pass_info =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass_stretch_pic,
		.framebuffer = framebuffer_stretch_pic[_renderer._currentSwapChainImageIdx],
		.renderArea =
		{
			.offset = {0, 0},
			.extent = _renderer._extentUnscaled,
		},
	};

	VkDescriptorSet desc_sets[] =
	{
		buffers->GetDescriptorSet(),
		desc_set_final_blit[_renderer._currentFrameInFlight]
	};

	FinalBlitPushConstants push_constants = { .input_dimensions = {(float)extent.width, (float)extent.height} };

	vkCmdBeginRenderPass(commands, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout_final_blit, 0, std::size(desc_sets), desc_sets, 0, 0);
	int pipeline_idx = (filtered ? FINAL_BLIT_FILTERED : 0) | (warped ? FINAL_BLIT_WARPED : 0);
	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_final_blit[pipeline_idx]);
	vkCmdPushConstants(commands, pipeline_layout_final_blit,
		VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_constants), &push_constants);
	vkCmdDraw(commands, 4, 1, 0, 0);
	vkCmdEndRenderPass(commands);

	return true;
}

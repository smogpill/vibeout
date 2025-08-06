// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Render/Shared/Base.h"
class Buffer;
class Renderer;
enum class ImageID;

class Draw
{
public:
	Draw(Renderer& renderer, bool& result);
	~Draw();

	bool InitPipelines();
	void ShutdownPipelines();
	bool ClearStretchPics();
	bool FinalBlit(VkCommandBuffer commands, ImageID imageID, VkExtent2D extent, bool filtered, bool warped);
	bool SubmitStretchPics(VkCommandBuffer commands);

private:
	enum
	{
		STRETCH_PIC_SDR,
		STRETCH_PIC_HDR,
		STRETCH_PIC_NUM_PIPELINES
	};

	enum
	{
		// Note: those values are ORed together to get the pipeline number
		FINAL_BLIT_FILTERED = 0x1,
		FINAL_BLIT_WARPED = 0x2,
		// ... resulting in 4 different combinations
		FINAL_BLIT_NUM_PIPELINES = 4
	};

#define TEXNUM_WHITE (~0)
#define MAX_STRETCH_PICS (1<<14)

	/*
	drawStatic_t draw = {
		.scale = 1.0f,
		.alpha_scale = 1.0f
	};*/

	struct StretchPic
	{
		float x, y, w, h;
		float s, t, w_s, h_t;
		uint32 color, tex_handle;
	};

	// Not using global UBO b/c it's only filled when a world is drawn, but here we need it all the time
	struct StretchPic_UBO
	{
		float ui_hdr_nits;
		float tm_hdr_saturation_scale;
	};

	struct FinalBlitPushConstants
	{
		float input_dimensions[2];
	};

	bool Init();
	bool InitRenderPass();

	Renderer& _renderer;
	//clipRect_t clip_rect;
	bool clip_enable = false;
	StretchPic stretch_pic_queue[MAX_STRETCH_PICS];
	int num_stretch_pics = 0;
	VkPipelineLayout        pipeline_layout_stretch_pic = nullptr;
	VkPipelineLayout        pipeline_layout_final_blit = nullptr;
	VkRenderPass            render_pass_stretch_pic = nullptr;
	VkPipeline              pipeline_stretch_pic[STRETCH_PIC_NUM_PIPELINES] = {};
	VkPipeline              pipeline_final_blit[FINAL_BLIT_NUM_PIPELINES] = {};
	std::vector<VkFramebuffer> framebuffer_stretch_pic;
	Buffer* buf_stretch_pic_queue[maxFramesInFlight] = {};
	Buffer* buf_ubo[maxFramesInFlight] = {};
	VkDescriptorSetLayout   desc_set_layout_sbo = {};
	VkDescriptorSetLayout   desc_set_layout_ubo = {};
	VkDescriptorSetLayout   desc_set_layout_final_blit = {};
	VkDescriptorPool        desc_pool_sbo = {};
	VkDescriptorPool        desc_pool_ubo = {};
	VkDescriptorPool        desc_pool_final_blit = {};
	VkDescriptorSet         desc_set_sbo[maxFramesInFlight] = {};
	VkDescriptorSet         desc_set_ubo[maxFramesInFlight] = {};
	VkDescriptorSet         desc_set_final_blit[maxFramesInFlight] = {};
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Base.h"
#include "Vibeout/Render/Allocator/DeviceAllocator.h"
#include "Shaders/GlobalTextures.h"
class Renderer;

//#define MAX_RIMAGES 2048
#define MAX_RIMAGES NB_GLOBAL_TEXTURES
#define MAX_RBUFFERS 16
#define DESTROY_LATENCY (maxFramesInFlight * 4)

class Textures
{
public:
	Textures(Renderer& renderer, bool& result);
	~Textures();

	bool InitFramebufferImages();
	void ShutdownFramebufferImages();
	void QueueImageBarrier(VkCommandBuffer commands, ImageID imageID, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

	const VkImage& GetImage(ImageID imageID) const;
	const VkImageView& GetImageView(ImageID imageID) const;
	const VkDescriptorSet& GetCurrentDescSet() const;
	const VkDescriptorSetLayout& GetDescSetLayout() const { return desc_set_layout_textures; }
	const VkSampler& GetDefaultSampler() const { return tex_sampler; }
	const VkSampler& GetClampToEdgeSampler() const { return tex_sampler_linear_clamp_edge; }
	const VkSampler& GetClampToBorderSampler() const { return tex_sampler_linear_clamp_border; }

private:
	friend class Draw;

	bool Init();
	bool InitInvalidTexture();
	bool InitBlueNoise();
	bool InitHeightmap();
	bool InitSamplers();
	void InvalidateTextureDescriptors();

	struct UnusedResources
	{
		VkImage         images[MAX_RIMAGES] = {};
		VkImageView     image_views[MAX_RIMAGES] = {};
		VkImageView     image_views_mip0[MAX_RIMAGES] = {};
		DeviceMemory    image_memory[MAX_RIMAGES] = {};
		uint32_t        image_num = 0;
		VkBuffer        buffers[MAX_RBUFFERS] = {};
		VkDeviceMemory  buffer_memory[MAX_RBUFFERS] = {};
		uint32_t        buffer_num = 0;
	};

	struct TextureSystem
	{
		UnusedResources unused_resources[DESTROY_LATENCY] = {};
	};

	Renderer& _renderer;
	TextureSystem texture_system = {};

	VkImage          tex_images[MAX_RIMAGES] = {};
	VkImageView      tex_image_views[MAX_RIMAGES] = {};
	VkImageView      tex_image_views_mip0[MAX_RIMAGES] = {};
	VkDeviceMemory   mem_blue_noise = nullptr;
	VkDeviceMemory   mem_heightmap = nullptr;
	VkDeviceMemory   mem_envmap = nullptr;
	VkImage          img_blue_noise = nullptr;
	VkImageView      imv_blue_noise = nullptr;
	VkImage _img_heightmap = nullptr;
	VkImageView _imv_heightmap = nullptr;
	VkImage          img_envmap = nullptr;
	VkImageView      imv_envmap = nullptr;
	VkDescriptorPool desc_pool_textures = nullptr;

	VkImage          tex_invalid_texture_image = nullptr;
	VkImageView      tex_invalid_texture_image_view = nullptr;
	DeviceMemory     tex_invalid_texture_image_memory = {};

	VkDeviceMemory mem_images[NB_IMAGES] = {};

	DeviceMemory            tex_image_memory[MAX_RIMAGES] = {};
	VkBindImageMemoryInfo   tex_bind_image_info[MAX_RIMAGES] = {};
	DeviceMemoryAllocator* tex_device_memory_allocator = nullptr;

	// Resources for the normal map normalization pass that runs on texture upload
	VkDescriptorSetLayout   normalize_desc_set_layout = nullptr;
	VkPipelineLayout        normalize_pipeline_layout = nullptr;
	VkPipeline              normalize_pipeline = nullptr;
	VkDescriptorSet         normalize_descriptor_sets[maxFramesInFlight] = {};

	VkSampler tex_sampler = nullptr;
	VkSampler tex_sampler_nearest = nullptr;
	VkSampler tex_sampler_nearest_mipmap_aniso = nullptr;
	VkSampler tex_sampler_linear_clamp_edge = nullptr;
	VkSampler tex_sampler_linear_clamp_border = nullptr;

	VkDescriptorSetLayout desc_set_layout_textures = nullptr;
	VkDescriptorSet desc_set_textures_even = nullptr;
	VkDescriptorSet desc_set_textures_odd = nullptr;
	VkImage                     images[NB_IMAGES];
	VkImageView                 images_views[NB_IMAGES];

	// Array for tracking when the textures have been uploaded.
	// On frame N (which is modulo MAX_FRAMES_IN_FLIGHT), a texture is uploaded, and that writes N into this array.
	// At the same time, its storage image descriptor is created in normalize_descriptor_sets[N], and the texture
	// is normalized using the normalize_pipeline. When vkpt_textures_end_registration() runs on the next frame
	// with the same N, it sees the texture again, and deletes its descriptor from that descriptor set.
	uint32                tex_upload_frames[MAX_RIMAGES] = {};

	int image_loading_dirty_flag = 0;
	uint8 descriptor_set_dirty_flags[maxFramesInFlight] = {}; // initialized in vkpt_textures_initialize
};

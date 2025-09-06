// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Vibeout/Render/Shared/Textures.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Buffer/Buffer.h"
#include "Vibeout/Game/Game.h"
#include "Vibeout/World/World.h"
#include "Vibeout/World/Terrain/Terrain.h"
#include "Vibeout/Resource/Texture/Texture.h"
#include "Vibeout/Base/Utils.h"

Textures::Textures(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

Textures::~Textures()
{
	VO_ASSERT(false);
}

const VkImage& Textures::GetImage(ImageID imageID) const
{
	VO_ASSERT((int)imageID < std::size(images));
	return images[(int)imageID];
}

const VkImageView& Textures::GetImageView(ImageID imageID) const
{
	VO_ASSERT((int)imageID < std::size(images_views));
	return images_views[(int)imageID];
}

const VkDescriptorSet& Textures::GetCurrentDescSet() const
{
	return (_renderer._frameCounter & 1) ? desc_set_textures_odd : desc_set_textures_even;
}

bool Textures::Init()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	InvalidateTextureDescriptors();

	tex_device_memory_allocator = create_device_memory_allocator(device);

	VO_TRY(InitInvalidTexture());
	VO_TRY(InitSamplers());

	VkDescriptorSetLayoutBinding layout_bindings[] = {
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_RIMAGES,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
#define IMG_DO(_name, _binding, ...) \
		{ \
			.binding         = BINDING_OFFSET_IMAGES + _binding, \
			.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, \
			.descriptorCount = 1, \
			.stageFlags      = VK_SHADER_STAGE_ALL, \
		},
	LIST_IMAGES
	LIST_IMAGES_A_B
#undef IMG_DO
#define IMG_DO(_name, _binding, ...) \
		{ \
			.binding         = BINDING_OFFSET_TEXTURES + _binding, \
			.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, \
			.descriptorCount = 1, \
			.stageFlags      = VK_SHADER_STAGE_ALL, \
		},
	LIST_IMAGES
	LIST_IMAGES_A_B
#undef IMG_DO
		{
			.binding = BINDING_OFFSET_BLUE_NOISE,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = BINDING_OFFSET_HEIGHTMAP,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = BINDING_OFFSET_TERRAIN_DIFFUSE,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
	};

	VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = std::size(layout_bindings),
		.pBindings = layout_bindings,
	};

	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &desc_set_layout_textures));
	//SET_VK_NAME(desc_set_layout_textures, "Textures");

	VkDescriptorPoolSize pool_sizes[] = {
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = maxFramesInFlight * (MAX_RIMAGES + 2 * NB_IMAGES) + 128,
		},
		{
			.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = maxFramesInFlight * MAX_RIMAGES
		}
	};

	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = maxFramesInFlight * 2,
		.poolSizeCount = std::size(pool_sizes),
		.pPoolSizes = pool_sizes,
	};

	VO_TRY_VK(vkCreateDescriptorPool(device, &pool_info, nullptr, &desc_pool_textures));
	//SET_VK_NAME(desc_pool_textures, "Textures");

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = desc_pool_textures,
		.descriptorSetCount = 1,
		.pSetLayouts = &desc_set_layout_textures,
	};

	VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, &desc_set_textures_even));
	VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, &desc_set_textures_odd));

	//SET_VK_NAME(desc_set_textures_even, "Even");
	//SET_VK_NAME(desc_set_textures_odd, "Odd");

	VO_TRY(UpdateDescriptorSet(BINDING_OFFSET_HEIGHTMAP, tex_invalid_texture_image_view, tex_sampler));
	VO_TRY(UpdateDescriptorSet(BINDING_OFFSET_TERRAIN_DIFFUSE, tex_invalid_texture_image_view, tex_sampler));

	VO_TRY(InitBlueNoise());

	return true;
}

void Textures::InvalidateTextureDescriptors()
{
	for (int index = 0; index < maxFramesInFlight; index++)
		descriptor_set_dirty_flags[index] = 1;
}

bool Textures::UpdateDescriptorSet(uint binding, const VkImageView& imageView, const VkSampler& sampler)
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	vkQueueWaitIdle(_renderer._graphicsQueue);

	VkDescriptorImageInfo desc_img_info =
	{
		.sampler = sampler,
		.imageView = imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet s =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = desc_set_textures_even,
		.dstBinding = binding,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &desc_img_info,
	};

	vkUpdateDescriptorSets(device, 1, &s, 0, nullptr);

	s.dstSet = desc_set_textures_odd;
	vkUpdateDescriptorSets(device, 1, &s, 0, nullptr);

	vkQueueWaitIdle(_renderer._graphicsQueue);

	return true;
}

bool Textures::SetLayout(const VkCommandBuffer& cmds, const VkImage& image, VkImageLayout fromLayout, VkImageLayout toLayout)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = fromLayout; // Or previous layout
	barrier.newLayout = toLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;

	vkCmdPipelineBarrier(
		cmds,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Or appropriate stage
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	return true;
}

bool Textures::InitInvalidTexture()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	const VkImageCreateInfo image_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.extent = { 1, 1, 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VO_TRY_VK(vkCreateImage(device, &image_create_info, NULL, &tex_invalid_texture_image));

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device, tex_invalid_texture_image, &memory_requirements);

	tex_invalid_texture_image_memory.alignment = memory_requirements.alignment;
	tex_invalid_texture_image_memory.size = memory_requirements.size;
	VO_TRY(_renderer.GetMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tex_invalid_texture_image_memory.memory_type));

	allocate_device_memory(tex_device_memory_allocator, &tex_invalid_texture_image_memory);

	VO_TRY_VK(vkBindImageMemory(device, tex_invalid_texture_image, tex_invalid_texture_image_memory.memory,
		tex_invalid_texture_image_memory.memory_offset));

	const VkImageSubresourceRange subresource_range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.levelCount = 1,
		.layerCount = 1
	};

	const VkImageViewCreateInfo image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = tex_invalid_texture_image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = image_create_info.format,
		.subresourceRange = subresource_range
	};

	VO_TRY_VK(vkCreateImageView(device, &image_view_create_info, nullptr, &tex_invalid_texture_image_view));

	VkCommandBuffer cmd_buf = _renderer.BeginCommandBuffer(_renderer._graphicsCommandBuffers);

	{
		VkImageMemoryBarrier barrier = IMAGE_BARRIER();
		barrier.image = tex_invalid_texture_image;
		barrier.subresourceRange = subresource_range;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
	}

	const VkClearColorValue color = { .float32 = { 1.0f, 0.0f, 1.0f, 1.0f } };
	const VkImageSubresourceRange range = subresource_range;
	vkCmdClearColorImage(cmd_buf, tex_invalid_texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1, &range);

	{
		VkImageMemoryBarrier barrier = IMAGE_BARRIER();
		barrier.image = tex_invalid_texture_image;
		barrier.subresourceRange = subresource_range;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
	}

	VO_TRY(_renderer.SubmitCommandBufferSimple(cmd_buf, _renderer._graphicsQueue));

	vkQueueWaitIdle(_renderer._graphicsQueue);
	return true;
}

bool Textures::InitBlueNoise()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	const int num_images = NB_BLUE_NOISE_TEX / 4;
	const int res = BLUE_NOISE_RES;
	size_t img_size = res * res;
	size_t total_size = img_size * sizeof(uint16_t);

	Buffer::Setup buf_img_upload_setup;
	buf_img_upload_setup._size = total_size * NB_BLUE_NOISE_TEX;
	buf_img_upload_setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buf_img_upload_setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	bool result;
	Buffer buf_img_upload(_renderer, buf_img_upload_setup, result);
	VO_TRY(result);

	uint16_t* bn_tex = (uint16_t*)buf_img_upload.Map();

	for (int i = 0; i < num_images; i++)
	{
		int w, h, n;
		char buf[1024];

		snprintf(buf, sizeof buf, "Assets/BlueNoise/%d_%d/HDR_RGBA_%04d.png", res, res, i);

		//coByte* filedata = 0;
		uint16_t* data = 0;
		//int filelen = FS_LoadFile(buf, (void**)&filedata);

		data = stbi_load_16(buf, &w, &h, &n, 4);
		/*
		if (filedata) {
			//data = stbi_load_16_from_memory(filedata, (int)filelen, &w, &h, &n, 4);
			data = stbi_load_16(buf, &w, &h, &n, 4);
			free(filedata);
		}
		*/

		if (!data) {
			VO_ERROR("error loading blue noise tex {}\n", buf);
			buf_img_upload.Unmap();
			return false;
		}

		/* loaded images are RGBA, want to upload as texture array though */
		for (int k = 0; k < 4; k++) {
			for (int j = 0; j < img_size; j++)
				bn_tex[(i * 4 + k) * img_size + j] = data[j * 4 + k];
		}

		stbi_image_free(data);
	}
	buf_img_upload.Unmap();
	bn_tex = nullptr;

	VkImageCreateInfo img_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_R16_UNORM,
		.extent = {
			.width = BLUE_NOISE_RES,
			.height = BLUE_NOISE_RES,
			.depth = 1,
		},
		.mipLevels = 1,
		.arrayLayers = NB_BLUE_NOISE_TEX,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_STORAGE_BIT
							   | VK_IMAGE_USAGE_TRANSFER_DST_BIT
							   | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VO_TRY_VK(vkCreateImage(device, &img_info, nullptr, &img_blue_noise));
	_renderer.SetObjectName(img_blue_noise, "BlueNoise");

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(device, img_blue_noise, &mem_req);
	VO_ASSERT(mem_req.size >= buf_img_upload.GetSize());

	VO_TRY(_renderer.AllocateGPUMemory(mem_req, &mem_blue_noise));

	VO_TRY_VK(vkBindImageMemory(device, img_blue_noise, mem_blue_noise, 0));

	VkImageViewCreateInfo img_view_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = img_blue_noise,
		.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
		.format = VK_FORMAT_R16_UNORM,
		.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_R,
		},
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = NB_BLUE_NOISE_TEX,
		},
	};
	VO_TRY_VK(vkCreateImageView(device, &img_view_info, nullptr, &imv_blue_noise));
	_renderer.SetObjectName(imv_blue_noise, "BlueNoise");

	VkCommandBuffer cmd_buf = _renderer.BeginCommandBuffer(_renderer._graphicsCommandBuffers);

	for (int layer = 0; layer < NB_BLUE_NOISE_TEX; layer++) {

		VkImageSubresourceRange subresource_range = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = uint32(layer),
			.layerCount = 1,
		};

		{
			VkImageMemoryBarrier barrier = IMAGE_BARRIER();
			barrier.image = img_blue_noise;
			barrier.subresourceRange = subresource_range;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
		}

		VkBufferImageCopy cpy_info = {
			.bufferOffset = total_size * layer,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = uint32(layer),
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { BLUE_NOISE_RES, BLUE_NOISE_RES, 1 }
		};
		vkCmdCopyBufferToImage(cmd_buf, buf_img_upload.GetBuffer(), img_blue_noise,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy_info);


		{
			VkImageMemoryBarrier barrier = IMAGE_BARRIER();
			barrier.image = img_blue_noise;
			barrier.subresourceRange = subresource_range;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
		}
	}

	VO_TRY(_renderer.SubmitCommandBufferSimple(cmd_buf, _renderer._graphicsQueue));

	VkDescriptorImageInfo desc_img_info =
	{
		.sampler = tex_sampler,
		.imageView = imv_blue_noise,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet s =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = desc_set_textures_even,
		.dstBinding = BINDING_OFFSET_BLUE_NOISE,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &desc_img_info,
	};

	vkUpdateDescriptorSets(device, 1, &s, 0, nullptr);

	s.dstSet = desc_set_textures_odd;
	vkUpdateDescriptorSets(device, 1, &s, 0, nullptr);

	vkQueueWaitIdle(_renderer._graphicsQueue);

	return true;
}

bool Textures::InitHeightmap()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	const World* world = World::s_instance;
	VO_TRY(world);
	const Terrain* terrain = world->GetTerrain();
	VO_TRY(terrain);

	const std::vector<uint16>& data = terrain->GetHeightmapData();
	size_t img_size = data.size();
	size_t total_size = img_size * sizeof(uint16_t);

	Buffer::Setup buf_img_upload_setup;
	buf_img_upload_setup._size = total_size;
	buf_img_upload_setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buf_img_upload_setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	bool result;
	Buffer buf_img_upload(_renderer, buf_img_upload_setup, result);
	VO_TRY(result);

	uint16* mapped = (uint16*)buf_img_upload.Map();
	memcpy(mapped, data.data(), total_size);
	buf_img_upload.Unmap();

	const glm::ivec3& size = terrain->Size();

	VkImageCreateInfo img_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_R16_UNORM,
		.extent = {
			.width = (uint32)size.x,
			.height = (uint32)size.z,
			.depth = 1,
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_STORAGE_BIT
							   | VK_IMAGE_USAGE_TRANSFER_DST_BIT
							   | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VO_TRY_VK(vkCreateImage(device, &img_info, nullptr, &_img_heightmap));
	_renderer.SetObjectName(_img_heightmap, "Heightmap");

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(device, _img_heightmap, &mem_req);
	VO_ASSERT(mem_req.size >= buf_img_upload.GetSize());

	VO_TRY(_renderer.AllocateGPUMemory(mem_req, &_mem_heightmap));

	VO_TRY_VK(vkBindImageMemory(device, _img_heightmap, _mem_heightmap, 0));

	VkImageViewCreateInfo img_view_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = _img_heightmap,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_R16_UNORM,
		.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_R,
		},
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	VO_TRY_VK(vkCreateImageView(device, &img_view_info, nullptr, &_imv_heightmap));
	_renderer.SetObjectName(_imv_heightmap, "Heightmap");

	VkCommandBuffer cmd_buf = _renderer.BeginCommandBuffer(_renderer._graphicsCommandBuffers);

	VkImageSubresourceRange subresource_range = 
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	{
		VkImageMemoryBarrier barrier = IMAGE_BARRIER();
		barrier.image = _img_heightmap;
		barrier.subresourceRange = subresource_range;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
	}

	VkBufferImageCopy cpy_info =
	{
		.bufferOffset = 0,
		.imageSubresource =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = { 0, 0, 0 },
		.imageExtent = { (uint32)size.x, (uint32)size.z, 1 }
	};
	vkCmdCopyBufferToImage(cmd_buf, buf_img_upload.GetBuffer(), _img_heightmap,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy_info);

	{
		VkImageMemoryBarrier barrier = IMAGE_BARRIER();
		barrier.image = _img_heightmap;
		barrier.subresourceRange = subresource_range;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
	}

	VO_TRY(_renderer.SubmitCommandBufferSimple(cmd_buf, _renderer._graphicsQueue));
	VO_TRY(UpdateDescriptorSet(BINDING_OFFSET_HEIGHTMAP, _imv_heightmap, tex_sampler));
	return true;
}

bool Textures::InitTerrainDiffuse()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	World* world = World::s_instance;
	VO_TRY(world);

	const Terrain* terrain = world->GetTerrain();
	const ResourceHandle<Texture>& diffuseRes = terrain->GetDiffuseTex();
	const Texture* texture = diffuseRes.Get();
	VO_TRY(texture);
	VO_TRY(texture->GetNbComponents() == 4);

	const uint32 width = texture->GetWidth();
	const uint32 height = texture->GetHeigth();
	size_t total_size = texture->GetBufferSize8();

	Buffer::Setup buf_img_upload_setup;
	buf_img_upload_setup._size = total_size;
	buf_img_upload_setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buf_img_upload_setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	bool result;
	Buffer buf_img_upload(_renderer, buf_img_upload_setup, result);
	VO_TRY(result);

	// Copy content
	{
		uint8* bn_tex = (uint8*)buf_img_upload.Map();
		memcpy(bn_tex, texture->GetBuffer(), total_size);
		buf_img_upload.Unmap();
		bn_tex = nullptr;
	}

	VkImageCreateInfo img_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.extent = {
			.width = (uint32)width,
			.height = (uint32)height,
			.depth = 1,
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_STORAGE_BIT
							   | VK_IMAGE_USAGE_TRANSFER_DST_BIT
							   | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VO_TRY_VK(vkCreateImage(device, &img_info, nullptr, &_img_terrainDiffuse));
	_renderer.SetObjectName(_img_terrainDiffuse, "TerrainDiffuse");

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(device, _img_terrainDiffuse, &mem_req);
	VO_ASSERT(mem_req.size >= buf_img_upload.GetSize());

	VO_TRY(_renderer.AllocateGPUMemory(mem_req, &_mem_terrainDiffuse));

	VO_TRY_VK(vkBindImageMemory(device, _img_terrainDiffuse, _mem_terrainDiffuse, 0));

	VkImageViewCreateInfo img_view_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = _img_terrainDiffuse,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A,
		},
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	VO_TRY_VK(vkCreateImageView(device, &img_view_info, nullptr, &_imv_terrainDiffuse));
	_renderer.SetObjectName(_imv_terrainDiffuse, "TerrainDiffuse");

	VkCommandBuffer cmd_buf = _renderer.BeginCommandBuffer(_renderer._graphicsCommandBuffers);

	VkImageSubresourceRange subresource_range =
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	{
		VkImageMemoryBarrier barrier = IMAGE_BARRIER();
		barrier.image = _img_terrainDiffuse;
		barrier.subresourceRange = subresource_range;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
	}

	VkBufferImageCopy cpy_info =
	{
		.bufferOffset = 0,
		.imageSubresource =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = { 0, 0, 0 },
		.imageExtent = { (uint32)width, (uint32)height, 1 }
	};
	vkCmdCopyBufferToImage(cmd_buf, buf_img_upload.GetBuffer(), _img_terrainDiffuse,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy_info);

	{
		VkImageMemoryBarrier barrier = IMAGE_BARRIER();
		barrier.image = _img_terrainDiffuse;
		barrier.subresourceRange = subresource_range;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
	}

	VO_TRY(_renderer.SubmitCommandBufferSimple(cmd_buf, _renderer._graphicsQueue));
	VO_TRY(UpdateDescriptorSet(BINDING_OFFSET_TERRAIN_DIFFUSE, _imv_terrainDiffuse, tex_sampler));
	return true;
}

bool Textures::InitSamplers()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	{
		VkSamplerCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = 16,
			.minLod = 0.0f,
			.maxLod = 128.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VO_TRY_VK(vkCreateSampler(device, &info, nullptr, &tex_sampler));
		_renderer.SetObjectName(tex_sampler, "Tex");
	}

	{
		VkSamplerCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_NEAREST,
			.minFilter = VK_FILTER_NEAREST,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.anisotropyEnable = VK_FALSE,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VO_TRY_VK(vkCreateSampler(device, &info, nullptr, &tex_sampler_nearest));
		_renderer.SetObjectName(tex_sampler_nearest, "Nearest");
	}

	{
		VkSamplerCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_NEAREST,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = 16,
			.minLod = 0.0f,
			.maxLod = 128.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VO_TRY_VK(vkCreateSampler(device, &info, nullptr, &tex_sampler_nearest_mipmap_aniso));
		_renderer.SetObjectName(tex_sampler_nearest_mipmap_aniso, "NearestMipmapAniso");
	}

	{
		VkSamplerCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.anisotropyEnable = VK_FALSE,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VO_TRY_VK(vkCreateSampler(device, &info, nullptr, &tex_sampler_linear_clamp_edge));
		_renderer.SetObjectName(tex_sampler_linear_clamp_edge, "LinearClamp");
	}

	{
		VkSamplerCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.anisotropyEnable = VK_FALSE,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VO_TRY_VK(vkCreateSampler(device, &info, nullptr, &tex_sampler_linear_clamp_border));
		_renderer.SetObjectName(tex_sampler_linear_clamp_border, "LinearClampBorder");
	}

	return true;
}

bool Textures::InitFramebufferImages()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkImageCreateInfo images_create_info[NB_IMAGES] = {};
	for (VkImageCreateInfo& info : images_create_info)
	{
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

#define IMG_DO(_name, _binding, _vkformat, _glslformat, _w, _h) \
	{ \
		VkImageCreateInfo& info = images_create_info[(int)ImageID::_name]; \
		info.format = VK_FORMAT_##_vkformat; \
		info.extent = { .width  = _w, .height = _h, .depth  = 1 }; \
	}
	LIST_IMAGES
		LIST_IMAGES_A_B
#undef IMG_DO

		size_t total_size = 0;

	for (int i = 0; i < NB_IMAGES; i++)
	{
		VO_TRY_VK(vkCreateImage(device, &images_create_info[i], nullptr, &images[i]));
		//SET_VK_NAME(images[i], std::format("Image");

		VkMemoryRequirements mem_req;
		vkGetImageMemoryRequirements(device, images[i], &mem_req);

		total_size += AlignUp(mem_req.size, mem_req.alignment);

		VO_TRY(_renderer.AllocateGPUMemory(mem_req, &mem_images[i]));

		VO_TRY_VK(vkBindImageMemory(device, images[i], mem_images[i], 0));
	}

	VkDeviceSize video_memory_size = _renderer.GetAvailableVideoMemory();
	if (total_size > video_memory_size / 2)
	{
		VO_ERROR("Warning: The renderer uses {}MB for internal screen-space resources, which is\n"
			"more than half of the available video memory ({}MB). This may cause poor performance.\n"
			"Consider limiting the maximum dynamic resolution scale, using a lower fixed resolution\n"
			"scale, or lowering the output resolution.\n",
			(float)total_size / (1024 * 1024), (float)video_memory_size / (1024 * 1024));
	}
	else
	{
		VO_INFO("Screen-space image memory: {}MB\n", (float)total_size / (1024 * 1024));
	}

	// Attach labels to images
#define IMG_DO(_name, _binding, ...) _renderer.SetObjectName(images[(int)ImageID::_name], #_name);
	LIST_IMAGES
	LIST_IMAGES_A_B
#undef IMG_DO

		VkImageViewCreateInfo images_view_create_info[NB_IMAGES]{};
	for (VkImageViewCreateInfo& info : images_view_create_info)
	{
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		};
		info.components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
	}
#define IMG_DO(_name, _binding, _vkformat, _glslformat, _w, _h) \
	{ \
		VkImageViewCreateInfo& info = images_view_create_info[(int)ImageID::_name]; \
		info.format = VK_FORMAT_##_vkformat; \
		info.image = images[(int)ImageID::_name]; \
	}
	LIST_IMAGES
	LIST_IMAGES_A_B
#undef IMG_DO

		for (int i = 0; i < NB_IMAGES; i++)
			VO_TRY_VK(vkCreateImageView(device, &images_view_create_info[i], nullptr, &images_views[i]));

	// Attach labels
#define IMG_DO(_name, ...) _renderer.SetObjectName(images_views[(int)ImageID::_name], #_name);
	LIST_IMAGES
	LIST_IMAGES_A_B
#undef IMG_DO

	VkDescriptorImageInfo desc_output_img_info[NB_IMAGES] = {};
	VkDescriptorImageInfo img_info[NB_IMAGES] = {};
	for (int i = 0; i < NB_IMAGES; ++i)
	{
		{
			VkDescriptorImageInfo& info = desc_output_img_info[i];
			info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			info.imageView = images_views[i];
		}
		{
			VkDescriptorImageInfo& info = img_info[i];
			info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			info.imageView = images_views[i];
			info.sampler = tex_sampler_nearest;
		}
	}

	/*
	for (int i = VKPT_IMG_BLOOM_HBLUR; i <= VKPT_IMG_BLOOM_VBLUR; i++)
	{
		img_info[i].sampler = qvk.tex_sampler_linear_clamp_edge;
	}
	*/
	img_info[(int)ImageID::ASVGF_TAA_A].sampler = tex_sampler;
	img_info[(int)ImageID::ASVGF_TAA_B].sampler = tex_sampler;
	img_info[(int)ImageID::TAA_OUTPUT].sampler = tex_sampler;

	VkWriteDescriptorSet output_img_write[NB_IMAGES * 2] = {};

	for (int even_odd = 0; even_odd < 2; even_odd++)
	{
		/* create information to update descriptor sets */
#define IMG_DO(_name, _binding, ...) { \
			VkWriteDescriptorSet elem_image = { \
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, \
				.dstSet          = even_odd ? desc_set_textures_odd : desc_set_textures_even, \
				.dstBinding      = BINDING_OFFSET_IMAGES + _binding, \
				.dstArrayElement = 0, \
				.descriptorCount = 1, \
				.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, \
				.pImageInfo      = &desc_output_img_info[(int)ImageID::_name], \
			}; \
			VkWriteDescriptorSet elem_texture = { \
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, \
				.dstSet          = even_odd ? desc_set_textures_odd : desc_set_textures_even, \
				.dstBinding      = BINDING_OFFSET_TEXTURES + _binding, \
				.dstArrayElement = 0, \
				.descriptorCount = 1, \
				.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, \
				.pImageInfo      = &img_info[(int)ImageID::_name], \
			}; \
			output_img_write[(int)ImageID::_name] = elem_image; \
			output_img_write[(int)ImageID::_name + NB_IMAGES] = elem_texture; \
		}
		LIST_IMAGES;
		if (even_odd)
		{
			LIST_IMAGES_B_A;
		}
		else
		{
			LIST_IMAGES_A_B;
		}
#undef IMG_DO

		vkUpdateDescriptorSets(device, std::size(output_img_write), output_img_write, 0, nullptr);
	}

	VkCommandBuffer cmd_buf = _renderer.BeginCommandBuffer(_renderer._graphicsCommandBuffers);

	VkImageSubresourceRange subresource_range =
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	for (int i = 0; i < NB_IMAGES; i++)
	{
		VkImageMemoryBarrier barrier = IMAGE_BARRIER();
		barrier.image = images[i];
		barrier.subresourceRange = subresource_range;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		QUEUE_IMAGE_BARRIER(cmd_buf, barrier);
	}

	VkQueue graphicsQueue = _renderer._graphicsQueue;
	VO_TRY(_renderer.SubmitCommandBufferSimple(cmd_buf, graphicsQueue));

	vkQueueWaitIdle(graphicsQueue);
	return true;
}

void Textures::ShutdownFramebufferImages()
{
	const VkDevice device = _renderer.GetDevice();

	for (int i = 0; i < NB_IMAGES; i++)
	{
		if (device)
		{
			vkDestroyImageView(device, images_views[i], nullptr);
			vkDestroyImage(device, images[i], nullptr);
		}

		images_views[i] = VK_NULL_HANDLE;
		images[i] = VK_NULL_HANDLE;

		if (mem_images[i])
		{
			if (device)
				vkFreeMemory(device, mem_images[i], nullptr);
			mem_images[i] = VK_NULL_HANDLE;
		}
	}
}

void Textures::QueueImageBarrier(VkCommandBuffer commands, ImageID imageID, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	VkImageSubresourceRange subresource_range =
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = GetImage(imageID);
	barrier.subresourceRange = subresource_range;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

bool Textures::RegisterTerrainTextures()
{
	VO_TRY(InitHeightmap());
	VO_TRY(InitTerrainDiffuse());
	return true;
}

void Textures::UnregisterTerrainTextures()
{
	VO_ASSERT(false);
}

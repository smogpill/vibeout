// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Bloom.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Shared/Buffers.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Shared/Textures.h"
#include "Vibeout/Render/Shared/Shaders.h"

struct PushConstants
{
	uint32 _inputWidth;
	uint32 _inputHeight;
	uint32 _outputWidth;
	uint32 _outputHeight;
};

const float Bloom::s_defaultIntensity = 0.15f;
const float Bloom::s_menuIntensity = 1.0f;
const float Bloom::s_waterIntensity = 1.0f;
const int Bloom::s_minResForBloomSampling = 4;

Bloom::Bloom(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

Bloom::~Bloom()
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	if (_downsamplePipelineLayout)
		vkDestroyPipelineLayout(device, _downsamplePipelineLayout, nullptr);
	if (_upsamplePipelineLayout)
		vkDestroyPipelineLayout(device, _upsamplePipelineLayout, nullptr);
	if (_mixPipelineLayout)
		vkDestroyPipelineLayout(device, _mixPipelineLayout, nullptr);
	if (_descSetLayout1)
		vkDestroyDescriptorSetLayout(device, _descSetLayout1, nullptr);
	if (_descSetLayout2)
		vkDestroyDescriptorSetLayout(device, _descSetLayout2, nullptr);
}

bool Bloom::Init()
{
	VO_TRY(InitDescriptorSetLayouts());
	VO_TRY(InitPipelineLayouts());
	return true;
}

bool Bloom::InitDescriptorSetLayouts()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkDescriptorSetLayoutBinding layoutBindings1[] =
	{
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		}
	};

	VkDescriptorSetLayoutBinding layoutBindings2[] =
	{
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		},
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		}
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo1 = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo1.bindingCount = std::size(layoutBindings1);
	layoutInfo1.pBindings = layoutBindings1;

	VkDescriptorSetLayoutCreateInfo layoutInfo2 = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo2.bindingCount = std::size(layoutBindings2);
	layoutInfo2.pBindings = layoutBindings2;

	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layoutInfo1, nullptr, &_descSetLayout1));
	_renderer.SetObjectName(_descSetLayout1, "Bloom1");
	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layoutInfo2, nullptr, &_descSetLayout2));
	_renderer.SetObjectName(_descSetLayout2, "Bloom2");
	return true;
}

bool Bloom::InitPipelineLayouts()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	VkDescriptorSetLayout desc_set_layouts1[] =
	{
		buffers->GetDescriptorSetLayout(),
		textures->GetDescSetLayout(),
		_descSetLayout1
	};
	VkDescriptorSetLayout desc_set_layouts2[] =
	{
		buffers->GetDescriptorSetLayout(),
		textures->GetDescSetLayout(),
		_descSetLayout2
	};

	for (VkDescriptorSetLayout& layout : desc_set_layouts1)
		VO_TRY(layout);
	for (VkDescriptorSetLayout& layout : desc_set_layouts2)
		VO_TRY(layout);

	VkPushConstantRange push_constant_range =
	{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = sizeof(PushConstants)
	};

	{
		VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		info.setLayoutCount = std::size(desc_set_layouts1);
		info.pSetLayouts = desc_set_layouts1;
		info.pushConstantRangeCount = 1;
		info.pPushConstantRanges = &push_constant_range;
		VO_TRY_VK(vkCreatePipelineLayout(device, &info, nullptr, &_downsamplePipelineLayout));
		_renderer.SetObjectName(_downsamplePipelineLayout, "Downsample");
	}

	{
		VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		info.setLayoutCount = std::size(desc_set_layouts2);
		info.pSetLayouts = desc_set_layouts2;
		info.pushConstantRangeCount = 1;
		info.pPushConstantRanges = &push_constant_range;
		VO_TRY_VK(vkCreatePipelineLayout(device, &info, nullptr, &_upsamplePipelineLayout));
		_renderer.SetObjectName(_upsamplePipelineLayout, "Upsample");
	}

	{
		VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		info.setLayoutCount = std::size(desc_set_layouts1);
		info.pSetLayouts = desc_set_layouts1;
		VO_TRY_VK(vkCreatePipelineLayout(device, &info, nullptr, &_mixPipelineLayout));
		_renderer.SetObjectName(_mixPipelineLayout, "Mix");
	}
	return true;
}

void Bloom::Reset()
{
	_intensity = s_defaultIntensity;
}

void Bloom::Update(GlobalUniformBuffer& ubo, float deltaTime, bool under_water, bool menu_mode)
{
	if (under_water)
	{
		_underwaterAnimation = glm::min(1.f, _underwaterAnimation + deltaTime * 3.f);
		_intensity = s_waterIntensity;
	}
	else
	{
		_underwaterAnimation = glm::max(0.f, _underwaterAnimation - deltaTime * 3.f);
		_intensity = std::lerp(s_defaultIntensity, s_waterIntensity, _underwaterAnimation);
	}

	if (menu_mode)
	{
		/*
		if (menu_start_ms == 0)
			menu_start_ms = Sys_Milliseconds();
		uint32_t current_ms = Sys_Milliseconds();

		float phase = max(0.f, min(1.f, (float)(current_ms - menu_start_ms) / 150.f));
		ubo->tonemap_hdr_clamp_strength = phase; // Clamp color in HDR mode, to ensure menu is legible
		phase = powf(phase, 0.25f);

		bloom_sigma = phase * 0.03f;

		*/

		ubo._bloom2_intensity = s_menuIntensity;
	}
	else
	{
		ubo._bloom2_intensity = _intensity;
	}
}

bool Bloom::InitFramebufferRelated()
{
	_nbPasses = 0;
	_imageSizes.clear();
	const VkExtent2D extent = _renderer._extentTAAImages;
	glm::uvec2 size(extent.width, extent.height);
	int maxSize = glm::max(extent.width, extent.height);
	while (maxSize > s_minResForBloomSampling)
	{
		maxSize /= 2;
		size /= 2u;
		_imageSizes.push_back(size);
		++_nbPasses;
	}

	VO_TRY(InitImages());
	VO_TRY(InitDescriptorSets());
	VO_TRY(InitPipelines());
	return true;
}

void Bloom::ShutdownFramebufferRelated()
{
	ShutdownPipelines();
	ShutdownDescriptorSets();
	ShutdownImages();
	_nbPasses = 0;
}

bool Bloom::InitImages()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);

	const VkExtent2D frameExtents = _renderer._extentTAAImages;

	VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	const VkImageSubresourceRange subresource_range =
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.levelCount = 1,
		.layerCount = 1
	};

	VkImageViewCreateInfo image_view_create_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = imageCreateInfo.format,
		.subresourceRange = subresource_range
	};

	VO_ASSERT(_downsampleImages.empty());
	VO_ASSERT(_upsampleImages.empty());
	VO_ASSERT(_ownedImages.empty());
	VO_ASSERT(_downsampleImageViews.empty());
	VO_ASSERT(_upsampleImageViews.empty());
	VO_ASSERT(_deviceMemories.empty());

	for (int pass = 0; pass < _nbPasses; ++pass)
	{
		imageCreateInfo.extent = { _imageSizes[pass].x, _imageSizes[pass].y, 1u };

		{
			VkImage image;
			VO_TRY_VK(vkCreateImage(device, &imageCreateInfo, nullptr, &image));
			_renderer.SetObjectName(image, std::format("Bloom::Downsample{}", pass).c_str());
			_ownedImages.push_back(image);

			VkMemoryRequirements memReq;
			vkGetImageMemoryRequirements(device, image, &memReq);

			VkDeviceMemory deviceMemory;
			VO_TRY(_renderer.AllocateGPUMemory(memReq, &deviceMemory));
			_deviceMemories.push_back(deviceMemory);
			VO_TRY_VK(vkBindImageMemory(device, image, deviceMemory, 0));

			_downsampleImages.push_back(image);
			image_view_create_info.image = image;

			VkImageView imageView;
			VO_TRY_VK(vkCreateImageView(device, &image_view_create_info, nullptr, &imageView));
			_renderer.SetObjectName(imageView, std::format("Bloom::Downsample{}", pass).c_str());
			_downsampleImageViews.push_back(imageView);
		}

		if (pass < _nbPasses - 1)
		{
			VkImage image;

			if (pass)
			{
				VO_TRY_VK(vkCreateImage(device, &imageCreateInfo, nullptr, &image));
				_renderer.SetObjectName(image, std::format("Bloom::Upsample{}", pass).c_str());
				_ownedImages.push_back(image);

				VkMemoryRequirements mem_req;
				vkGetImageMemoryRequirements(device, image, &mem_req);

				VkDeviceMemory deviceMemory;
				VO_TRY(_renderer.AllocateGPUMemory(mem_req, &deviceMemory));
				_deviceMemories.push_back(deviceMemory);
				VO_TRY_VK(vkBindImageMemory(device, image, deviceMemory, 0));
			}
			else
			{
				image = textures->GetImage(ImageID::BLOOM);
			}

			_upsampleImages.push_back(image);
			image_view_create_info.image = image;

			VkImageView imageView;
			VO_TRY_VK(vkCreateImageView(device, &image_view_create_info, nullptr, &imageView));
			_renderer.SetObjectName(imageView, std::format("Bloom::Upsample{}", pass).c_str());
			_upsampleImageViews.push_back(imageView);
		}
	}

	return true;
}

bool Bloom::InitDescriptorSets()
{
	const VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	const Textures* textures = _renderer._textures;
	VO_TRY(textures);

	// Create the desc pool
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{
				.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = uint32(_nbPasses + (_nbPasses - 1) * 2 + 1)
			},
			{
				.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = uint32(_nbPasses + _nbPasses - 1 + 1)
			}
		};

		VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.maxSets = uint32(_nbPasses + _nbPasses - 1 + 1);
		poolInfo.poolSizeCount = std::size(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		VO_TRY_VK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descPool));
		_renderer.SetObjectName(_descPool, "Bloom");
	}

	// Allocate the desc sets
	{
		VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocateInfo.descriptorPool = _descPool;

		// Downsample
		{
			const int nb = _nbPasses;
			std::vector<VkDescriptorSetLayout> layouts;
			layouts.resize(nb, _descSetLayout1);
			allocateInfo.descriptorSetCount = nb;
			allocateInfo.pSetLayouts = layouts.data();
			_downsampleDescriptorSets.resize(nb, nullptr);
			VO_TRY_VK(vkAllocateDescriptorSets(device, &allocateInfo, _downsampleDescriptorSets.data()));

			for (int pass = 0; pass < nb; ++pass)
			{
				_renderer.SetObjectName(_downsampleDescriptorSets[pass], std::format("Bloom::Downsample{}", pass).c_str());
			}
		}

		// Upsample
		{
			const int nb = _nbPasses - 1;
			std::vector<VkDescriptorSetLayout> layouts;
			layouts.resize(nb, _descSetLayout2);
			allocateInfo.descriptorSetCount = nb;
			allocateInfo.pSetLayouts = layouts.data();
			_upsampleDescriptorSets.resize(nb, nullptr);
			VO_TRY_VK(vkAllocateDescriptorSets(device, &allocateInfo, _upsampleDescriptorSets.data()));

			for (int pass = 0; pass < nb; ++pass)
			{
				_renderer.SetObjectName(_upsampleDescriptorSets[pass], std::format("Bloom::Upsample{}", pass).c_str());
			}
		}

		// Mix
		{
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &_descSetLayout1;
			VO_TRY_VK(vkAllocateDescriptorSets(device, &allocateInfo, &_mixDescriptorSet));
			_renderer.SetObjectName(_mixDescriptorSet, "Bloom::Mix");
		}
	}

	// Write desc sets
	{
		const VkSampler clampToBorderSampler = textures->GetClampToBorderSampler();
		VO_TRY(clampToBorderSampler);
		const VkSampler clampToEdgeSampler = textures->GetClampToEdgeSampler();
		VO_TRY(clampToEdgeSampler);

		std::vector<VkDescriptorImageInfo> imageInfos;
		std::vector<VkWriteDescriptorSet> writes;
		const size_t nbWrites = _downsampleDescriptorSets.size() * 2 + _upsampleDescriptorSets.size() * 3 + 2;
		imageInfos.resize(nbWrites, {});
		writes.resize(nbWrites, {});

		for (VkDescriptorImageInfo& info : imageInfos)
		{
			info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		for (VkWriteDescriptorSet& write : writes)
		{
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
		}

		int imageOffset = 0;
		int writeOffset = 0;

		VkImageView sourceImageView = textures->GetImageView(ImageID::TAA_OUTPUT);
		VkImageView previousImageView = sourceImageView;

		// Downsample
		{
			for (int pass = 0; pass < _nbPasses; ++pass)
			{
				VkDescriptorSet descSet = _downsampleDescriptorSets[pass];

				// Input
				{
					VkDescriptorImageInfo& imageInfo = imageInfos[imageOffset++];
					imageInfo.sampler = clampToBorderSampler;
					imageInfo.imageView = previousImageView;

					VkWriteDescriptorSet& write = writes[writeOffset++];
					write.dstSet = descSet;
					write.dstBinding = 0;
					write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					write.pImageInfo = &imageInfo;
				}

				// Output
				{
					VkDescriptorImageInfo& imageInfo = imageInfos[imageOffset++];
					imageInfo.imageView = _downsampleImageViews[pass];

					VkWriteDescriptorSet& write = writes[writeOffset++];
					write.dstSet = descSet;
					write.dstBinding = 1;
					write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					write.pImageInfo = &imageInfo;

					previousImageView = imageInfo.imageView;
				}
			}
		}

		// Upsample
		{
			const int nbUpsamplePasses = _nbPasses - 1;
			for (int pass = 0; pass < nbUpsamplePasses; ++pass)
			{
				VkDescriptorSet descSet = _upsampleDescriptorSets[pass];

				// Input 1
				{
					VkDescriptorImageInfo& imageInfo = imageInfos[imageOffset++];
					imageInfo.sampler = clampToEdgeSampler;
					imageInfo.imageView = previousImageView;

					VkWriteDescriptorSet& write = writes[writeOffset++];
					write.dstSet = descSet;
					write.dstBinding = 0;
					write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					write.pImageInfo = &imageInfo;
				}

				const int imageIndex = _nbPasses - 2 - pass;

				// Input 2
				{
					VkDescriptorImageInfo& imageInfo = imageInfos[imageOffset++];
					imageInfo.sampler = clampToEdgeSampler;
					imageInfo.imageView = _downsampleImageViews[imageIndex];

					VkWriteDescriptorSet& write = writes[writeOffset++];
					write.dstSet = descSet;
					write.dstBinding = 1;
					write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					write.pImageInfo = &imageInfo;
				}

				// Output
				{
					VkDescriptorImageInfo& imageInfo = imageInfos[imageOffset++];
					imageInfo.imageView = _upsampleImageViews[imageIndex];

					VkWriteDescriptorSet& write = writes[writeOffset++];
					write.dstSet = descSet;
					write.dstBinding = 2;
					write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					write.pImageInfo = &imageInfo;

					previousImageView = imageInfo.imageView;
				}
			}
		}

		// Mix
		{
			// Input
			{
				VkDescriptorImageInfo& imageInfo = imageInfos[imageOffset++];
				imageInfo.sampler = clampToEdgeSampler;
				imageInfo.imageView = _upsampleImageViews[0];

				VkWriteDescriptorSet& write = writes[writeOffset++];
				write.dstSet = _mixDescriptorSet;
				write.dstBinding = 0;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.pImageInfo = &imageInfo;
			}

			// Output
			{
				VkDescriptorImageInfo& imageInfo = imageInfos[imageOffset++];
				imageInfo.imageView = sourceImageView;

				VkWriteDescriptorSet& write = writes[writeOffset++];
				write.dstSet = _mixDescriptorSet;
				write.dstBinding = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				write.pImageInfo = &imageInfo;
			}
		}

		vkUpdateDescriptorSets(device, (uint32)writes.size(), writes.data(), 0, nullptr);
	}

	return true;
}

bool Bloom::InitPipelines()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkComputePipelineCreateInfo pipeline_info[(int)PipelineID::END] = {};
	for (VkComputePipelineCreateInfo& info : pipeline_info)
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::DOWNSAMPLE];
		info.stage = SHADER_STAGE("BloomDownsample.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _downsamplePipelineLayout;
	}
	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::UPSAMPLE_COMBINE];
		info.stage = SHADER_STAGE("BloomUpsampleCombine.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _upsamplePipelineLayout;
	}
	{
		VkComputePipelineCreateInfo& info = pipeline_info[(int)PipelineID::MIX];
		info.stage = SHADER_STAGE("BloomMix.comp", VK_SHADER_STAGE_COMPUTE_BIT);
		info.layout = _mixPipelineLayout;
	}

	VO_TRY_VK(vkCreateComputePipelines(device, 0, std::size(pipeline_info), pipeline_info, nullptr, _pipelines));
	return true;
}

void Bloom::ShutdownPipelines()
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	for (VkPipeline& pipeline : _pipelines)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		pipeline = nullptr;
	}
}

void Bloom::ShutdownDescriptorSets()
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	_downsampleDescriptorSets.clear();
	_upsampleDescriptorSets.clear();
	_mixDescriptorSet = nullptr;
	if (_descPool)
	{
		vkDestroyDescriptorPool(device, _descPool, nullptr);
		_descPool = nullptr;
	}
}

void Bloom::ShutdownImages()
{
	VkDevice device = _renderer.GetDevice();
	VO_ASSERT(device);
	for (const VkImageView& imageView : _downsampleImageViews)
		vkDestroyImageView(device, imageView, nullptr);
	for (const VkImageView& imageView : _upsampleImageViews)
		vkDestroyImageView(device, imageView, nullptr);
	for (const VkImage& image : _ownedImages)
		vkDestroyImage(device, image, nullptr);
	for (const VkDeviceMemory& deviceMemory : _deviceMemories)
		vkFreeMemory(device, deviceMemory, nullptr);
	_downsampleImageViews.clear();
	_upsampleImageViews.clear();
	_downsampleImages.clear();
	_upsampleImages.clear();
	_ownedImages.clear();
	_deviceMemories.clear();
}

#define BARRIER_COMPUTE(_commands_, _img_) \
	do { \
		VkImageSubresourceRange subresource_range = { \
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT, \
			.baseMipLevel   = 0, \
			.levelCount     = 1, \
			.baseArrayLayer = 0, \
			.layerCount     = 1 \
		}; \
		VkImageMemoryBarrier barrier = IMAGE_BARRIER(); \
		barrier.image            = _img_; \
		barrier.subresourceRange = subresource_range; \
		barrier.srcAccessMask    = VK_ACCESS_SHADER_WRITE_BIT; \
		barrier.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT; \
		barrier.oldLayout        = VK_IMAGE_LAYOUT_GENERAL; \
		barrier.newLayout        = VK_IMAGE_LAYOUT_GENERAL; \
		QUEUE_IMAGE_BARRIER(_commands_, barrier); \
	} while(0)

bool Bloom::RecordCommandBuffer(VkCommandBuffer commands)
{
	VO_SCOPE_VK_CMD_LABEL(commands, "Bloom");
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	Textures* textures = _renderer._textures;
	VO_TRY(textures);
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);

	//glm::uvec2 sourceSize(_renderer._extent_taa_output.width, _renderer._extent_taa_output.height);
	const glm::uvec2 sourceSize(_renderer._extentTAAImages.width, _renderer._extentTAAImages.height);

	VkDescriptorSet desc_sets[3] =
	{
		buffers->GetDescriptorSet(),
		textures->GetCurrentDescSet(),
		nullptr
	};

	{
		PushConstants pushConstants;
		glm::uvec2 previousSize = sourceSize;

		// Downsample
		{
			VO_SCOPE_VK_CMD_LABEL(commands, "Downsample");
			QueueImageBarrier(commands, textures->GetImage(ImageID::TAA_OUTPUT), VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::DOWNSAMPLE]);

			for (int pass = 0; pass < _nbPasses; ++pass)
			{
				desc_sets[2] = _downsampleDescriptorSets[pass];
				vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _downsamplePipelineLayout, 0, std::size(desc_sets), desc_sets, 0, 0);

				const glm::uvec2 targetSize = _imageSizes[pass];
				pushConstants._inputWidth = previousSize.x;
				pushConstants._inputHeight = previousSize.y;
				pushConstants._outputWidth = targetSize.x;
				pushConstants._outputHeight = targetSize.y;
				vkCmdPushConstants(commands, _downsamplePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushConstants), &pushConstants);
				vkCmdDispatch(commands, (pushConstants._outputWidth + 15) / 16, (pushConstants._outputHeight + 15) / 16, 1);

				QueueImageBarrier(commands, _downsampleImages[pass], VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

				previousSize = targetSize;
			}
		}

		// Upsample
		{
			VO_SCOPE_VK_CMD_LABEL(commands, "Upsample");
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::UPSAMPLE_COMBINE]);


			//const vec3 tintNear = uvec3(1.0f);
			//const vec3 tintFar = uvec3(1.0f);
			//const vec3 tintFar = coColor(236, 226, 255);
			//const vec3 tintFar = coColor(226, 216, 255);
			//const vec3 tintFar = coColor(219, 216, 255);
			for (int pass = 0; pass < _nbPasses - 1; ++pass)
			{
				//const float tintRatio = float(pass + 1) / float(_nbBloomPasses - 1);
				//const vec3 tint = coLerp(tintNear, tintFar, vec3(tintRatio));

				desc_sets[2] = _upsampleDescriptorSets[pass];
				vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _upsamplePipelineLayout, 0, std::size(desc_sets), desc_sets, 0, 0);

				const int targetImageIdx = _nbPasses - 2 - pass;

				const glm::uvec2 targetSize = _imageSizes[targetImageIdx];
				pushConstants._inputWidth = previousSize.x;
				pushConstants._inputHeight = previousSize.y;
				pushConstants._outputWidth = targetSize.x;
				pushConstants._outputHeight = targetSize.y;
				vkCmdPushConstants(commands, _upsamplePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushConstants), &pushConstants);
				vkCmdDispatch(commands, (pushConstants._outputWidth + 15) / 16, (pushConstants._outputHeight + 15) / 16, 1);

				QueueImageBarrier(commands, _upsampleImages[targetImageIdx], VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

				previousSize = targetSize;
			}
		}

		// Mix
		if (false)
		{
			VO_SCOPE_VK_CMD_LABEL(commands, "Mix");
			QueueImageBarrier(commands, textures->GetImage(ImageID::TAA_OUTPUT), VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelines[(int)PipelineID::MIX]);
			desc_sets[2] = _mixDescriptorSet;
			vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE, _mixPipelineLayout, 0, std::size(desc_sets), desc_sets, 0, 0);
			vkCmdDispatch(commands, (sourceSize.x + 15) / 16, (sourceSize.y + 15) / 16, 1);
		}
	}

	return true;
}

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Renderer;
struct GlobalUniformBuffer;

class Bloom
{
public:
	Bloom(Renderer& renderer, bool& result);
	~Bloom();

	void Reset();
	void Update(GlobalUniformBuffer& ubo, float frameTime, bool underwater, bool menuMode);
	bool InitFramebufferRelated();
	void ShutdownFramebufferRelated();

	bool RecordCommandBuffer(VkCommandBuffer commands);

private:
	static const float s_defaultIntensity;
	static const float s_menuIntensity;
	static const float s_waterIntensity;
	static const int s_minResForBloomSampling;

	bool Init();
	bool InitDescriptorSetLayouts();
	bool InitPipelineLayouts();
	bool InitPipelines();
	bool InitDescriptorSets();
	bool InitImages();
	void ShutdownPipelines();
	void ShutdownDescriptorSets();
	void ShutdownImages();

	enum class PipelineID
	{
		DOWNSAMPLE,
		UPSAMPLE_COMBINE,
		MIX,
		END
	};

	Renderer& _renderer;
	bool _enable = false;
	float _intensity = s_defaultIntensity;
	float _underwaterAnimation = 0.0f;
	int _nbPasses = 0;
	VkPipeline _pipelines[(int)PipelineID::END];
	VkPipelineLayout _downsamplePipelineLayout = nullptr;
	VkPipelineLayout _upsamplePipelineLayout = nullptr;
	VkPipelineLayout _mixPipelineLayout = nullptr;
	VkDescriptorSetLayout _descSetLayout1 = nullptr;
	VkDescriptorSetLayout _descSetLayout2 = nullptr;
	VkDescriptorPool _descPool = nullptr;
	std::vector<VkDescriptorSet> _downsampleDescriptorSets;
	std::vector<VkDescriptorSet> _upsampleDescriptorSets;
	VkDescriptorSet _mixDescriptorSet = nullptr;
	std::vector<VkImage> _downsampleImages;
	std::vector<VkImage> _upsampleImages;
	std::vector<VkImage> _ownedImages;
	std::vector<VkImageView> _downsampleImageViews;
	std::vector<VkImageView> _upsampleImageViews;
	std::vector<VkDeviceMemory> _deviceMemories;
	std::vector<glm::uvec2> _imageSizes;
};

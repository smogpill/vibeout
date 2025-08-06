// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Render/Shared/Base.h"
class Renderer;

class PathTracer
{
public:
	enum class PipelineID
	{
		PRIMARY_RAYS,
		DIRECT_LIGHTING,
		INDIRECT_LIGHTING_0,
		INDIRECT_LIGHTING_1,
		INDIRECT_LIGHTING_END,
		END = INDIRECT_LIGHTING_END
	};

	struct PushConstants
	{
		int _bounce;
	};

	PathTracer(Renderer& renderer, bool& result);

	bool InitPipelines();
	void ShutdownPipelines();
	void DispatchRays(VkCommandBuffer commands, PipelineID pipelineID, PushConstants push, uint32 width, uint32 height, uint32 depth);
	void TracePrimaryRays(VkCommandBuffer commands);
	void TraceLighting(VkCommandBuffer commands, int nbBounces);

private:
	bool Init();
	void SetupPipeline(VkCommandBuffer cmd_buf, VkPipelineBindPoint bind_point, PipelineID pipelineID);

	Renderer& _renderer;
	VkDescriptorPool _descriptorPool = nullptr;
	VkDescriptorSet _descriptorSets[maxFramesInFlight] = {};
	VkDescriptorSetLayout _descriptorSetLayout = nullptr;
	VkPipelineLayout _pipelineLayout = nullptr;
	VkPipeline _pipelines[int(PipelineID::END)] = {};
};

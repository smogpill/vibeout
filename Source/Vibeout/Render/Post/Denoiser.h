// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Render/Shared/Base.h"
class Renderer;

class Denoiser
{
public:
	Denoiser(Renderer& renderer, bool& result);
	~Denoiser();

	bool InitPipelines();
	void ShutdownPipelines();
	bool ASVGF_GradientReproject(VkCommandBuffer commands);
	bool ASVGF_Filter(VkCommandBuffer commands, bool enable_lf);
	bool Compositing(VkCommandBuffer commands);
	bool Interleave(VkCommandBuffer commands);
	bool TAA(VkCommandBuffer commands);

private:
	enum class PipelineID
	{
		GRADIENT_IMAGE,
		GRADIENT_ATROUS,
		GRADIENT_REPROJECT,
		TEMPORAL,
		ATROUS_LF,
		ATROUS_ITER_0,
		ATROUS_ITER_1,
		ATROUS_ITER_2,
		ATROUS_ITER_3,
		TAAU,
		CHECKERBOARD_INTERLEAVE,
		COMPOSITING,
		END
	};

	bool Init();

	Renderer& _renderer;
	VkPipeline _pipelines[(int)PipelineID::END] = {};
	VkPipelineLayout _pipeline_layout_atrous = nullptr;
	VkPipelineLayout _pipeline_layout_general = nullptr;
	VkPipelineLayout _pipeline_layout_taa = nullptr;
};

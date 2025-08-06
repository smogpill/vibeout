// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Renderer;
struct GlobalUniformBuffer;

class ToneMapping
{
public:
	ToneMapping(Renderer& renderer, bool& result);
	~ToneMapping();
	void RequestReset();
	bool InitPipelines();
	void ShutdownPipelines();
	bool RecordCommandBuffer(VkCommandBuffer commands, float frame_time);
	void FillUBO(GlobalUniformBuffer& ubo);

private:
	friend class Renderer;

	enum class PipelineID
	{
		TONE_MAPPING_HISTOGRAM,
		TONE_MAPPING_CURVE,
		TONE_MAPPING_APPLY_SDR,
		TONE_MAPPING_APPLY_HDR,
		END
	};

	bool Init();
	bool Reset(VkCommandBuffer commands);

	Renderer& _renderer;
	VkPipeline _pipelines[(int)PipelineID::END] = {};
	VkPipelineLayout pipeline_layout_tone_mapping_histogram = nullptr;
	VkPipelineLayout pipeline_layout_tone_mapping_curve = nullptr;
	VkPipelineLayout pipeline_layout_tone_mapping_apply = nullptr;
	int reset_required = 1; // If 1, recomputes tone curve based only on this frame 

	/// Defines the white point and where we switch from an identity transform
	/// to a Reinhard transform in the additional tone mapper we apply at the
	/// end of the previous tone mapping pipeline.
	/// Must be between 0 and 1; pixels with luminances above this value have
	/// their RGB values slowly clamped to 1, up to tm_white_point.
	float _kneeStart = 0.6f;

	/// Should be greater than 1; defines those RGB values that get mapped to 1.
	float _kneeWhitePoint = 10.0f;

	float _slopeBlurSigma = 12.0f;
};

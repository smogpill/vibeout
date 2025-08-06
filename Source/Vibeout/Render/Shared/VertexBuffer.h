// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Render/Shared/Base.h"
#include "Shaders/VertexBuffer.h"
class Renderer;
class Buffer;
struct ReadbackBuffer;

class VertexBuffer
{
public:
	VertexBuffer(Renderer& renderer, bool& result);
	~VertexBuffer();

	bool Readback(ReadbackBuffer& dst);
	bool InitPipelines();
	void ShutdownPipelines();
	VkDescriptorSet GetDescSet() const { return _descSet; }
	VkDescriptorSetLayout GetDescSetLayout() const { return _descSetLayout; }
	Buffer* GetToneMapBuffer() const { return _toneMapBuffer; }

private:
	friend class ToneMapping;

	bool Init();

	Renderer& _renderer;
	VkDescriptorPool desc_pool_vertex_buffer = nullptr;
	VkDescriptorSet _descSet = nullptr;
	VkDescriptorSetLayout _descSetLayout = nullptr;
	Buffer* null_buffer = nullptr;
	Buffer* _readbackBuffer = nullptr;
	Buffer* _stagingReadbackBuffers[maxFramesInFlight] = {};
	Buffer* _toneMapBuffer = nullptr;
};

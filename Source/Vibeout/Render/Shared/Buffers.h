// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Render/Shared/Base.h"
#include "Shaders/GlobalUBO.h"
class Buffer;
class Renderer;

class Buffers
{
public:
	enum class BufferID
	{
		UNIFORM,
		TLAS,
		END
	};

	Buffers(Renderer& renderer, bool& result);
	~Buffers();

	void CopyFromStaging(VkCommandBuffer commandBuffer, BufferID bufferID, int nbRegions, const VkBufferCopy* regions);
	const VkDescriptorSet& GetDescriptorSet() const { return _descSet; }
	const VkDescriptorSetLayout& GetDescriptorSetLayout() const { return _descSetLayout; }
	Buffer* GetDeviceBuffer(BufferID bufferID) const { return _bufferGroups[(int)bufferID]._deviceBuffer; }
	void* Map(BufferID bufferID);
	void Unmap(BufferID bufferID);

private:
	bool Init();

	struct BufferGroup
	{
		Buffer* _hostBuffers[maxFramesInFlight] = {};
		Buffer* _deviceBuffer = nullptr;
		size_t _size = 0;
	};

	Renderer& _renderer;
	BufferGroup _bufferGroups[(int)BufferID::END];
	VkDescriptorPool _descPool = nullptr;
	VkDescriptorSetLayout _descSetLayout = nullptr;
	VkDescriptorSet _descSet = nullptr;
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Render/Shared/Base.h"
#include "Shaders/GlobalStorage.h"
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

	void CopyFromStaging(VkCommandBuffer commands);
	const VkDescriptorSet& GetDescriptorSet() const { return _descSet; }
	const VkDescriptorSetLayout& GetDescriptorSetLayout() const { return _descSetLayout; }
	Buffer* GetDeviceBuffer(BufferID bufferID) const { return _deviceBuffers[(int)bufferID]; }
	void* AllocateInStaging(BufferID bufferID, uint32 offset, uint32 size);

private:
	bool Init();
	void UnmapStagingIfMapped();

	struct UploadRequest
	{
		BufferID _bufferID;
		VkBufferCopy _copyVK;
	};

	Renderer& _renderer;
	Buffer* _stagingBuffers[maxFramesInFlight] = {};
	Buffer* _deviceBuffers[(int)BufferID::END] = {};
	VkDescriptorPool _descPool = nullptr;
	VkDescriptorSetLayout _descSetLayout = nullptr;
	VkDescriptorSet _descSet = nullptr;
	uint32 _allocatedInStaging = 0;
	Buffer* _mappedStaging = nullptr;
	void* _mappedStagingPtr = nullptr;
	std::vector<UploadRequest> _uploadRequests;
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Render/Shared/Base.h"
#include "Shaders/VertexBuffer.h"
class Renderer;
class Buffer;
struct ReadbackBuffer;

struct ModelGeometry
{
	uint8* geometry_storage = nullptr;
	VkAccelerationStructureGeometryKHR* geometries = nullptr;
	VkAccelerationStructureBuildRangeInfoKHR* build_ranges = nullptr;
	uint32* prim_counts = nullptr;
	uint32* prim_offsets = nullptr;
	uint32 num_geometries = 0;
	uint32 max_geometries = 0;
	VkAccelerationStructureBuildSizesInfoKHR build_sizes = {};
	VkDeviceSize blas_data_offset = {};
	VkAccelerationStructureKHR accel = {};
	VkDeviceAddress blas_device_address = {};
	VkGeometryInstanceFlagsKHR instance_flags = 0;
	uint32 instance_mask = 0;
	uint32 sbt_offset = 0;
};

struct ModelVBO
{
	Buffer* _buffer = nullptr;
	Buffer* _stagingBuffer = nullptr;
	int registrationSequence = 0;
	ModelGeometry _opaqueGeom;
	ModelGeometry _transparentGeom;
	ModelGeometry _maskedGeom;
	uint64 _vertexDataOffset = 0;
	uint32 _totalNbTris = 0;
	bool _static = false;
};

struct ModelDesc
{
	ModelGeometry _geometry;
};

struct WorldData
{
	std::vector<VboPrimitive> _primitives;
	ModelGeometry _opaqueGeom;
	ModelGeometry _transparentGeom;
	ModelGeometry _maskedGeom;
	std::vector<ModelDesc> _models;
	uint64 _vertexDataOffset = 0;
};

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
	bool BuildWorldData();
	bool UploadWorld();

private:
	static constexpr uint32 s_maxNbModels = 8 * 1024;

	/// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkAccelerationStructureCreateInfoKHR.html
	static constexpr uint32 s_accelStructAlignment = 256;
	
	friend class ToneMapping;

	bool Init();
	bool SuballocateModelBlasMemory(ModelGeometry& geometry, uint64& vboSize, const char* name);
	bool CreateModelBlas(ModelGeometry& info, VkBuffer buffer, const char* name);
	bool BuildModelBlas(VkCommandBuffer cmds, ModelGeometry& info, uint64 first_vertex_offset, const Buffer& buffer);
	bool CreatePrimitiveBuffer();
	void DestroyPrimitiveBuffer();

	Renderer& _renderer;
	VkDescriptorPool desc_pool_vertex_buffer = nullptr;
	VkDescriptorSet _descSet = nullptr;
	VkDescriptorSetLayout _descSetLayout = nullptr;
	Buffer* null_buffer = nullptr;
	Buffer* _readbackBuffer = nullptr;
	Buffer* _stagingReadbackBuffers[maxFramesInFlight] = {};
	Buffer* _toneMapBuffer = nullptr;
	Buffer* _worldBuffer = nullptr;
	Buffer* _matricesBuffer = nullptr;
	Buffer* _stagingMatricesBuffers[maxFramesInFlight] = {};
	Buffer* _instancedPrimitiveBuffer = nullptr;
	Buffer* _instancedPositionsBuffer = nullptr;
	Buffer* _accelScratchBuffer = nullptr;
	uint32 _nbInstancedPrimitives = 1 << 20;
	ModelVBO _modelVertexData[s_maxNbModels];
	WorldData* _worldData = nullptr;
};

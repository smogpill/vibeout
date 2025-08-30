// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "VertexBuffer.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Buffer/Buffer.h"
#include "Vibeout/Render/Shared/Utils.h"
#include "Vibeout/Render/Shared/Buffers.h"
#include "Vibeout/Base/Utils.h"
#include "Vibeout/Game/Game.h"

VertexBuffer::VertexBuffer(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

VertexBuffer::~VertexBuffer()
{
	delete _worldData;
	VkDevice device = _renderer.GetDevice();
	if (desc_pool_vertex_buffer)
		vkDestroyDescriptorPool(device, desc_pool_vertex_buffer, nullptr);
	if (_descSetLayout)
		vkDestroyDescriptorSetLayout(device, _descSetLayout, nullptr);

	delete null_buffer;
	delete _readbackBuffer;
	for (Buffer* buffer : _stagingReadbackBuffers)
		delete buffer;
	delete _toneMapBuffer;
}

bool VertexBuffer::Init()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VkDescriptorSetLayoutBinding vbo_layout_bindings[] =
	{
		{
			.binding = PRIMITIVE_BUFFER_BINDING_IDX,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = VERTEX_BUFFER_FIRST_MODEL + s_maxNbModels,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = POSITION_BUFFER_BINDING_IDX,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = MATRIX_BUFFER_BINDING_IDX,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = READBACK_BUFFER_BINDING_IDX,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = TONE_MAPPING_BUFFER_BINDING_IDX,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		}
	};

	VkDescriptorSetLayoutCreateInfo layout_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = std::size(vbo_layout_bindings),
		.pBindings = vbo_layout_bindings,
	};

	VO_TRY_VK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &_descSetLayout));

	{
		Buffer::Setup setup;
		setup._size = sizeof(ReadbackBuffer);
		setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		_readbackBuffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
	}

	{
		Buffer::Setup setup;
		setup._size = sizeof(MatrixBuffer);
		setup._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		_matricesBuffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
		_renderer.SetObjectName(_matricesBuffer->GetBuffer(), "Matrices");
	}

	for (int frame = 0; frame < maxFramesInFlight; ++frame)
	{
		Buffer::Setup setup;
		setup._size = sizeof(MatrixBuffer);
		setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		bool result;
		_stagingMatricesBuffers[frame] = new Buffer(_renderer, setup, result);
		VO_TRY(result);
		_renderer.SetObjectName(_stagingMatricesBuffers[frame]->GetBuffer(), std::format("StagingMatrices{}", frame).c_str());
	}

	{
		Buffer::Setup setup;
		setup._size = sizeof(ToneMappingBuffer);
		setup._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		_toneMapBuffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
	}

	for (int frame = 0; frame < maxFramesInFlight; ++frame)
	{
		Buffer::Setup setup;
		setup._size = sizeof(ReadbackBuffer);
		setup._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		bool result;
		_stagingReadbackBuffers[frame] = new Buffer(_renderer, setup, result);
		VO_TRY(result);
		_renderer.SetObjectName(_stagingReadbackBuffers[frame]->GetBuffer(), std::format("StagingReadback{}", frame).c_str());
	}

	{
		Buffer::Setup setup;
		setup._size = 4;
		setup._usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		null_buffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
	}

	VkDescriptorPoolSize pool_size =
	{
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = std::size(vbo_layout_bindings) + s_maxNbModels + 128,
	};

	VkDescriptorPoolCreateInfo pool_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 2,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
	};

	VO_TRY_VK(vkCreateDescriptorPool(device, &pool_info, nullptr, &desc_pool_vertex_buffer));

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = desc_pool_vertex_buffer,
		.descriptorSetCount = 1,
		.pSetLayouts = &_descSetLayout,
	};

	VO_TRY_VK(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, &_descSet));

	VkDescriptorBufferInfo buf_info =
	{
		.buffer = null_buffer->GetBuffer(),
		.offset = 0,
		.range = null_buffer->GetSize(),
	};

	VkWriteDescriptorSet output_buf_write =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descSet,
		.dstArrayElement = VERTEX_BUFFER_WORLD,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &buf_info,
	};

	output_buf_write.dstBinding = MATRIX_BUFFER_BINDING_IDX;
	buf_info.buffer = _matricesBuffer->GetBuffer();
	buf_info.range = sizeof(MatrixBuffer);
	vkUpdateDescriptorSets(device, 1, &output_buf_write, 0, nullptr);

	output_buf_write.dstBinding = READBACK_BUFFER_BINDING_IDX;
	buf_info.buffer = _readbackBuffer->GetBuffer();
	buf_info.range = sizeof(ReadbackBuffer);
	vkUpdateDescriptorSets(device, 1, &output_buf_write, 0, nullptr);

	output_buf_write.dstBinding = TONE_MAPPING_BUFFER_BINDING_IDX;
	buf_info.buffer = _toneMapBuffer->GetBuffer();
	buf_info.range = sizeof(ToneMappingBuffer);
	vkUpdateDescriptorSets(device, 1, &output_buf_write, 0, nullptr);

	VO_TRY(CreatePrimitiveBuffer());

	for (ModelVBO& vbo : _modelVertexData)
		vbo = ModelVBO();

	/*
	for (int i = 0; i < s_maxNbModels; i++)
	{
		write_model_vbo_descriptor(i, null_buffer.buffer, null_buffer.size);
	}
	*/

	return true;
}

bool VertexBuffer::Readback(ReadbackBuffer& dst)
{
	Buffer* buffer = _stagingReadbackBuffers[_renderer._currentFrameInFlight];
	void* mapped = buffer->Map();
	VO_TRY(mapped);
	memcpy(&dst, mapped, sizeof(ReadbackBuffer));
	buffer->Unmap();
	return true;
}

bool VertexBuffer::InitPipelines()
{
	/*
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	Buffers* buffers = _renderer._buffers;
	VO_TRY(buffers);
	VkDescriptorSetLayout descSetLayoutUBO = buffers->GetDescriptorSetLayout();
	VO_TRY(descSetLayoutUBO);
	*/
	return true;
}

void VertexBuffer::ShutdownPipelines()
{
}

bool VertexBuffer::BuildWorldData()
{
	delete _worldData; _worldData = nullptr;
	Game* game = Game::_instance;
	if (!game)
		return true;

	std::unique_ptr<WorldData> worldData(new WorldData());

	//worldData->
	return true;
}

bool VertexBuffer::UploadWorld()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	vkDeviceWaitIdle(device);
	delete _worldBuffer; _worldBuffer = nullptr;

	const uint64 nbPrimitives = _worldData->_primitives.size();
	uint64 vboSize = nbPrimitives * sizeof(VboPrimitive);
	_worldData->_vertexDataOffset = vboSize;
	vboSize += nbPrimitives * sizeof(mat3);
	const uint64 stagingSize = vboSize;

	// Suballocations
	{
		VO_TRY(SuballocateModelBlasMemory(_worldData->_opaqueGeom, vboSize, "WorldOpaque"));
		VO_TRY(SuballocateModelBlasMemory(_worldData->_transparentGeom, vboSize, "WorldTransparent"));
		VO_TRY(SuballocateModelBlasMemory(_worldData->_maskedGeom, vboSize, "WorldMasked"));

		for (int i = 0; i < _worldData->_models.size(); i++)
		{
			ModelDesc& model = _worldData->_models[i];
			VO_TRY(SuballocateModelBlasMemory(model._geometry, vboSize, std::format("Model{}", i).c_str()));
		}
	}

	{
		Buffer::Setup setup;
		setup._size = vboSize;
		setup._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		_worldBuffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
	}
	
	_renderer.SetObjectName(_worldBuffer->GetBuffer(), "WorldBuffer");

	std::unique_ptr<Buffer> stagingBuffer;
	{
		Buffer::Setup setup;
		setup._size = stagingSize;
		setup._usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		bool result;
		stagingBuffer = std::make_unique<Buffer>(_renderer, setup, result);
		VO_TRY(result);
	}

	{
		VO_TRY(CreateModelBlas(_worldData->_opaqueGeom, _worldBuffer->GetBuffer(), "WorldOpaque"));
		VO_TRY(CreateModelBlas(_worldData->_transparentGeom, _worldBuffer->GetBuffer(), "WorldTransparent"));
		VO_TRY(CreateModelBlas(_worldData->_maskedGeom, _worldBuffer->GetBuffer(), "WorldMasked"));

		for (int i = 0; i < _worldData->_models.size(); i++)
		{
			ModelDesc& model = _worldData->_models[i];
			VO_TRY(CreateModelBlas(model._geometry, _worldBuffer->GetBuffer(), std::format("Model{}", i).c_str()));
		}
	}

	uint8* stagingData = (uint8*)stagingBuffer->Map();
	memcpy(stagingData, _worldData->_primitives.data(), nbPrimitives * sizeof(VboPrimitive));

	auto vectorCopy = [](const float* in, float* out)
		{
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
		};

	mat3* positions = (mat3*)(stagingData + _worldData->_vertexDataOffset);
	for (uint32 primIdx = 0; primIdx < nbPrimitives; ++primIdx)
	{
		vectorCopy(_worldData->_primitives[primIdx].pos0, positions[primIdx][0]);
		vectorCopy(_worldData->_primitives[primIdx].pos1, positions[primIdx][1]);
		vectorCopy(_worldData->_primitives[primIdx].pos2, positions[primIdx][2]);
	}

	stagingBuffer->Unmap();

	VkCommandBuffer cmds = _renderer.BeginCommandBuffer(_renderer._graphicsCommandBuffers);

	VkBufferCopy copyRegion = {};
	copyRegion.size = stagingBuffer->GetSize();

	vkCmdCopyBuffer(cmds, stagingBuffer->GetBuffer(), _worldBuffer->GetBuffer(), 1, &copyRegion);

	VkBufferMemoryBarrier barrier = BUFFER_BARRIER();
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	barrier.buffer = _worldBuffer->GetBuffer();
	barrier.offset = 0;
	barrier.size = VK_WHOLE_SIZE;

	QUEUE_BUFFER_BARRIER(cmds, barrier);

	{
		VO_TRY(BuildModelBlas(cmds, _worldData->_opaqueGeom, _worldData->_vertexDataOffset, *_worldBuffer));
		VO_TRY(BuildModelBlas(cmds, _worldData->_transparentGeom, _worldData->_vertexDataOffset, *_worldBuffer));
		VO_TRY(BuildModelBlas(cmds, _worldData->_maskedGeom, _worldData->_vertexDataOffset, *_worldBuffer));

		/*
		bsp_mesh->geom_opaque.instance_mask = AS_FLAG_OPAQUE;
		bsp_mesh->geom_opaque.instance_flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
		bsp_mesh->geom_opaque.sbt_offset = SBTO_OPAQUE;

		bsp_mesh->geom_transparent.instance_mask = AS_FLAG_TRANSPARENT;
		bsp_mesh->geom_transparent.instance_flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
		bsp_mesh->geom_transparent.sbt_offset = SBTO_OPAQUE;

		bsp_mesh->geom_masked.instance_mask = AS_FLAG_OPAQUE;
		bsp_mesh->geom_masked.instance_flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR | VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		bsp_mesh->geom_masked.sbt_offset = SBTO_MASKED;
		*/

		for (ModelDesc& model : _worldData->_models)
		{
			VO_TRY(BuildModelBlas(cmds, model._geometry, _worldData->_vertexDataOffset, *_worldBuffer));

			/*
			model.geometry.instance_mask = model->transparent ? bsp_mesh->geom_transparent.instance_mask : bsp_mesh->geom_opaque.instance_mask;
			model.geometry.instance_flags = model->masked ? bsp_mesh->geom_masked.instance_flags : model->transparent ? bsp_mesh->geom_transparent.instance_flags : bsp_mesh->geom_opaque.instance_flags;
			model.geometry.sbt_offset = model->masked ? bsp_mesh->geom_masked.sbt_offset : bsp_mesh->geom_opaque.sbt_offset;
			*/
		}
	}

	VO_TRY(_renderer.SubmitCommandBuffer(cmds, _renderer._graphicsQueue, 0, nullptr, nullptr, 0, nullptr, nullptr));

	vkDeviceWaitIdle(device);

	stagingBuffer.reset();

	VkDescriptorBufferInfo bufInfo =
	{
		.buffer = _worldBuffer->GetBuffer(),
		.offset = 0,
		.range = nbPrimitives * sizeof(VboPrimitive),
	};

	VkWriteDescriptorSet write = 
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descSet,
		.dstBinding = PRIMITIVE_BUFFER_BINDING_IDX,
		.dstArrayElement = VERTEX_BUFFER_WORLD,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &bufInfo,
	};

	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

	return true;
}

bool VertexBuffer::CreatePrimitiveBuffer()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	{
		Buffer::Setup setup;
		setup._size = sizeof(VboPrimitive) * _nbInstancedPrimitives;
		setup._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 
			| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		_instancedPrimitiveBuffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
		_renderer.SetObjectName(_instancedPrimitiveBuffer->GetBuffer(), "Instanced primitives");
	}

	{
		Buffer::Setup setup;
		setup._size = sizeof(mat3) * _nbInstancedPrimitives;
		setup._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 
			| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		setup._memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		bool result;
		_instancedPositionsBuffer = new Buffer(_renderer, setup, result);
		VO_TRY(result);
		_renderer.SetObjectName(_instancedPositionsBuffer->GetBuffer(), "Instanced positions");
	}

	VkDescriptorBufferInfo bufferInfo = {};

	VkWriteDescriptorSet output_buf_write = {};
	{
		output_buf_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		output_buf_write.dstSet = _descSet;
		output_buf_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		output_buf_write.descriptorCount = 1;
		output_buf_write.pBufferInfo = &bufferInfo;
	};

	output_buf_write.dstBinding = PRIMITIVE_BUFFER_BINDING_IDX;
	output_buf_write.dstArrayElement = VERTEX_BUFFER_INSTANCED;
	bufferInfo.buffer = _instancedPrimitiveBuffer->GetBuffer();
	bufferInfo.range = _instancedPrimitiveBuffer->GetSize();
	vkUpdateDescriptorSets(device, 1, &output_buf_write, 0, nullptr);

	output_buf_write.dstBinding = POSITION_BUFFER_BINDING_IDX;
	output_buf_write.dstArrayElement = 0;
	bufferInfo.buffer = _instancedPositionsBuffer->GetBuffer();
	bufferInfo.range = _instancedPositionsBuffer->GetSize();
	vkUpdateDescriptorSets(device, 1, &output_buf_write, 0, nullptr);

	return true;
}

void VertexBuffer::DestroyPrimitiveBuffer()
{
	delete _instancedPrimitiveBuffer; _instancedPrimitiveBuffer = nullptr;
	delete _instancedPositionsBuffer; _instancedPositionsBuffer = nullptr;
}

bool VertexBuffer::SuballocateModelBlasMemory(ModelGeometry& info, uint64& vboSize, const char* name)
{
	VkDevice device = _renderer.GetDevice();

	VkAccelerationStructureBuildSizesInfoKHR build_sizes =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
	};

	info.build_sizes = build_sizes;

	if (info.num_geometries == 0)
		return true;

	VkAccelerationStructureBuildGeometryInfoKHR blasBuildinfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.geometryCount = info.num_geometries,
		.pGeometries = info.geometries
	};

	_renderer._vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&blasBuildinfo, info.prim_counts, &info.build_sizes);

	if (info.build_sizes.buildScratchSize > _accelScratchBuffer->GetSize())
	{
		VO_ERROR("Model {} requires {} scratch buffer to build its BLAS, while only {} are available.", 
			name, info.build_sizes.buildScratchSize, _accelScratchBuffer->GetSize());
		info.num_geometries = 0;
	}
	else
	{
		vboSize = AlignUp<uint64>(vboSize, s_accelStructAlignment);

		info.blas_data_offset = vboSize;
		vboSize += info.build_sizes.accelerationStructureSize;
	}

	return true;
}

bool VertexBuffer::CreateModelBlas(ModelGeometry& info, VkBuffer buffer, const char* name)
{
	if (info.num_geometries == 0)
		return true;

	VkDevice device = _renderer.GetDevice();

	VkAccelerationStructureCreateInfoKHR blasCreateInfo = {};
	blasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	blasCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	blasCreateInfo.buffer = buffer;
	blasCreateInfo.offset = info.blas_data_offset;
	blasCreateInfo.size = info.build_sizes.accelerationStructureSize;

	VO_TRY_VK(_renderer._vkCreateAccelerationStructureKHR(device, &blasCreateInfo, nullptr, &info.accel));

	VkAccelerationStructureDeviceAddressInfoKHR as_device_address_info =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.accelerationStructure = info.accel
	};

	info.blas_device_address = _renderer._vkGetAccelerationStructureDeviceAddressKHR(device, &as_device_address_info);

	if (name)
		_renderer.SetObjectName(info.accel, name);

	return true;
}

bool VertexBuffer::BuildModelBlas(VkCommandBuffer cmds, ModelGeometry& info, uint64 first_vertex_offset, const Buffer& buffer)
{
	if (!info.accel)
		return true;

	VO_ASSERT(buffer.GetDeviceAddress());

	uint32 total_prims = 0;

	for (uint32 index = 0; index < info.num_geometries; index++)
	{
		VkAccelerationStructureGeometryKHR* geometry = info.geometries + index;

		geometry->geometry.triangles.vertexData.deviceAddress = buffer.GetDeviceAddress()
			+ info.prim_offsets[index] * sizeof(mat3) + first_vertex_offset;

		total_prims += info.prim_counts[index];
	}

	VkAccelerationStructureBuildGeometryInfoKHR blasBuildinfo = {};
	{
		blasBuildinfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		blasBuildinfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		blasBuildinfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		blasBuildinfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		blasBuildinfo.geometryCount = info.num_geometries;
		blasBuildinfo.pGeometries = info.geometries;
		blasBuildinfo.dstAccelerationStructure = info.accel;
		blasBuildinfo.scratchData.deviceAddress = _accelScratchBuffer->GetDeviceAddress();
	};

	const VkAccelerationStructureBuildRangeInfoKHR* pBlasBuildRange = info.build_ranges;

	_renderer._vkCmdBuildAccelerationStructuresKHR(cmds, 1, &blasBuildinfo, &pBlasBuildRange);

	VkMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR
					   | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
		.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR
	};

	VkPipelineStageFlags blas_dst_stage = /*qvk.use_ray_query ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : */VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
	vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		blas_dst_stage, 0, 1,
		&barrier, 0, 0, 0, 0);

	return true;
}

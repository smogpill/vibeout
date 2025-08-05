// Copyright(c) 2025 Jounayd Id Salah
#ifndef GLOBAL_STORAGE_H
#define GLOBAL_STORAGE_H
#include "ShaderInterface.h"

#define GLOBAL_TLAS_NODES_BINDING_IDX 0
#define GLOBAL_TLAS_LEAVES_BINDING_IDX 1

#ifndef __cplusplus
layout(set = GLOBAL_STORAGE_DESC_SET_IDX, binding = GLOBAL_TLAS_NODES_BINDING_IDX, std430) readonly buffer TLASNodes
{
	uint _data[];
} inTLASNodes;
layout(set = GLOBAL_STORAGE_DESC_SET_IDX, binding = GLOBAL_TLAS_LEAVES_BINDING_IDX, std430) readonly buffer TLASLeaves
{
	uint _data[];
} inTLASLeaves;
#endif

#endif

/*
Copyright (C) 2018 Christoph Schied
Copyright (C) 2019-2021, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _VERTEX_BUFFER_H_
#define _VERTEX_BUFFER_H_

#include "ShaderInterface.h"
#include "SharedConstants.h"

#define MAX_LIGHT_POLYS         4096
#define LIGHT_POLY_VEC4S        4
#define MATERIAL_UINTS          6

// should match the same constant declared in material.h
#define MAX_PBR_MATERIALS       4096

#define MAX_MATRICES        32768

#define ALIGN_SIZE_4(x, n)  ((x * n + 3) & (~3))

#define PRIMITIVE_BUFFER_BINDING_IDX 0
#define POSITION_BUFFER_BINDING_IDX 1
#define LIGHT_BUFFER_BINDING_IDX 2
#define LIGHT_COUNTS_HISTORY_BUFFER_BINDING_IDX 3
#define MATRIX_BUFFER_BINDING_IDX 4
#define READBACK_BUFFER_BINDING_IDX 5
#define TONE_MAPPING_BUFFER_BINDING_IDX 6
#define SUN_COLOR_BUFFER_BINDING_IDX 7
#define SUN_COLOR_UBO_BINDING_IDX 8
#define LIGHT_STATS_BUFFER_BINDING_IDX 9

#define VERTEX_BUFFER_WORLD 0
#define VERTEX_BUFFER_INSTANCED 1
#define VERTEX_BUFFER_FIRST_MODEL 2

#define SUN_COLOR_ACCUMULATOR_FIXED_POINT_SCALE 0x100000
#define SKY_COLOR_ACCUMULATOR_FIXED_POINT_SCALE 0x100

// A structure that is used in primitive buffers to store complete information about one triangle. 
// Its size is 8x float4 or 128 bytes to align with GPU cache lines.
// Path tracing accesses the primitive information in a very incoherent way, where every thread
// is likely to read a different primitive. Packing the info into one struct should reduce the
// total traffic from video memory by reading entire cache lines instead of sparse values from
// different buffers.
struct VboPrimitive
{
	vec3 pos0;
	uint material_id;

	vec3 pos1;
	int cluster;

	vec3 pos2;
	uint shell;

	uvec3 normals;
	uint instance;

	uvec3 tangents;
	/* packed half2x16, with emissive in x component/low 16 bits and
	 * alpha in y component/high 16 bits */
	uint emissive_and_alpha;

	vec2 uv0;
	vec2 uv1;
	vec2 uv2;
	uvec2 custom0;  // The custom fields store motion for instanced meshes in the animated buffer,
	uvec2 custom1;  // or blend indices and weights for skinned meshes before they're animated.
	uvec2 custom2;
};

struct MatrixBuffer
{
	vec4 _matrices[MAX_MATRICES * 3];
};

struct ToneMappingBuffer
{
	int accumulator[HISTOGRAM_BINS];
	float curve[HISTOGRAM_BINS];
	float normalized[HISTOGRAM_BINS];
	float adapted_luminance;
	float tonecurve;
};

struct ReadbackBuffer
{
	uint material;
	vec3 hdr_color;
	float adapted_luminance;
};

#ifndef __cplusplus

#ifdef VERTEX_READONLY
#define VERTEX_READONLY_FLAG readonly
#else
#define VERTEX_READONLY_FLAG
#endif

struct MaterialInfo
{
	uint base_texture;
	uint normals_texture;
	uint emissive_texture;
	uint mask_texture;
	float bump_scale;
	float roughness_override;
	float metalness_factor;
	float emissive_factor;
	float specular_factor;
	float base_factor;
	float light_style_scale;
	uint num_frames;
	uint next_frame;
};

// The buffers with primitive data, currently two of them: world and instanced.
// They are stored in an array to allow branchless access with nonuniformEXT.
layout(set = VERTEX_BUFFER_DESC_SET_IDX, binding = PRIMITIVE_BUFFER_BINDING_IDX) VERTEX_READONLY_FLAG buffer PRIMITIVE_BUFFER
{
	VboPrimitive primitives[];
} primitive_buffers[];

// The buffer with just the position data for animated models.
layout(set = VERTEX_BUFFER_DESC_SET_IDX, binding = POSITION_BUFFER_BINDING_IDX) VERTEX_READONLY_FLAG buffer POSITION_BUFFER
{
	float positions[];
} instanced_position_buffer;

layout(set = VERTEX_BUFFER_DESC_SET_IDX, binding = MATRIX_BUFFER_BINDING_IDX) readonly buffer MATRIX_BUFFER
{
	MatrixBuffer _matrixBuffer;
};

layout(set = VERTEX_BUFFER_DESC_SET_IDX, binding = READBACK_BUFFER_BINDING_IDX) buffer READBACK_BUFFER
{
	ReadbackBuffer readback;
};

layout(set = VERTEX_BUFFER_DESC_SET_IDX, binding = TONE_MAPPING_BUFFER_BINDING_IDX) buffer TONE_MAPPING_BUFFER
{
	ToneMappingBuffer tonemap_buffer;
};

struct Triangle
{
	mat3x3 positions;
	mat3x3 positions_prev;
	mat3x3 normals;
	mat3x2 tex_coords;
	mat3x3 tangents;
	uint   material_id;
	uint   shell;
	int    cluster;
	uint   instance_index;
	uint   instance_prim;
	float  emissive_factor;
	float  alpha;
};

MaterialInfo get_material_info(uint material_id)
{
	/*
	uint material_index = material_id & MATERIAL_INDEX_MASK;

	uint data[MATERIAL_UINTS];
	data[0] = light_buffer.material_table[material_index * MATERIAL_UINTS + 0];
	data[1] = light_buffer.material_table[material_index * MATERIAL_UINTS + 1];
	data[2] = light_buffer.material_table[material_index * MATERIAL_UINTS + 2];
	data[3] = light_buffer.material_table[material_index * MATERIAL_UINTS + 3];
	data[4] = light_buffer.material_table[material_index * MATERIAL_UINTS + 4];
	data[5] = light_buffer.material_table[material_index * MATERIAL_UINTS + 5];

	MaterialInfo minfo;
	minfo.base_texture = data[0] & 0xffff;
	minfo.normals_texture = data[0] >> 16;
	minfo.emissive_texture = data[1] & 0xffff;
	minfo.mask_texture = data[1] >> 16;
	minfo.bump_scale = unpackHalf2x16(data[2]).x;
	minfo.roughness_override = unpackHalf2x16(data[2]).y;
	minfo.metalness_factor = unpackHalf2x16(data[3]).x;
	minfo.emissive_factor = unpackHalf2x16(data[3]).y;
	minfo.specular_factor = unpackHalf2x16(data[5]).x;
	minfo.base_factor = unpackHalf2x16(data[5]).y;
	minfo.num_frames = data[4] & 0xffff;
	minfo.next_frame = (data[4] >> 16) & (MAX_PBR_MATERIALS - 1);

	// Apply the light style for non-camera materials.
	// Camera materials use the same bits to store the camera ID.
	if ((material_id & MATERIAL_KIND_MASK) != MATERIAL_KIND_CAMERA)
	{
		uint light_style = (material_id & MATERIAL_LIGHT_STYLE_MASK) >> MATERIAL_LIGHT_STYLE_SHIFT;
		if (light_style != 0)
		{
			minfo.emissive_factor *= light_buffer.light_styles[light_style];
		}
	}

	return minfo;

	*/

	MaterialInfo info;
	info.base_texture = 0;
	info.normals_texture = 0;
	info.emissive_texture = 0;
	info.mask_texture = 0;
	info.bump_scale = 0.0f;
	info.roughness_override = 1.0f;
	info.metalness_factor = 0.0f;
	info.emissive_factor = 0.0f;
	info.specular_factor = 0.0f;
	info.base_factor = 1.0f;
	info.light_style_scale;
	info.num_frames = 1;
	info.next_frame = 1;
	return info;
}

uint animate_material(uint material, int frame)
{
	// Apply frame-based material animation: go through the linked list of materials.
	if (frame > 0)
	{
		uint new_material = material;
		MaterialInfo minfo = get_material_info(new_material);
		frame = frame % int(minfo.num_frames);

		while (frame-- > 0) {
			new_material = minfo.next_frame;
			minfo = get_material_info(new_material);
		}

		material = new_material | (material & ~MATERIAL_INDEX_MASK); // preserve flags
	}
	return material;
}

Triangle load_triangle(uint buffer_idx, uint prim_id)
{
	VboPrimitive prim = primitive_buffers[nonuniformEXT(buffer_idx)].primitives[prim_id];

	Triangle t;
	t.positions[0] = prim.pos0;
	t.positions[1] = prim.pos1;
	t.positions[2] = prim.pos2;

	t.positions_prev[0] = t.positions[0] + unpackHalf4x16(prim.custom0).xyz;
	t.positions_prev[1] = t.positions[1] + unpackHalf4x16(prim.custom1).xyz;
	t.positions_prev[2] = t.positions[2] + unpackHalf4x16(prim.custom2).xyz;

	t.normals[0] = decode_normal(prim.normals.x);
	t.normals[1] = decode_normal(prim.normals.y);
	t.normals[2] = decode_normal(prim.normals.z);

	t.tangents[0] = decode_normal(prim.tangents.x);
	t.tangents[1] = decode_normal(prim.tangents.y);
	t.tangents[2] = decode_normal(prim.tangents.z);

	t.tex_coords[0] = prim.uv0;
	t.tex_coords[1] = prim.uv1;
	t.tex_coords[2] = prim.uv2;

	t.material_id = prim.material_id;
	t.shell = prim.shell;
	t.cluster = prim.cluster;
	t.instance_index = prim.instance;
	t.instance_prim = 0;

	vec2 emissive_and_alpha = unpackHalf2x16(prim.emissive_and_alpha);
	t.emissive_factor = emissive_and_alpha.x;
	t.alpha = emissive_and_alpha.y;

	return t;
}

Triangle load_and_transform_triangle(int instance_idx, uint buffer_idx, uint prim_id)
{
	Triangle t = load_triangle(buffer_idx, prim_id);

	if (instance_idx >= 0)
	{
		// Instance of a static mesh: transform the vertices.

		ModelInstance mi = _instanceBuffer.model_instances[instance_idx];

		t.positions[0] = vec3(mi.transform * vec4(t.positions[0], 1.0));
		t.positions[1] = vec3(mi.transform * vec4(t.positions[1], 1.0));
		t.positions[2] = vec3(mi.transform * vec4(t.positions[2], 1.0));

		t.positions_prev[0] = vec3(mi.transform_prev * vec4(t.positions_prev[0], 1.0));
		t.positions_prev[1] = vec3(mi.transform_prev * vec4(t.positions_prev[1], 1.0));
		t.positions_prev[2] = vec3(mi.transform_prev * vec4(t.positions_prev[2], 1.0));

		t.normals[0] = normalize(vec3(mi.transform * vec4(t.normals[0], 0.0)));
		t.normals[1] = normalize(vec3(mi.transform * vec4(t.normals[1], 0.0)));
		t.normals[2] = normalize(vec3(mi.transform * vec4(t.normals[2], 0.0)));

		t.tangents[0] = normalize(vec3(mi.transform * vec4(t.tangents[0], 0.0)));
		t.tangents[1] = normalize(vec3(mi.transform * vec4(t.tangents[1], 0.0)));
		t.tangents[2] = normalize(vec3(mi.transform * vec4(t.tangents[2], 0.0)));

		if (mi.material != 0) {
			t.material_id = mi.material;
			t.shell = mi.shell;
		}
		int frame = int(mi.alpha_and_frame >> 16);
		t.material_id = animate_material(t.material_id, frame);
		t.cluster = mi.cluster;
		t.emissive_factor = 1.0;
		t.alpha *= unpackHalf2x16(mi.alpha_and_frame).x;

		// Store the index of that instance and the prim offset relative to the instance.
		t.instance_index = uint(instance_idx);
		t.instance_prim = prim_id - mi.render_prim_offset;
	}
	else if (buffer_idx == VERTEX_BUFFER_INSTANCED)
	{
		// Instance of an animated or skinned mesh, coming from the primbuf.
		// In this case, `instance_idx` is -1 because it's not a static mesh, 
		// so load the original animated instance to find out its prim offset.

		ModelInstance mi = _instanceBuffer.model_instances[t.instance_index];
		t.instance_prim = prim_id - mi.render_prim_offset;
	}
	else if (buffer_idx == VERTEX_BUFFER_WORLD)
	{
		// Static BSP primitive.

		t.instance_index = ~0u;
		t.instance_prim = prim_id;
	}

	return t;
}

#ifndef VERTEX_READONLY
void store_triangle(Triangle t, uint buffer_idx, uint prim_id)
{
	VboPrimitive prim;

	prim.pos0 = t.positions[0];
	prim.pos1 = t.positions[1];
	prim.pos2 = t.positions[2];

	prim.custom0 = packHalf4x16(vec4(t.positions_prev[0] - t.positions[0], 0));
	prim.custom1 = packHalf4x16(vec4(t.positions_prev[1] - t.positions[1], 0));
	prim.custom2 = packHalf4x16(vec4(t.positions_prev[2] - t.positions[2], 0));

	prim.normals.x = encode_normal(t.normals[0]);
	prim.normals.y = encode_normal(t.normals[1]);
	prim.normals.z = encode_normal(t.normals[2]);

	prim.tangents.x = encode_normal(t.tangents[0]);
	prim.tangents.y = encode_normal(t.tangents[1]);
	prim.tangents.z = encode_normal(t.tangents[2]);

	prim.uv0 = t.tex_coords[0];
	prim.uv1 = t.tex_coords[1];
	prim.uv2 = t.tex_coords[2];

	prim.material_id = t.material_id;
	prim.shell = t.shell;
	prim.cluster = t.cluster;
	prim.instance = t.instance_index;
	prim.emissive_and_alpha = packHalf2x16(vec2(t.emissive_factor, t.alpha));

	primitive_buffers[nonuniformEXT(buffer_idx)].primitives[prim_id] = prim;

	if (buffer_idx == VERTEX_BUFFER_INSTANCED)
	{
		for (int vert = 0; vert < 3; vert++)
		{
			for (int axis = 0; axis < 3; axis++)
			{
				instanced_position_buffer.positions[prim_id * 9 + vert * 3 + axis]
					= t.positions[vert][axis];
			}
		}
	}
}
#endif

mat3x4 get_iqm_matrix(uint index)
{
	mat3x4 result;
	result[0] = _matrixBuffer._matrices[index * 3 + 0];
	result[1] = _matrixBuffer._matrices[index * 3 + 1];
	result[2] = _matrixBuffer._matrices[index * 3 + 2];
	return result;
}

#endif
#endif
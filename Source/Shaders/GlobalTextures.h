// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#ifndef GLOBAL_TEXTURES_H
#define GLOBAL_TEXTURES_H
#include "ShaderInterface.h"
#include "SharedConstants.h"

#define GLOBAL_TEXTURES_DESC_SET_IDX 1
#define GLOBAL_TEXTURES_TEX_ARR_BINDING_IDX 0

#define IMG_WIDTH  (_renderer._extentScreenImages.width)
#define IMG_HEIGHT (_renderer._extentScreenImages.height)
#define IMG_WIDTH_UNSCALED  (_renderer._extentUnscaled.width)
#define IMG_HEIGHT_UNSCALED (_renderer._extentUnscaled.height)

#define IMG_WIDTH_GRAD  ((_renderer._extentScreenImages.width + GRAD_DWN - 1) / GRAD_DWN)
#define IMG_HEIGHT_GRAD ((_renderer._extentScreenImages.height + GRAD_DWN - 1) / GRAD_DWN)

#define IMG_WIDTH_TAA  (_renderer._extentTAAImages.width)
#define IMG_HEIGHT_TAA  (_renderer._extentTAAImages.height)

#define LIST_IMAGES \
	IMG_DO(PT_MOTION,                  0, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_TRANSPARENT,             1, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_HF,        2, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_ATROUS_PING_LF_SH,    3, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_ATROUS_PONG_LF_SH,    4, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_ATROUS_PING_LF_COCG,  5, R16G16_SFLOAT,       rg16f,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_ATROUS_PONG_LF_COCG,  6, R16G16_SFLOAT,       rg16f,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_ATROUS_PING_HF,       7, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_ATROUS_PONG_HF,       8, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_ATROUS_PING_SPEC,     9, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_ATROUS_PONG_SPEC,    10, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_ATROUS_PING_MOMENTS, 11, R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_ATROUS_PONG_MOMENTS, 12, R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_COLOR,               13, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_GRAD_LF_PING,        14, R16G16_SFLOAT,       rg16f,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_GRAD_LF_PONG,        15, R16G16_SFLOAT,       rg16f,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_GRAD_HF_SPEC_PING,   16, R16G16_SFLOAT,       rg16f,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_GRAD_HF_SPEC_PONG,   17, R16G16_SFLOAT,       rg16f,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(PT_SHADING_POSITION,       18, R32G32B32A32_SFLOAT, rgba32f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(FLAT_COLOR,                19, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(FLAT_MOTION,               20, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_GODRAYS_THROUGHPUT_DIST,21, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(BLOOM_HBLUR,               22, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_TAA / 4,   IMG_HEIGHT_TAA / 4 ) \
	IMG_DO(BLOOM_VBLUR,               23, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_TAA / 4,   IMG_HEIGHT_TAA / 4 ) \
	IMG_DO(TAA_OUTPUT,                24, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_TAA,       IMG_HEIGHT_TAA ) \
	IMG_DO(PT_VIEW_DIRECTION,         25, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VIEW_DIRECTION2,        26, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_THROUGHPUT,             27, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_BOUNCE_THROUGHPUT,      28, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(HQ_COLOR_INTERLEAVED,      29, R32G32B32A32_SFLOAT, rgba32f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_COLOR_LF_SH,            30, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_COLOR_LF_COCG,          31, R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_COLOR_HF,               32, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_COLOR_SPEC,             33, R32_UINT,            r32ui,   IMG_WIDTH,		IMG_HEIGHT     ) \
	IMG_DO(PT_GEO_NORMAL2,            34, R32_UINT,            r32ui,   IMG_WIDTH,		IMG_HEIGHT     ) \
	IMG_DO(FSR_EASU_OUTPUT,           35, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(FSR_RCAS_OUTPUT,           36, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(BLOOM,				      37, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_TAA / 2, IMG_HEIGHT_TAA / 2 ) \

#define NB_IMAGES_BASE 38

#define LIST_IMAGES_A_B \
	IMG_DO(PT_VISBUF_PRIM_A,          NB_IMAGES_BASE + 0,  R32G32_UINT,         rg32ui,  IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VISBUF_PRIM_B,          NB_IMAGES_BASE + 1,  R32G32_UINT,         rg32ui,  IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VISBUF_BARY_A,          NB_IMAGES_BASE + 2,  R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VISBUF_BARY_B,          NB_IMAGES_BASE + 3,  R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_CLUSTER_A,              NB_IMAGES_BASE + 4,  R16_UINT,            r16ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_CLUSTER_B,              NB_IMAGES_BASE + 5,  R16_UINT,            r16ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_BASE_COLOR_A,           NB_IMAGES_BASE + 6,  R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_BASE_COLOR_B,           NB_IMAGES_BASE + 7,  R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_METALLIC_A,             NB_IMAGES_BASE + 8,  R8G8_UNORM,          rg8,     IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_METALLIC_B,             NB_IMAGES_BASE + 9,  R8G8_UNORM,          rg8,     IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VIEW_DEPTH_A,           NB_IMAGES_BASE + 10, R16_SFLOAT,          r32f,    IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VIEW_DEPTH_B,           NB_IMAGES_BASE + 11, R16_SFLOAT,          r32f,    IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_NORMAL_A,               NB_IMAGES_BASE + 12, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_NORMAL_B,               NB_IMAGES_BASE + 13, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_GEO_NORMAL_A,           NB_IMAGES_BASE + 14, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_GEO_NORMAL_B,           NB_IMAGES_BASE + 15, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_FILTERED_SPEC_A,     NB_IMAGES_BASE + 16, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_FILTERED_SPEC_B,     NB_IMAGES_BASE + 17, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_MOMENTS_HF_A,   NB_IMAGES_BASE + 18, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_MOMENTS_HF_B,   NB_IMAGES_BASE + 19, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_TAA_A,               NB_IMAGES_BASE + 20, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_TAA,  IMG_HEIGHT_TAA ) \
	IMG_DO(ASVGF_TAA_B,               NB_IMAGES_BASE + 21, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_TAA,  IMG_HEIGHT_TAA ) \
	IMG_DO(ASVGF_RNG_SEED_A,          NB_IMAGES_BASE + 22, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_RNG_SEED_B,          NB_IMAGES_BASE + 23, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_LF_SH_A,  NB_IMAGES_BASE + 24, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_LF_SH_B,  NB_IMAGES_BASE + 25, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_LF_COCG_A,NB_IMAGES_BASE + 26, R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_LF_COCG_B,NB_IMAGES_BASE + 27, R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_GRAD_SMPL_POS_A,     NB_IMAGES_BASE + 28, R32_UINT,            r32ui,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_GRAD_SMPL_POS_B,     NB_IMAGES_BASE + 29, R32_UINT,            r32ui,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \

#define LIST_IMAGES_B_A \
	IMG_DO(PT_VISBUF_PRIM_B,          NB_IMAGES_BASE + 0,  R32G32_UINT,         rg32ui,  IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VISBUF_PRIM_A,          NB_IMAGES_BASE + 1,  R32G32_UINT,         rg32ui,  IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VISBUF_BARY_B,          NB_IMAGES_BASE + 2,  R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VISBUF_BARY_A,          NB_IMAGES_BASE + 3,  R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_CLUSTER_B,              NB_IMAGES_BASE + 4,  R16_UINT,            r16ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_CLUSTER_A,              NB_IMAGES_BASE + 5,  R16_UINT,            r16ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_BASE_COLOR_B,           NB_IMAGES_BASE + 6,  R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_BASE_COLOR_A,           NB_IMAGES_BASE + 7,  R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_METALLIC_B,             NB_IMAGES_BASE + 8,  R8G8_UNORM,          rg8,     IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_METALLIC_A,             NB_IMAGES_BASE + 9,  R8G8_UNORM,          rg8,     IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VIEW_DEPTH_B,           NB_IMAGES_BASE + 10, R16_SFLOAT,          r32f,    IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_VIEW_DEPTH_A,           NB_IMAGES_BASE + 11, R16_SFLOAT,          r32f,    IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_NORMAL_B,               NB_IMAGES_BASE + 12, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_NORMAL_A,               NB_IMAGES_BASE + 13, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_GEO_NORMAL_B,           NB_IMAGES_BASE + 14, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(PT_GEO_NORMAL_A,           NB_IMAGES_BASE + 15, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_FILTERED_SPEC_B,     NB_IMAGES_BASE + 16, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_FILTERED_SPEC_A,     NB_IMAGES_BASE + 17, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_MOMENTS_HF_B,   NB_IMAGES_BASE + 18, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_MOMENTS_HF_A,   NB_IMAGES_BASE + 19, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_TAA_B,               NB_IMAGES_BASE + 20, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_TAA,  IMG_HEIGHT_TAA ) \
	IMG_DO(ASVGF_TAA_A,               NB_IMAGES_BASE + 21, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH_TAA,  IMG_HEIGHT_TAA ) \
	IMG_DO(ASVGF_RNG_SEED_B,          NB_IMAGES_BASE + 22, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_RNG_SEED_A,          NB_IMAGES_BASE + 23, R32_UINT,            r32ui,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_LF_SH_B,  NB_IMAGES_BASE + 24, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_LF_SH_A,  NB_IMAGES_BASE + 25, R16G16B16A16_SFLOAT, rgba16f, IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_LF_COCG_B,NB_IMAGES_BASE + 26, R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_HIST_COLOR_LF_COCG_A,NB_IMAGES_BASE + 27, R16G16_SFLOAT,       rg16f,   IMG_WIDTH,      IMG_HEIGHT     ) \
	IMG_DO(ASVGF_GRAD_SMPL_POS_B,     NB_IMAGES_BASE + 28, R32_UINT,            r32ui,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \
	IMG_DO(ASVGF_GRAD_SMPL_POS_A,     NB_IMAGES_BASE + 29, R32_UINT,            r32ui,   IMG_WIDTH_GRAD, IMG_HEIGHT_GRAD) \

#define NB_IMAGES (NB_IMAGES_BASE + 30)

#define BINDING_OFFSET_IMAGES					(1 + GLOBAL_TEXTURES_TEX_ARR_BINDING_IDX)
#define BINDING_OFFSET_TEXTURES					(BINDING_OFFSET_IMAGES + NB_IMAGES)
#define BINDING_OFFSET_BLUE_NOISE				(BINDING_OFFSET_TEXTURES + NB_IMAGES)
#define BINDING_OFFSET_HEIGHTMAP				(BINDING_OFFSET_BLUE_NOISE + NB_IMAGES)
//#define BINDING_OFFSET_IMAGE_BLOOM_DOWNSAMPLE	(BINDING_OFFSET_BLUE_NOISE + 1)
//#define BINDING_OFFSET_TEX_BLOOM_DOWNSAMPLE		(BINDING_OFFSET_IMAGE_BLOOM_DOWNSAMPLE + 1)

#ifdef __cplusplus

//#if MAX_RIMAGES != NB_GLOBAL_TEXTURES
//#error need to fix the constant here as well
//#endif

enum class ImageID
{
#define IMG_DO(_name, ...) _name,
	LIST_IMAGES
	LIST_IMAGES_A_B
#undef IMG_DO
	END
};

static_assert(NB_IMAGES == (int)ImageID::END);

#else
								
layout(
	set = GLOBAL_TEXTURES_DESC_SET_IDX, 
	binding = GLOBAL_TEXTURES_TEX_ARR_BINDING_IDX
) uniform sampler2D global_texture_descriptors[];

#define SAMPLER_r16ui   usampler2D
#define SAMPLER_r32ui   usampler2D
#define SAMPLER_rg32ui  usampler2D
#define SAMPLER_r32i    isampler2D
#define SAMPLER_r32f    sampler2D
#define SAMPLER_rg32f   sampler2D
#define SAMPLER_rg16f   sampler2D
#define SAMPLER_rgba32f sampler2D
#define SAMPLER_rgba16f sampler2D
#define SAMPLER_rgba8   sampler2D
#define SAMPLER_r8      sampler2D
#define SAMPLER_rg8     sampler2D

#define IMAGE_r16ui   uimage2D
#define IMAGE_r32ui   uimage2D
#define IMAGE_rg32ui  uimage2D
#define IMAGE_r32i    iimage2D
#define IMAGE_r32f    image2D
#define IMAGE_rg32f   image2D
#define IMAGE_rg16f   image2D
#define IMAGE_rgba32f image2D
#define IMAGE_rgba16f image2D
#define IMAGE_rgba8   image2D
#define IMAGE_r8      image2D
#define IMAGE_rg8     image2D

// Framebuffer images
#define IMG_DO(_name, _binding, _vkformat, _glslformat, _w, _h) \
	layout(set = GLOBAL_TEXTURES_DESC_SET_IDX, binding = BINDING_OFFSET_IMAGES + _binding, _glslformat) \
	uniform IMAGE_##_glslformat IMG_##_name;
LIST_IMAGES
LIST_IMAGES_A_B
#undef IMG_DO

// Framebuffer textures
#define IMG_DO(_name, _binding, _vkformat, _glslformat, _w, _h) \
	layout(set = GLOBAL_TEXTURES_DESC_SET_IDX, binding = BINDING_OFFSET_TEXTURES + _binding) \
	uniform SAMPLER_##_glslformat TEX_##_name;
LIST_IMAGES
LIST_IMAGES_A_B
#undef IMG_DO

layout(
	set = GLOBAL_TEXTURES_DESC_SET_IDX,
	binding = BINDING_OFFSET_BLUE_NOISE
) uniform sampler2DArray TEX_BLUE_NOISE;

layout(
	set = GLOBAL_TEXTURES_DESC_SET_IDX,
	binding = BINDING_OFFSET_HEIGHTMAP
) uniform sampler2D TEX_HEIGHTMAP;

/*
layout(
	set = GLOBAL_TEXTURES_DESC_SET_IDX,
	binding = BINDING_OFFSET_IMAGE_BLOOM_DOWNSAMPLE,
	rgba16f
) uniform image2D IMAGE_BLOOM_DOWNSAMPLE;

layout(
	set = GLOBAL_TEXTURES_DESC_SET_IDX,
	binding = BINDING_OFFSET_TEX_BLOOM_DOWNSAMPLE
) uniform sampler2DArray TEX_BLOOM_DOWNSAMPLE;
*/

vec4 global_texture(uint idx, vec2 tex_coord)
{
	if (idx >= NB_GLOBAL_TEXTURES)
		return vec4(1, 0, 1, 0);
	return texture(global_texture_descriptors[nonuniformEXT(idx)], tex_coord);
}

vec4 global_textureLod(uint idx, vec2 tex_coord, float lod)
{
	if (idx >= NB_GLOBAL_TEXTURES)
		return vec4(1, 1, 0, 0);
	return textureLod(global_texture_descriptors[nonuniformEXT(idx)], tex_coord, lod);
}

vec4 global_textureGrad(uint idx, vec2 tex_coord, vec2 d_x, vec2 d_y)
{
	if (idx >= NB_GLOBAL_TEXTURES)
		return vec4(1, 1, 0, 0);
	return textureGrad(global_texture_descriptors[nonuniformEXT(idx)], tex_coord, d_x, d_y);
}

ivec2 global_textureSize(uint idx, int level)
{
	if (idx >= NB_GLOBAL_TEXTURES)
		return ivec2(0);
	return textureSize(global_texture_descriptors[nonuniformEXT(idx)], level);
}

#endif

#endif

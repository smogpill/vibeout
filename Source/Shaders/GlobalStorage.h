// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#ifndef GLOBAL_STORAGE_H
#define GLOBAL_STORAGE_H
#include "ShaderInterface.h"

#define GLOBAL_UBO_BINDING_IDX 0
#define GLOBAL_TLAS_NODES_BINDING_IDX 1
#define GLOBAL_TLAS_LEAVES_BINDING_IDX 2

#ifdef __cplusplus
struct alignas(16) GlobalUniformBuffer
#else
struct GlobalUniformBuffer
#endif
{
	SHARED_STRUCT_ATTRIB(mat4, _view, {});
	SHARED_STRUCT_ATTRIB(mat4, _proj, {});
	SHARED_STRUCT_ATTRIB(mat4, _invView, {});
	SHARED_STRUCT_ATTRIB(mat4, _invProj, {});
	SHARED_STRUCT_ATTRIB(mat4, _prevView, {});
	SHARED_STRUCT_ATTRIB(mat4, _prevProj, {});
	SHARED_STRUCT_ATTRIB(mat4, _prevInvProj, {});
	SHARED_STRUCT_ATTRIB(vec4, fs_blend_color, {});
	SHARED_STRUCT_ATTRIB(vec4, fs_colorize, {});

	SHARED_STRUCT_ATTRIB(vec3, _camPos, {});
	SHARED_STRUCT_ATTRIB(int, pt_projection, 0);

	SHARED_STRUCT_ATTRIB(vec2, _projection_fov_scale, {});
	SHARED_STRUCT_ATTRIB(vec2, _projection_fov_scale_prev, {});
	SHARED_STRUCT_ATTRIB(vec2, sub_pixel_jitter, {});
	SHARED_STRUCT_ATTRIB(float, _verticalFOV, 0.0f);
	SHARED_STRUCT_ATTRIB(float, temporal_blend_factor, 0.0f);

	SHARED_STRUCT_ATTRIB(ivec3, _worldMaskOrigin, {});
	SHARED_STRUCT_ATTRIB(int, _currentFrameIdx, 0);

	SHARED_STRUCT_ATTRIB(int, _width, 0);
	SHARED_STRUCT_ATTRIB(int, _height, 0);
	SHARED_STRUCT_ATTRIB(float, _invWidth, 0.0f);
	SHARED_STRUCT_ATTRIB(float, _invHeight, 0.0f);
	SHARED_STRUCT_ATTRIB(int, _prevWidth, 0);
	SHARED_STRUCT_ATTRIB(int, _prevHeight, 0);
	SHARED_STRUCT_ATTRIB(int, _unscaledWidth, 0);
	SHARED_STRUCT_ATTRIB(int, _unscaledHeight, 0);
	SHARED_STRUCT_ATTRIB(int, _screenImageWidth, 0);
	SHARED_STRUCT_ATTRIB(int, _screenImageHeight, 0);
	SHARED_STRUCT_ATTRIB(int, _taaImageWidth, 0);
	SHARED_STRUCT_ATTRIB(int, _taaImageHeight, 0);
	SHARED_STRUCT_ATTRIB(int, _taa_output_width, 0);
	SHARED_STRUCT_ATTRIB(int, _taa_output_height, 0);
	SHARED_STRUCT_ATTRIB(int, _prev_taa_output_width, 0);
	SHARED_STRUCT_ATTRIB(int, _prev_taa_output_height, 0);
	SHARED_STRUCT_ATTRIB(int, _drawVoxelLevel, 15);
	SHARED_STRUCT_ATTRIB(int, _maxIndirectLightingVoxelLevel, 10);
	SHARED_STRUCT_ATTRIB(int, _ptSwapCheckerboard, 0);
	SHARED_STRUCT_ATTRIB(float, bloom_intensity, 0.002f);
	SHARED_STRUCT_ATTRIB(float, _bloom2_intensity, 0.1f); // Froyok uses 0.3f
	SHARED_STRUCT_ATTRIB(float, _bloom2_radius, 0.7f); // Froyok uses 0.85f for something closer to a gaussian
	SHARED_STRUCT_ATTRIB(float, _fakeEmissive, 10.0f);
	SHARED_STRUCT_ATTRIB(float, tonemap_hdr_clamp_strength, 0.0f);
	SHARED_STRUCT_ATTRIB(float, cylindrical_hfov, 0.0f);
	SHARED_STRUCT_ATTRIB(float, cylindrical_hfov_prev, 0.0f);
	SHARED_STRUCT_ATTRIB(float, flt_antilag_hf, 1); /* A-SVGF anti-lag filter strength, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, flt_antilag_lf, 0.2f);
	SHARED_STRUCT_ATTRIB(float, flt_antilag_spec, 2);
	SHARED_STRUCT_ATTRIB(float, flt_antilag_spec_motion, 0.004f); /* scaler for motion vector scaled specular anti-blur adjustment */
	SHARED_STRUCT_ATTRIB(float, flt_atrous_depth, 0.5f); /* wavelet fitler sensitivity to depth, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, flt_atrous_deflicker_lf, 2); /* max brightness difference between adjacent pixels in the LF channel, (0..inf) */
	SHARED_STRUCT_ATTRIB(float, flt_atrous_hf, 4); /* number of a-trous wavelet filter iterations on the LF channel, [0..4] */
	SHARED_STRUCT_ATTRIB(float, flt_atrous_lf, 4);
	SHARED_STRUCT_ATTRIB(float, flt_atrous_spec, 3);
	SHARED_STRUCT_ATTRIB(float, flt_atrous_lum_hf, 16); /* wavelet filter sensitivity to luminance, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, flt_atrous_normal_hf, 64); /* wavelet filter sensitivity to normals, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, flt_atrous_normal_lf, 8);
	SHARED_STRUCT_ATTRIB(float, flt_atrous_normal_spec, 1);
	SHARED_STRUCT_ATTRIB(float, flt_enable, 1); /* switch for the entire SVGF reconstruction, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, flt_fixed_albedo, 0); /* if nonzero, replaces surface albedo with that value after filtering */
	SHARED_STRUCT_ATTRIB(float, flt_grad_weapon, 0.25f); /* gradient scale for the first person weapon, [0..1] */
	SHARED_STRUCT_ATTRIB(float, flt_min_alpha_color_hf, 0.02f); /* minimum weight for the new frame data, color channel, (0..1] */
	SHARED_STRUCT_ATTRIB(float, flt_min_alpha_color_lf, 0.01f);
	SHARED_STRUCT_ATTRIB(float, flt_min_alpha_color_spec, 0.01f);
	SHARED_STRUCT_ATTRIB(float, flt_min_alpha_moments_hf, 0.01f); /* minimum weight for the new frame data, moments channel, (0..1] */
	SHARED_STRUCT_ATTRIB(float, flt_scale_hf, 1); /* overall per-channel output scale, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, flt_scale_lf, 1);
	SHARED_STRUCT_ATTRIB(float, flt_scale_overlay, 1.0); /* scale for transparent and emissive objects visible with primary rays */
	SHARED_STRUCT_ATTRIB(float, flt_scale_spec, 1);
	SHARED_STRUCT_ATTRIB(float, flt_show_gradients, 0); /* switch for showing the gradient values as overlay image, 0 or 1 */
	SHARED_STRUCT_ATTRIB(int, flt_taa, 2); /* temporal anti-aliasing mode: 0 = off, 1 = regular TAA, 2 = temporal upscale */
	SHARED_STRUCT_ATTRIB(float, flt_taa_anti_sparkle, 0.25f); /* strength of the anti-sparkle filter of TAA, [0..1] */
	SHARED_STRUCT_ATTRIB(float, flt_taa_variance, 1.0f); /* temporal AA variance window scale, 0 means disable NCC, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, flt_taa_history_weight, 0.95f); /* temporal AA weight of the history sample, [0..1) */
	SHARED_STRUCT_ATTRIB(float, flt_temporal_hf, 1); /* temporal filter strength, [0..1] */
	SHARED_STRUCT_ATTRIB(float, flt_temporal_lf, 1);
	SHARED_STRUCT_ATTRIB(float, flt_temporal_spec, 1);
	SHARED_STRUCT_ATTRIB(float, pt_aperture, 0.0f); /* aperture size for the Depth of Field effect, in world units */
	SHARED_STRUCT_ATTRIB(float, pt_aperture_angle, 0); /* rotation of the polygonal aperture, [0..1] */
	SHARED_STRUCT_ATTRIB(float, pt_aperture_type, 0); /* number of aperture polygon edges, circular if less than 3 */
	SHARED_STRUCT_ATTRIB(float, pt_beam_softness, 1.0f); /* beam softness */
	SHARED_STRUCT_ATTRIB(float, pt_bump_scale, 1.0f); /* scale for normal maps [0..1] */
	SHARED_STRUCT_ATTRIB(float, pt_cameras, 1); /* switch for security cameras, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, pt_direct_polygon_lights, 1); /* switch for direct lighting from local polygon lights, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, pt_direct_roughness_threshold, 0.18f); /* roughness value where the path tracer switches direct light specular sampling from NDF based to light based, [0..1] */
	SHARED_STRUCT_ATTRIB(float, pt_direct_dyn_lights, 1); /* switch for direct lighting from local sphere lights, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, pt_direct_sun_light, 1); /* switch for direct lighting from the sun, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, pt_explosion_brightness, 4.0f); /* brightness factor for explosions */
	SHARED_STRUCT_ATTRIB(float, pt_fake_roughness_threshold, 0.20f); /* roughness value where the path tracer starts switching indirect light specular sampling from NDF based to SH based, [0..1] */
	SHARED_STRUCT_ATTRIB(float, pt_focus, 200); /* focal distance for the Depth of Field effect, in world units */
	SHARED_STRUCT_ATTRIB(float, pt_fog_brightness, 0.01f); /* global multiplier for the color of fog volumes */
	SHARED_STRUCT_ATTRIB(float, pt_indirect_polygon_lights, 1); /* switch for bounce lighting from local polygon lights, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, pt_indirect_dyn_lights, 1); /* switch for bounce lighting from local sphere lights, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, pt_light_stats, 1); /* switch for statistical light PDF correction, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, pt_max_log_sky_luminance, -3); /* maximum sky luminance, log2 scale, used for polygon light selection, (-inf..inf) */
	SHARED_STRUCT_ATTRIB(float, pt_min_log_sky_luminance, -10); /* minimum sky luminance, log2 scale, used for polygon light selection, (-inf..inf) */
	SHARED_STRUCT_ATTRIB(float, pt_metallic_override, -1); /* overrides metallic parameter of all materials if non-negative, [0..1] */
	SHARED_STRUCT_ATTRIB(float, pt_ndf_trim, 0.9f); /* trim factor for GGX NDF sampling (0..1] */
	SHARED_STRUCT_ATTRIB(int, pt_num_bounce_rays, 1); /* number of bounce rays, valid values are 0 (disabled), 0.5 (half-res diffuse), 1 (full-res diffuse + specular), 2 (two bounces) */
	SHARED_STRUCT_ATTRIB(float, pt_particle_softness, 0.7f); /* particle softness */
	SHARED_STRUCT_ATTRIB(float, pt_particle_brightness, 100); /* particle brightness */
	SHARED_STRUCT_ATTRIB(float, pt_reflect_refract, 2); /* number of reflection or refraction bounces: 0, 1 or 2 */
	SHARED_STRUCT_ATTRIB(float, pt_roughness_override, -1); /* overrides roughness of all materials if non-negative, [0..1] */
	SHARED_STRUCT_ATTRIB(float, pt_specular_anti_flicker, 2); /* fade factor for rough reflections of surfaces far away, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, pt_specular_mis, 1); /* enables the use of MIS between specular direct lighting and BRDF specular rays */
	SHARED_STRUCT_ATTRIB(float, pt_sun_bounce_range, 2000); /* range limiter for indirect lighting from the sun, helps reduce noise, (0..inf) */
	SHARED_STRUCT_ATTRIB(float, pt_sun_specular, 1.0f); /* scale for the direct specular reflection of the sun */
	SHARED_STRUCT_ATTRIB(float, pt_texture_lod_bias, 0); /* LOD bias for textures, (-inf..inf) */
	SHARED_STRUCT_ATTRIB(float, pt_toksvig, 1); /* intensity of Toksvig roughness correction, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, pt_thick_glass, 0); /* switch for thick glass refraction: 0 (disabled), 1 (reference mode only), 2 (real-time mode) */
	SHARED_STRUCT_ATTRIB(float, pt_water_density, 0.5f); /* scale for light extinction in water and other media, [0..inf) */
	SHARED_STRUCT_ATTRIB(float, tm_debug, 0); /* switch to show the histogram (1) or tonemapping curve (2) */
	SHARED_STRUCT_ATTRIB(float, tm_dyn_range_stops, 7.0f); /* Effective display dynamic range in linear stops = log2((max+refl)/(darkest+refl)) (eqn. 6), (-inf..0) */
	SHARED_STRUCT_ATTRIB(float, tm_enable, 1); /* switch for tone mapping, 0 or 1 */
	SHARED_STRUCT_ATTRIB(float, tm_exposure_bias, 0.0f); /* exposure bias, log-2 scale */
	SHARED_STRUCT_ATTRIB(float, tm_exposure_speed_down, 1); /* speed of exponential eye adaptation when scene gets darker, 0 means instant */
	SHARED_STRUCT_ATTRIB(float, tm_exposure_speed_up, 2); /* speed of exponential eye adaptation when scene gets brighter, 0 means instant */
	SHARED_STRUCT_ATTRIB(float, tm_blend_scale_border, 1); /* scale factor for full screen blend intensity, at screen border */
	SHARED_STRUCT_ATTRIB(float, tm_blend_scale_center, 0); /* scale factor for full screen blend intensity, at screen center */
	SHARED_STRUCT_ATTRIB(float, tm_blend_scale_fade_exp, 4); /* exponent used to interpolate between "border" and "center" factors */
	SHARED_STRUCT_ATTRIB(float, tm_blend_distance_factor, 1.2f); /* scale for the distance from the screen center when computing full screen blend intensity */
	SHARED_STRUCT_ATTRIB(float, tm_blend_max_alpha, 0.2f); /* maximum opacity for full screen blend effects */
	SHARED_STRUCT_ATTRIB(float, tm_high_percentile, 90); /* high percentile for computing histogram average, (0..100] */
	SHARED_STRUCT_ATTRIB(float, tm_knee_start, 0.6f); /* where to switch from a linear to a rational function ramp in the post-tonemapping process, (0..1)  */
	SHARED_STRUCT_ATTRIB(float, tm_low_percentile, 70); /* low percentile for computing histogram average, [0..100) */
	SHARED_STRUCT_ATTRIB(float, tm_max_luminance, 1.0f); /* auto-exposure maximum luminance, (0..inf) */
	SHARED_STRUCT_ATTRIB(float, tm_min_luminance, 0.0002f); /* auto-exposure minimum luminance, (0..inf) */
	SHARED_STRUCT_ATTRIB(float, tm_noise_blend, 0.5f); /* Amount to blend noise values between autoexposed and flat image [0..1] */
	SHARED_STRUCT_ATTRIB(float, tm_noise_stops, -12); /* Absolute noise level in photographic stops, (-inf..inf) */
	SHARED_STRUCT_ATTRIB(float, tm_reinhard, 0.5f); /* blend factor between adaptive curve tonemapper (0) and Reinhard curve (1) */
	SHARED_STRUCT_ATTRIB(float, tm_slope_blur_sigma, 12.0f); /* sigma for Gaussian blur of tone curve slopes, (0..inf) */
	SHARED_STRUCT_ATTRIB(float, tm_white_point, 10.0f); /* how bright colors can be before they become white, (0..inf) */
	SHARED_STRUCT_ATTRIB(float, tm_hdr_peak_nits, 800.0f); /* Exposure value 0 is mapped to this display brightness (post tonemapping) */
	SHARED_STRUCT_ATTRIB(float, tm_hdr_saturation_scale, 100); /* HDR mode saturation adjustment, percentage [0..200], with 0% -> desaturated, 100% -> normal, 200% -> oversaturated */
	SHARED_STRUCT_ATTRIB(float, ui_hdr_nits, 300); /* HDR mode UI (stretch pic) brightness in nits */
};

#ifndef __cplusplus
layout(set = GLOBAL_STORAGE_DESC_SET_IDX, binding = GLOBAL_UBO_BINDING_IDX, std140) uniform UBO
{
	GlobalUniformBuffer _globalUBO;
};
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
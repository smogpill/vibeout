// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#ifndef SHADER_INTERFACE_H
#define SHADER_INTERFACE_H

#ifdef __cplusplus
#define SHARED_STRUCT_ATTRIB(_type_, _name_, _defaultValue_) _type_ _name_ = _defaultValue_
#else
#define SHARED_STRUCT_ATTRIB(_type_, _name_, _defaultValue_) _type_ _name_
#endif

#ifdef __cplusplus
// std40 alignment:
// Ref: https://stackoverflow.com/questions/45638520/ubos-and-their-alignments-in-vulkan
using uint = uint32; // Alignment: 4
using vec2 = float[2]; // Alignment: 8
using vec3 = float[3]; // Alignment: 16
using vec4 = float[4]; // Alignment: 16
using uvec2 = uint32[2]; // Alignment: 8
using uvec3 = uint32[3]; // Alignment: 16
using uvec4 = uint32[4]; // Alignment: 16
using ivec2 = int32[2]; // Alignment: 8
using ivec3 = int32[3]; // Alignment: 16
using ivec4 = int32[4]; // Alignment: 16
using mat3 = float[3][3]; // Alignment: 16?
using mat4 = float[4][4]; // Alignment: 16?
#endif

#endif

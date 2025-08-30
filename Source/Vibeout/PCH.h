// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

#if VO_DEBUG
#define _ITERATOR_DEBUG_LEVEL 2
#endif

#include <cassert>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <format>
#include <utility>
#include <filesystem>
#include <memory>
#include <future>

#define GLM_FORCE_XYZW_ONLY
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/common.hpp>
#include <glm/matrix.hpp> 
#include <glm/trigonometric.hpp>
#include <glm/vector_relational.hpp>
#include <glm/ext/matrix_transform.hpp> 
#include <glm/ext/matrix_clip_space.hpp> 
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/vector_double3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_keyboard.h>

//#define VMA_VULKAN_VERSION 1003000
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <vk_mem_alloc.h>

#define STBI_FAILURE_USERMSG
#include <stb_image.h>

#include <tinyobjloader/tiny_obj_loader.h>

#define VC_EXTRALEAN
#define NOLANGUAGE
#define NOKEYBOARDINFO
#define NOLSTRING
#define NOGDI
#define NOMINMAX
#include <Windows.h>

#define JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
#define JPH_DISABLE_CUSTOM_ALLOCATOR
#define JPH_DOUBLE_PRECISION
#define JPH_DEBUG_RENDERER
#define JPH_PROFILE_ENABLED
#define JPH_OBJECT_STREAM
#define JPH_USE_AVX
#define JPH_USE_SSE4_1
#define JPH_USE_SSE4_2
#define JPH_USE_LZCNT
#define JPH_USE_TZCNT
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "Base/Base.h"

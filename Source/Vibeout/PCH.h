// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

#define VC_EXTRALEAN
#define NOLANGUAGE
#define NOKEYBOARDINFO
#define NOLSTRING
#define NOGDI
#define NOMINMAX
//#define VMA_VULKAN_VERSION 1003000
#define GLM_FORCE_XYZW_ONLY

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
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_keyboard.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>
#include <stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>
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
#include <Windows.h>
#include "Base/Base.h"

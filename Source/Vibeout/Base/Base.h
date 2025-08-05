// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint = unsigned int;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

inline void ReturnVoid(int) {} // to avoid some gcc warnings with the comma operator
std::string GetVulkanError(VkResult result);
void Info(const std::string& message);
void Error(const char* file, uint line, const std::string& message);
template <class T>
constexpr T AlignUp(const T val, const T alignment)
{
	return (val + (alignment - 1)) & ~(alignment - 1);
}

#ifdef _MSC_VER
#define VO_BREAKPOINT() ReturnVoid(IsDebuggerPresent() && (__debugbreak(), 1))
#else
#define VO_BREAKPOINT() std::breakpoint_if_debugging()
#endif
#define VO_ASSERT(_cond_) assert(_cond_)
#define VO_INFO(...) Info(std::format(__VA_ARGS__))
#define VO_ERROR(...) Error(__FILE__, __LINE__, std::format(__VA_ARGS__))
#define VO_SAFE_SCOPE(_x_) do { _x_ } while (!!false)
#define VO_TRY(_cond_, ...) VO_SAFE_SCOPE( if(!(_cond_)){ __VA_OPT__(VO_ERROR(__VA_ARGS__)); VO_BREAKPOINT(); return false; } )
#define VO_TRY_NO_RETURN_VALUE(_cond_, ...) VO_SAFE_SCOPE( if(!(_cond_)){ __VA_OPT__(VO_ERROR(__VA_ARGS__)); VO_BREAKPOINT(); return; } )
#define VO_CHECK(_cond_, ...) VO_SAFE_SCOPE( if(!(_cond_)){ __VA_OPT__(VO_ERROR(__VA_ARGS__)); VO_BREAKPOINT(); } )
#define VO_TRY_VK(_cond_, ...) VO_SAFE_SCOPE( VkResult _result_ = _cond_; VO_TRY(_result_ == VK_SUCCESS, "{}", GetVulkanError(_result_).c_str()); )
#define VO_CHECK_VK(_cond_, ...) VO_SAFE_SCOPE( VkResult _result_ = _cond_; VO_CHECK(_result_ == VK_SUCCESS,"{}", GetVulkanError(_result_).c_str()); )

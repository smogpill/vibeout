// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Core/Core.h"
#include "Core.h"

std::string GetVulkanError(VkResult result)
{
    return std::format("[Vulkan][{}]", string_VkResult(result));
}

void Error(const char* file, uint line, const std::string& message)
{
    const std::string text = std::format("{}({}): {}: {}\n", file, line, "error", message.c_str());
    std::cerr << text;
#ifdef _WIN32
    ::OutputDebugStringA(text.c_str());
#endif
}

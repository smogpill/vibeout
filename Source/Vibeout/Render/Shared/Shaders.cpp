// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Vibeout/Render/Shared/Shaders.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Base/Utils.h"

Shaders::Shaders(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

Shaders::~Shaders()
{
	for (const auto& [key, module] : _moduleMap)
		vkDestroyShaderModule(_renderer.GetDevice(), module, nullptr);
}

bool Shaders::Init()
{
	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);

	VO_TRY(InitModule("PrimaryRays.comp"));
	VO_TRY(InitModule("DirectLighting.comp"));
	VO_TRY(InitModule("IndirectLighting.comp"));
	VO_TRY(InitModule("BloomDownsample.comp"));
	VO_TRY(InitModule("BloomUpsampleCombine.comp"));
	VO_TRY(InitModule("BloomMix.comp"));
	VO_TRY(InitModule("ToneMappingHistogram.comp"));
	VO_TRY(InitModule("ToneMappingCurve.comp"));
	VO_TRY(InitModule("ToneMappingApply.comp"));
	VO_TRY(InitModule("AsvgfGradientImg.comp"));
	VO_TRY(InitModule("AsvgfGradientAtrous.comp"));
	VO_TRY(InitModule("AsvgfGradientReproject.comp"));
	VO_TRY(InitModule("AsvgfAtrous.comp"));
	VO_TRY(InitModule("AsvgfLf.comp"));
	VO_TRY(InitModule("AsvgfTemporal.comp"));
	VO_TRY(InitModule("AsvgfTaau.comp"));
	VO_TRY(InitModule("CheckerboardInterleave.comp"));
	VO_TRY(InitModule("Compositing.comp"));
	VO_TRY(InitModule("FinalBlit.vert"));
	VO_TRY(InitModule("FinalBlit.frag"));

	return true;
}

bool Shaders::InitModule(const std::string& name)
{
	std::filesystem::path path = std::format("Shaders/{}.spv", name);
	
	VO_TRY(std::filesystem::exists(path), "Can't read shader file: {}", path.string());
	std::vector<char> buffer;
	VO_TRY(ReadBinaryFile(path.string(), buffer));

	// Vulkan expects the buffer to be uint32_t aligned and sized.
	const uint32 alignedSize = (buffer.size() + 3) & ~3;
	buffer.resize(alignedSize, 0u);

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = buffer.size();
	info.pCode = (uint32*)buffer.data();

	VkDevice device = _renderer.GetDevice();
	VO_TRY(device);
	VkShaderModule module = nullptr;
	VO_TRY_VK(vkCreateShaderModule(device, &info, nullptr, &module));
	//_renderer.SetObjectName(module, name);
	_moduleMap[name] = module;

	return true;
}

VkShaderModule Shaders::GetShaderModule(const std::string& name) const
{
	auto it = _moduleMap.find(name);
	return it != _moduleMap.end() ? it->second : nullptr;
}

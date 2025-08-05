// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Renderer;

class Shaders
{
public:
	Shaders(Renderer& renderer, bool& result);
	~Shaders();

	VkShaderModule GetShaderModule(const std::string& name) const;

private:
	
	bool Init();
	bool InitModule(const std::string& name);

	Renderer& _renderer;
	std::unordered_map<std::string, VkShaderModule> _moduleMap;
};

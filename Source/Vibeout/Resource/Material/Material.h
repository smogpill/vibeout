// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
class Texture;

class Material
{
public:
	Material() = default;
	Material(const tinyobj::material_t& inputMaterial, bool& result);

	ResourceHandle<Texture> _diffuseTex;
	ResourceHandle<Texture> _specularTex;
	ResourceHandle<Texture> _normalTex;
	ResourceHandle<Texture> _metallicTex;
	ResourceHandle<Texture> _roughnessTex;
	ResourceHandle<Texture> _emissionTex;
	//std::shared_ptr<Texture> _displacementTex;
	//std::shared_ptr<Texture> _reflexionTex;
	glm::vec3 _diffuse = glm::vec3(1.0f);
	glm::vec3 _emission = glm::vec3(0.0f);
	float _metallic = 0.0f;
	float _roughness = 1.0f;

private:
	bool Init(const tinyobj::material_t& inputMaterial);
};

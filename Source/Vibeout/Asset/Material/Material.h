// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Texture;

class Material
{
public:
	static Material* LoadFromObj(const tinyobj::material_t& inputMaterial);

	std::shared_ptr<Texture> _diffuseTex;
	std::shared_ptr<Texture> _specularTex;
	std::shared_ptr<Texture> _normalTex;
	std::shared_ptr<Texture> _metallicTex;
	std::shared_ptr<Texture> _roughnessTex;
	std::shared_ptr<Texture> _emissionTex;
	//std::shared_ptr<Texture> _displacementTex;
	//std::shared_ptr<Texture> _reflexionTex;
	glm::vec3 _diffuse = glm::vec3(1.0f);
	glm::vec3 _emission = glm::vec3(0.0f);
	float _metallic = 0.0f;
	float _roughness = 1.0f;
};

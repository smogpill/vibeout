// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Material.h"
#include "Vibeout/Resource/Texture/Texture.h"

bool Material::OnLoad(ResourceLoader& loader, const tinyobj::material_t& inputMaterial, const std::string& folder)
{
	_roughness = inputMaterial.roughness;
	_metallic = inputMaterial.metallic;
	_diffuse = glm::vec3(inputMaterial.diffuse[0], inputMaterial.diffuse[1], inputMaterial.diffuse[2]);
	_emission = glm::vec3(inputMaterial.emission[0], inputMaterial.emission[1], inputMaterial.emission[2]);
	return true;
}

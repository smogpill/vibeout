// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Material.h"

Material* Material::LoadFromObj(const tinyobj::material_t& inputMaterial)
{
	Material* material = new Material();
	material->_roughness = inputMaterial.roughness;
	material->_metallic = inputMaterial.metallic;
	material->_diffuse = glm::vec3(inputMaterial.diffuse[0], inputMaterial.diffuse[1], inputMaterial.diffuse[2]);
	material->_emission = glm::vec3(inputMaterial.emission[0], inputMaterial.emission[1], inputMaterial.emission[2]);
	return material;
}

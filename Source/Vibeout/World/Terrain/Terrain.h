// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
class Texture;

class Terrain
{
public:
	Terrain(const ResourceHandle<Texture>& heightmap, const ResourceHandle<Texture>& diffuse, bool& result);
	auto Size() -> glm::ivec3& { return _size; }
	auto Size() const -> const glm::ivec3& { return _size; }
	auto Data() const -> const std::vector<uint16>& { return _data; }

private:
	bool Init();
	bool InitHeightmap();

	std::vector<uint16> _data;
	glm::ivec3 _size;
	ResourceHandle<Texture> _heightmapTex;
	ResourceHandle<Texture> _diffuseTex;
};

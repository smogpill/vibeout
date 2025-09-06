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
	auto GetHeightmapData() const -> const std::vector<uint16>& { return _data; }
	auto GetHeightmapTex() const -> const ResourceHandle<Texture>& { return _heightmapTex; }
	auto GetDiffuseTex() const -> const ResourceHandle<Texture>& { return _diffuseTex; }

private:
	bool Init();
	bool InitHeightmap();

	std::vector<uint16> _data;
	glm::ivec3 _size;
	ResourceHandle<Texture> _heightmapTex;
	ResourceHandle<Texture> _diffuseTex;
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
class Texture;

class MapResource : public Resource
{
	using Base = Resource;
public:

	bool OnLoad(ResourceLoader& loader) override;
	auto GetHeightmapTex() const -> const ResourceHandle<Texture>& { return _heightmapTex; }
	auto GetDiffuseTex() const -> const ResourceHandle<Texture>& { return _diffuseTex; }

private:
	ResourceHandle<Texture> _heightmapTex;
	ResourceHandle<Texture> _diffuseTex;
};

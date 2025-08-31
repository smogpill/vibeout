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

private:
	ResourceHandle<Texture> _heightmapTex;
};

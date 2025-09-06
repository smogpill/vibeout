// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Terrain.h"
#include "Vibeout/Resource/Texture/Texture.h"

Terrain::Terrain(const ResourceHandle<Texture>& heightmap, const ResourceHandle<Texture>& diffuse, bool& result)
	: _heightmapTex(heightmap)
	, _diffuseTex(diffuse)
{
	result = Init();
}

bool Terrain::Init()
{
	VO_TRY(InitHeightmap());
	return true;
}

bool Terrain::InitHeightmap()
{
	const Texture* heightmapTex = _heightmapTex.Get();
	VO_TRY(heightmapTex);
	const uint32 width = heightmapTex->GetWidth();
	const uint32 height = heightmapTex->GetHeigth();
	_size.x = width;
	_size.y = 0xffff;
	_size.z = height;
	const uint32 nbTexels = width * height;
	_data.resize(nbTexels);
	const uint8* bufferIt = static_cast<const uint8*>(heightmapTex->GetBuffer());
	const uint32 stride = heightmapTex->GetStrideBetweenTexels();
	const uint32 hd = heightmapTex->Is16Bits();

	if (hd)
	{
		for (uint32 texelIdx = 0, sourceOffset = 0; texelIdx < nbTexels; ++texelIdx)
		{
			_data[texelIdx] = *(const uint16*)bufferIt;
			bufferIt += stride;
		}
	}
	else
	{
		for (uint32 texelIdx = 0, sourceOffset = 0; texelIdx < nbTexels; ++texelIdx)
		{
			_data[texelIdx] = *bufferIt * 257; // 255 * 257 -> 65535
			bufferIt += stride;
		}
	}
	return true;
}

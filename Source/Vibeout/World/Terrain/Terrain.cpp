// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Terrain.h"

Terrain::Terrain(const char* path, bool& result)
{
	result = Init(path);
}

bool Terrain::Init(const char* path)
{
	int width, height, nbComponents;
	uint16* data = stbi_load_16(path, &width, &height, &nbComponents, 1);
	VO_TRY(data, "error loading heightmap texture: {}", path);
	_data.assign(data, data + width * height);
	stbi_image_free(data);
	_size.x = width;
	_size.y = 0xffff;
	_size.z = height;
	return true;
}

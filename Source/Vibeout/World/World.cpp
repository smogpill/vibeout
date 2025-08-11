// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "World.h"
#include "Vibeout/World/Heightmap/Heightmap.h"

World::World(const char* name, bool& result)
	: _name(name)
{
	result = Init();
}

World::~World()
{
	delete _heightmap;
}

bool World::Init()
{
	_path = (std::filesystem::path("Assets/Worlds") / _name).string();
	std::string heightmapPath = (std::filesystem::path(_path) / "Heightmap.png").string();
	bool result;
	_heightmap = new Heightmap(heightmapPath.c_str(), result);
	VO_TRY(result);
	return true;
}

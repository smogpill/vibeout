// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "World.h"
#include "Vibeout/World/Terrain/Terrain.h"
#include "Vibeout/World/Describer/WorldDescriber.h"
#include "Vibeout/World/Acceleration/SparseOctree/SparseOctreeBuilder.h"
#include "Vibeout/World/Acceleration/SparseOctree/SparseOctree.h"

World::World(const char* name, bool& result)
	: _name(name)
{
	result = Init();
}

World::~World()
{
	delete _tlas;
	delete _terrain;
}

bool World::Init()
{
	const std::string mapId = std::string("Maps") + "/" + _name;
	_path = (std::filesystem::path("Assets") / mapId).string();
	std::string heightmapPath = (std::filesystem::path(_path) / "Heightmap.png").string();
	bool result;
	_terrain = new Terrain(heightmapPath.c_str(), result);
	VO_TRY(result);
	WorldDescriber describer(*this);
	SparseOctreeBuilder builder;
	_tlas = builder.Build(8, describer);
	VO_TRY(_tlas);
	return true;
}

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
class Terrain; class SparseOctree; class Map; class MapResource;

class World
{
public:
	World(const char* name, bool& result);
	~World();

	auto GetPath() const -> const std::string& { return _path; }
	auto GetTerrain() const -> const Terrain* { return _terrain; }
	auto GetVersion() const { return _version; }
	auto GetTLAS() const { return _tlas; }

private:
	bool Init();

	std::string _name;
	std::string _path;
	Terrain* _terrain = nullptr;
	SparseOctree* _tlas = nullptr;
	Map* _map = nullptr;
	uint32 _version = 0;
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Heightmap;
class SparseOctree;

class World
{
public:
	World(const char* name, bool& result);
	~World();

	auto GetPath() const -> const std::string& { return _path; }
	auto GetHeightmap() const -> const Heightmap* { return _heightmap; }
	auto GetVersion() const { return _version; }

private:
	bool Init();

	std::string _name;
	std::string _path;
	Heightmap* _heightmap = nullptr;
	SparseOctree* _octree = nullptr;
	uint32 _version = 0;
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
#include "Vibeout/Base/Singleton.h"
class Terrain; class SparseOctree; class Map; class MapResource; class PhysicsWorld;

class World : public Singleton<World>
{
public:
	World();
	~World();

	void SetTerrain(Terrain* terrain);
	auto GetTerrain() const -> const Terrain* { return _terrain; }
	auto GetVersion() const { return _version; }
	auto GetTLAS() const { return _tlas; }
	void RebuildStaticTLAS();

private:
	Terrain* _terrain = nullptr;
	SparseOctree* _tlas = nullptr;
	Map* _map = nullptr;
	PhysicsWorld* _physicsWorld = nullptr;
	uint32 _version = 0;
};

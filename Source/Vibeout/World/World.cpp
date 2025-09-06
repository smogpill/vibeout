// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "World.h"
#include "Vibeout/World/Terrain/Terrain.h"
#include "Vibeout/World/Describer/WorldDescriber.h"
#include "Vibeout/World/Acceleration/SparseOctree/SparseOctreeBuilder.h"
#include "Vibeout/World/Acceleration/SparseOctree/SparseOctree.h"
#include "Vibeout/Physics/PhysicsWorld.h"

World::World()
{
	_physicsWorld = new PhysicsWorld();
}

World::~World()
{
	delete _tlas;
	delete _physicsWorld;
}

void World::SetTerrain(Terrain* terrain)
{
	_terrain = terrain;
}

void World::RebuildStaticTLAS()
{
	delete _tlas; _tlas = nullptr;
	WorldDescriber describer(*this);
	SparseOctreeBuilder builder;
	_tlas = builder.Build(8, describer);
	VO_ASSERT(_tlas);
}

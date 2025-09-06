// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
#include "Vibeout/Base/Singleton.h"
class Terrain; class SparseOctree; class MapResource; class PhysicsWorld; class Camera;

class World : public Singleton<World>
{
public:
	World();
	~World();

	void SetTerrain(Terrain* terrain);
	auto GetTerrain() const -> const Terrain* { return _terrain; }
	auto GetTerrainVersion() const { return _terrainVersion; }
	void SetCamera(Camera* camera);
	auto GetCamera() const -> const Camera& { return *_camera; }
	auto GetStaticTlasVersion() const { return _staticTlasVersion; }
	auto GetTLAS() const { return _tlas; }
	void RebuildStaticTLAS();

private:
	// Managers
	//----------------------------
	PhysicsWorld* _physicsWorld = nullptr;

	// Content
	//----------------------------
	Terrain* _terrain = nullptr;
	Camera* _camera = nullptr;

	// Internal state
	//----------------------------
	SparseOctree* _tlas = nullptr;
	uint32 _terrainVersion = 0;
	uint32 _staticTlasVersion = 0;

	// Misc
	//----------------------------
	Camera* _defaultCamera = nullptr;
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/GameBase.h"
#include "Vibeout/Base/Singleton.h"
#include "Vibeout/Resource/Resource.h"
class Map; class Craft; class Camera; class Model; class CraftPack; class GameStateMachine;
class Player;

class Game : public Singleton<Game>
{
public:
	Game();
	~Game();

	void SetMapName(const std::string& name);
	void SetAndGiveMap(Map* map);
	void SetCraftPack(const ResourceHandle<CraftPack>& craftPack);
	auto GetMap() const -> Map* { return _map; }
	void Update(float deltaTime);
	auto GetDeltaTime() const -> float { return _deltaTime; }
	auto GetCamera() -> Camera& { return *_camera; }
	void OnMouseMotion(float xrel, float yrel);
	auto GetMapName() const -> const std::string& { return _mapName; }

	static inline Game* _instance = nullptr;

private:
	void FixedUpdate(float deltaTime);

	static const float s_fixedTimeStep;

	// Setup
	//----------------------------
	std::string _mapName;

	// Runtime
	//----------------------------
	GameStateMachine* _stateMachine = nullptr;
	Map* _map = nullptr;
	Craft* _craft = nullptr;
	Camera* _camera = nullptr;
	Player* _player = nullptr;

	// Frame
	//----------------------------
	float _deltaTime = 0.0f;
	float _fixedUpdateAccumulator = 0.0f;

	// Resources
	//----------------------------
	ResourceHandle<CraftPack> _craftPack;
};

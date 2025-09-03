// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/GameBase.h"
#include "Vibeout/Base/Singleton.h"
#include "Vibeout/Resource/Resource.h"
class Map; class Craft; class World; class Camera; class Model; class CraftPack; class GameStateMachine;
class Player;

class Game : public Singleton<Game>
{
public:
	Game();
	~Game();

	void SetMap(const char* name);
	void SetCraftPack(const ResourceHandle<CraftPack>& craftPack);
	auto GetMap() const -> Map* { return _map; }
	void Update(float deltaTime);
	auto GetDeltaTime() const -> float { return _deltaTime; }
	auto GetCamera() -> Camera& { return *_camera; }
	void OnMouseMotion(float xrel, float yrel);
	auto GetWorld() const -> World* { return _world; }

	static inline Game* _instance = nullptr;

private:
	void FixedUpdate(float deltaTime);

	static const float s_fixedTimeStep;

	GameStateMachine* _stateMachine = nullptr;
	Map* _map = nullptr;
	World* _world = nullptr;
	Craft* _craft = nullptr;
	Camera* _camera = nullptr;
	Player* _player = nullptr;
	float _deltaTime = 0.0f;
	float _fixedUpdateAccumulator = 0.0f;
	ResourceHandle<CraftPack> _craftPack;
};

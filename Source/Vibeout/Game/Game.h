// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/GameBase.h"
#include "Vibeout/Base/Singleton.h"
#include "Vibeout/Resource/Resource.h"
class Map; class Craft; class World; class Camera;
class Model;

class Game : public Singleton<Game>
{
public:
	Game();
	~Game();

	void SetWorld(const char* name);

	void Update(float deltaTime);
	float GetDeltaTime() const { return _deltaTime; }
	Camera& GetCamera() { return *_camera; }
	const GameState& GetState() const { return _state; }
	void OnMouseMotion(float xrel, float yrel);
	World* GetWorld() const { return _world; }

	static inline Game* _instance = nullptr;

private:
	void FixedUpdate(float deltaTime);
	void LoadCraftModels();

	static const float s_fixedTimeStep;

	GameState _state = GameState::NONE;
	World* _world = nullptr;
	Craft* _craft = nullptr;
	Camera* _camera = nullptr;
	float _deltaTime = 0.0f;
	float _fixedUpdateAccumulator = 0.0f;

	std::vector<ResourceHandle<Model>> _craftModels;
};

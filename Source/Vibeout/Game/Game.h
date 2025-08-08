// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/GameBase.h"
class Map; class Craft; class World; class Camera;

class Game
{
public:
	Game();

	void Update(float deltaTime);
	float GetDeltaTime() const { return _deltaTime; }
	Camera* GetCurrentCamera() { return _currentCamera; }
	const GameState& GetState() const { return _state; }

private:
	void FixedUpdate(float deltaTime);
	void InitCameras();

	static const float s_fixedTimeStep;

	GameState _state = GameState::NONE;
	World* _world = nullptr;
	Craft* _craft = nullptr;
	Camera* _defaultCamera = nullptr;
	Camera* _currentCamera = nullptr;
	float _deltaTime = 0.0f;
	float _fixedUpdateAccumulator = 0.0f;
};

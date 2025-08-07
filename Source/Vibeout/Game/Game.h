// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Map; class Craft; class World; class Camera;

class Game
{
public:
	Game();

	void Update();
	float GetDeltaTime() const { return _deltaTime; }
	Camera* GetCamera() { return _camera; }

private:
	void FixedUpdate();

	World* _world = nullptr;
	float _deltaTime = 0.0f;
	Map* _map = nullptr;
	Craft* _craft = nullptr;
	Camera* _camera = nullptr;
};

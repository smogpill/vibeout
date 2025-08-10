// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Game.h"
#include "Vibeout/World/World.h"
#include "Vibeout/Game/Camera/Camera.h"

const float Game::s_fixedTimeStep = 1.0f / 60.0f;

Game::Game()
	: _camera(new Camera())
{
	// Spawn pos
	_camera->SetTranslation(glm::dvec3(0, 2, 0));
}
Game::~Game()
{
	delete _camera;
}

void Game::Update(float rawDeltaTime)
{
	_deltaTime = std::clamp(rawDeltaTime, 0.0f, 0.1f);
	_fixedUpdateAccumulator += _deltaTime;

	while (_fixedUpdateAccumulator >= s_fixedTimeStep)
	{
		FixedUpdate(s_fixedTimeStep);
		_fixedUpdateAccumulator -= s_fixedTimeStep;
	}

	_camera->OnUpdate(_deltaTime);
	//const float interpolation = _fixedUpdateAccumulator / s_fixedTimeStep;
}

void Game::OnMouseMotion(float xrel, float yrel)
{
	_camera->Rotate(xrel, yrel);
}

void Game::FixedUpdate(float deltaTime)
{
}

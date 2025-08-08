// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Game.h"
#include "Vibeout/World/World.h"
#include "Vibeout/Game/Camera/Camera.h"

const float Game::s_fixedTimeStep = 1.0f / 60.0f;

Game::Game()
{
	InitCameras();
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

	//const float interpolation = _fixedUpdateAccumulator / s_fixedTimeStep;
}

void Game::FixedUpdate(float deltaTime)
{
}

void Game::InitCameras()
{
	_defaultCamera = new Camera();
	_currentCamera = _defaultCamera;
}

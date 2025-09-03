// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/States/GameState.h"

class GameStateMachine : public GameState
{
	using Base = GameState;
public:
	GameStateMachine();
protected:
	void OnUpdate() override;
};

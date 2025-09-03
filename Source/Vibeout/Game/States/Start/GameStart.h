// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/States/GameState.h"

class GameStart : public GameState
{
	using Base = GameState;
public:
	GameStart() : Base(GameStateID::START) {}
protected:
	void OnEnter(State* from, const StateMessage& message) override;
	void OnUpdate() override;
	void OnExit(State* to, const StateMessage& message) override;
};

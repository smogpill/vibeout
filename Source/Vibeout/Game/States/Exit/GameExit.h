// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/States/GameState.h"

class GameExit : public GameState
{
	using Base = GameState;
public:
	GameExit() : Base(GameStateID::EXIT) {}
protected:
	void OnEnter(State* from, const StateMessage& message) override;

private:
};

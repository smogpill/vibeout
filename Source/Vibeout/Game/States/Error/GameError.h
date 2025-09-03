// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/States/GameState.h"

class GameError : public GameState
{
	using Base = GameState;
public:
	GameError() : Base(GameStateID::ERROR) {}
protected:
	void OnEnter(State* from, const StateMessage& message) override;

private:
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/States/GameState.h"
class Map;

class GameLoad : public GameState
{
	using Base = GameState;
public:
	GameLoad() : Base(GameStateID::LOAD) {}
protected:
	void OnEnter(State* from, const StateMessage& message) override;
	void OnUpdate() override;
	void OnExit(State* to, const StateMessage& message) override;

private:
	void OnMapLoadingDone(bool result);
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/States/GameState.h"

class GameRun : public GameState
{
	using Base = GameState;
public:
	GameRun() : Base(GameStateID::RUN) {}
protected:
	void OnEnter(StateMachine* from, const StateMessage& message) override;
	void OnUpdate() override;
	void OnExit(StateMachine* to, const StateMessage& message) override;

private:
	void UpdateCamera();
};

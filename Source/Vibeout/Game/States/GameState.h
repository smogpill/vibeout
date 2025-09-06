// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/StateMachine.h"
class Game;

enum class GameStateID
{
	NONE,
	BOOT,
	LOAD,
	START,
	RUN,
	EXIT,
	ERROR
};

class GameState : public StateMachine
{
	using Base = StateMachine;
public:
	using Base::Base;
};

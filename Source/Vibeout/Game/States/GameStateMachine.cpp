// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "GameStateMachine.h"
#include "Boot/GameBoot.h"
#include "Load/GameLoad.h"
#include "Start/GameStart.h"
#include "Run/GameRun.h"
#include "Exit/GameExit.h"

GameStateMachine::GameStateMachine()
	: Base(GameStateID::NONE)
{
	AddState(*new GameBoot());
	AddState(*new GameLoad());
	AddState(*new GameStart());
	AddState(*new GameRun());
	AddState(*new GameExit());
	SetCurrentState(GameStateID::BOOT);
}

void GameStateMachine::OnUpdate()
{
	Base::OnUpdate();
}

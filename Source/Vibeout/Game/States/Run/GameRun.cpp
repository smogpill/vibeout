// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "GameRun.h"

void GameRun::OnEnter(StateMachine* from, const StateMessage& message)
{
	Base::OnEnter(from, message);
}

void GameRun::OnUpdate()
{
	Base::OnUpdate();
}

void GameRun::OnExit(StateMachine* to, const StateMessage& message)
{
	Base::OnExit(to, message);
}

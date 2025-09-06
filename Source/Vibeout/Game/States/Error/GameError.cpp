// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "GameError.h"

void GameError::OnEnter(StateMachine* from, const StateMessage& message)
{
	Base::OnEnter(from, message);
	GetParent()->SetCurrentState(GameStateID::EXIT);
}

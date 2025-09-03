// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "GameLoad.h"
#include "Vibeout/Game/Game.h"
#include "Vibeout/Game/Map/Map.h"

void GameLoad::OnEnter(State* from, const StateMessage& message)
{
	Base::OnEnter(from, message);
	Map* map = Game::s_instance->GetMap();
	VO_ASSERT(map);
	map->LoadAsync([this](bool result) { OnMapLoadingDone(result); });
}

void GameLoad::OnUpdate()
{
	Base::OnUpdate();
}

void GameLoad::OnExit(State* to, const StateMessage& message)
{
	Base::OnExit(to, message);
}

void GameLoad::OnMapLoadingDone(bool result)
{
	GetParent()->SetCurrentState(result ? GameStateID::RUN : GameStateID::ERROR);
}

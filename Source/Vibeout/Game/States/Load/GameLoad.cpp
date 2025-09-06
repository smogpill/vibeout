// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "GameLoad.h"
#include "Vibeout/Game/Game.h"
#include "Vibeout/Game/Map/Map.h"
#include "Vibeout/Base/Job/JobSystem.h"
#include "Vibeout/World/World.h"

void GameLoad::OnEnter(StateMachine* from, const StateMessage& message)
{
	Base::OnEnter(from, message);

	_loadedFlags = 0;
	_map = new Map(Game::s_instance->GetMapName());
	_map->LoadAsync([this](bool result) { OnMapResourceLoadingDone(result); });
}

void GameLoad::OnUpdate()
{
	Base::OnUpdate();
}

void GameLoad::OnExit(StateMachine* to, const StateMessage& message)
{
	delete _map; _map = nullptr;
	Base::OnExit(to, message);
}

void GameLoad::OnMapResourceLoadingDone(bool result)
{
	if (!result)
	{
		GetParent()->SetCurrentState(GameStateID::ERROR);
		return;
	}

	JobSystem* jobSystem = JobSystem::s_instance;
	jobSystem->Enqueue([this]() { RegisterMap(); });
}

void GameLoad::RegisterMap()
{
	Game::s_instance->SetAndGiveMap(_map);
	_map = nullptr;
	AddLoadedFlag(LoadedFlag::MAP_LOADED);
}

void GameLoad::AddLoadedFlag(LoadedFlag flag)
{
	std::scoped_lock lock(_mutex);
	_loadedFlags |= (uint32)flag;
	const uint32 fullFlags = (uint32)LoadedFlag::END - 1;
	if (_loadedFlags == fullFlags)
	{
		GetParent()->SetCurrentState(GameStateID::RUN);
	}
}

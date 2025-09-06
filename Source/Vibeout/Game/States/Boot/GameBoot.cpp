// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "GameBoot.h"
#include "Vibeout/Resource/Manager/ResourceManager.h"
#include "Vibeout/Game/Game.h"

void GameBoot::OnEnter(StateMachine* from, const StateMessage& message)
{
	Base::OnEnter(from, message);
	ResourceManager* resourceManager = ResourceManager::s_instance;
	_craftPack = resourceManager->GetHandle<CraftPack>("CraftPack");
	_craftPack.LoadAsync();
	_craftPack.AddCallback([this](bool result) { OnCraftPackLoadingDone(result); });
}

void GameBoot::OnCraftPackLoadingDone(bool result)
{
	Game::s_instance->SetCraftPack(std::move(_craftPack));
	GetParent()->SetCurrentState(GameStateID::LOAD);
}

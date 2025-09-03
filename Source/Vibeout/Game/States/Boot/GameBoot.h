// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Game/States/GameState.h"
#include "Vibeout/Resource/Craft/CraftPack.h"

class GameBoot : public GameState
{
	using Base = GameState;
public:
	GameBoot() : Base(GameStateID::BOOT) {}
protected:
	void OnEnter(State* from, const StateMessage& message) override;

private:
	void OnCraftPackLoadingDone(bool result);

	ResourceHandle<CraftPack> _craftPack;
};

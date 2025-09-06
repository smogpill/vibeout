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
	void OnEnter(StateMachine* from, const StateMessage& message) override;
	void OnUpdate() override;
	void OnExit(StateMachine* to, const StateMessage& message) override;

private:
	enum class LoadedFlag
	{
		MAP_LOADED,
		END
	};
	void OnMapResourceLoadingDone(bool result);
	void RegisterMap();
	void AddLoadedFlag(LoadedFlag flag);
	
	Map* _map = nullptr;
	std::mutex _mutex;
	uint32 _loadedFlags = 0; // [_mutex]
};

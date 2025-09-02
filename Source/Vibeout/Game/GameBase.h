// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

enum class GameState
{
	NONE,
	LOADING_BASE_MODELS,
	LOADING_MAP,
	SPAWNING_WORLD,
	SPAWNING_CRAFTS,
	RUNNING,
};

enum class CameraType
{
	GAME,
	FREE,
	END
};

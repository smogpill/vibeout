// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

enum class GameState
{
	NONE,
	SPAWNING_WORLD,
	SPAWNING_PLAYER,
	RUNNING,
};

enum class CameraType
{
	GAME,
	FREE,
	END
};

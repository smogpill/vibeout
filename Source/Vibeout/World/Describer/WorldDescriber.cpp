// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "WorldDescriber.h"
#include "Vibeout/World/World.h"

WorldDescriber::WorldDescriber(const World& world)
	: _world(world)
	, _heightmap(world.GetHeightmap())
{
	VO_ASSERT(_heightmap);
}

bool WorldDescriber::Overlaps(const glm::ivec3& coords) const
{

	return false;
}

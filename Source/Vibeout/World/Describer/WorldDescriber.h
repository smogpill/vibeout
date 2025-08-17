// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Describer.h"
#include "Vibeout/World/Heightmap/Heightmap.h"
class World;

class WorldDescriber final : public Describer
{
public:
	WorldDescriber(const World& world);

	bool OverlapsNormalizedAABB(const AABB& aabb) const override;

private:
	const World& _world;
	const Heightmap* _heightmap = nullptr;
};

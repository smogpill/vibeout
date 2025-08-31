// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Describer.h"
class World; class Terrain;

class WorldDescriber final : public Describer
{
public:
	WorldDescriber(const World& world);

	auto OverlapsNormalizedAABB(const AABB& aabb) const -> OverlapType override;

private:
	const World& _world;
	const Terrain* _terrain = nullptr;
};

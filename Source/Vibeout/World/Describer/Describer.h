// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Math/AABB.h"

class Describer
{
public:
	virtual ~Describer() = default;

	//virtual bool Overlaps(const glm::ivec3& coords, uint level) const = 0;
	virtual bool OverlapsNormalizedAABB(const AABB& aabb) const = 0;
protected:
	//auto ConvertCoordsToLocalAABB(const glm::ivec3& coords, uint level) const -> AABB;
};

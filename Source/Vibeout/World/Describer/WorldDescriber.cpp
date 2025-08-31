// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "WorldDescriber.h"
#include "Vibeout/World/World.h"
#include "Vibeout/World/Terrain/Terrain.h"

WorldDescriber::WorldDescriber(const World& world)
	: _world(world)
	, _terrain(world.GetTerrain())
{
	VO_ASSERT(_terrain);
}

auto WorldDescriber::OverlapsNormalizedAABB(const AABB& aabb) const -> OverlapType
{
	const std::vector<uint16>& data = _terrain->Data();
	const glm::ivec3 size = _terrain->Size();
	const glm::ivec3 aabbMin = glm::ivec3(aabb.Min() * glm::vec3(size));
	const glm::ivec3 aabbMax = glm::min(glm::ivec3(aabb.Max() * glm::vec3(size)), size - 1);

	uint16 maxY = 0;
	for (int32 z = aabbMin.z; z <= aabbMax.z; ++z)
	{
		uint32 index = z * size.x;
		for (int32 x = aabbMin.x; x <= aabbMax.x; ++x)
		{
			maxY = std::max(data[index + x], maxY);
		}
	}
	const float relativeMaxY = (float)maxY / size.y;
	if ((aabb.Min().y) <= relativeMaxY)
	{
		if ((aabb.Max().y) <= relativeMaxY)
			return OverlapType::INTERIOR;
		else
			return OverlapType::INTERSECTION;
	}

	return OverlapType::NONE;
}

/*
auto Describer::ConvertCoordsToLocalAABB(const glm::ivec3& coords, uint32 level) const -> AABB
{
	//const float scale = std::bit_cast<float>(((~level) & 0xff) << 23u);
	//const glm::vec3 origin = glm::vec3(1) - ;
	return AABB(origin, origin + glm::vec3(scale));
}
*/

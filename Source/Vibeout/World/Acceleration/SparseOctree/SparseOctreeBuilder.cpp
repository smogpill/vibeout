// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "SparseOctreeBuilder.h"
#include "Vibeout/World/Describer/Describer.h"

void SparseOctreeBuilder::Reserve(uint32 nbNodes)
{
	_nodes.reserve(nbNodes);
}

SparseOctree* SparseOctreeBuilder::Build(uint32 nbLevels, const Describer& describer)
{
	_nodes.clear();
	_nodes.emplace_back(Node());

	struct Task
	{
		glm::ivec3 _coords;
		uint32 _node;
		uint32 _childIdx;
		uint32 _level;
	};
	Task stack[s_maxNblevels];
	uint32 stackSize = 0;

	// Add root
	{
		Task& task = stack[stackSize++];
		task._node = 0;
		task._level = 0;
		task._childIdx = 0;
		task._coords = glm::ivec3(0, 0, 0);
	}

	do
	{
		const Task task = stack[stackSize];
		Node& node = _nodes[task._node];
		glm::ivec3 childCoords = task._coords << 1;
		childCoords.x |= (task._childIdx >> 0) & 1;
		childCoords.y |= (task._childIdx >> 1) & 1;
		childCoords.z |= (task._childIdx >> 2) & 1;
		if (describer.Overlaps(childCoords))
		{
			const uint32 childNodeIdx = (uint32)_nodes.size();
			node._children[task._childIdx] = childNodeIdx;
			_nodes.emplace_back();
		}
	} while (stackSize);

	return nullptr;
}

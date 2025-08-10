// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "SparseOctreeBuilder.h"

SparseOctreeBuilder::SparseOctreeBuilder()
{
	_nodes.reserve(1024 * 1024);
}

SparseOctree* SparseOctreeBuilder::Build(uint32 nbLevels, const uint16* heightmap, uint heightmapWidth, uint heightmapHeight)
{
	_nodes.clear();
	_nodes.emplace_back(Node());

	struct Task
	{
		uint32 _node;
		uint32 _level;
	};
	Task stack[s_maxNblevels];

	{
		Task& task = stack[0];
		task._node = 0;
		task._level = 0;
	}

	uint32 stackSize = 1;

	do
	{
		const Task task = stack[--stackSize];


		Node& node = _nodes[nodeIdx];

	} while (stackSize);


}

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class SparseOctree;

class SparseOctreeBuilder
{
public:
	SparseOctreeBuilder();
	SparseOctree* Build(uint32 nbLevels, const uint16* heightmap, uint heightmapWidth, uint heightmapHeight);

private:
	static constexpr uint32 s_maxNblevels = 16;
	struct Node
	{
		uint32 _children[8] = {};
	};
	std::vector<Node> _nodes;
	uint _size = 0;
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class SparseOctree;
class SparseOctreeDescriptor;

class SparseOctreeBuilder
{
public:
	void Reserve(uint32 nbNodes);
	auto Build(uint32 nbLevels, const SparseOctreeDescriptor& descriptor) -> SparseOctree*;

private:
	static constexpr uint32 s_maxNblevels = 16;
	struct Node
	{
		uint32 _children[8] = {};
	};
	std::vector<Node> _nodes;
	uint32 _borderSize = 0;
	uint32 _firstFreeNode = uint32(-1);
};

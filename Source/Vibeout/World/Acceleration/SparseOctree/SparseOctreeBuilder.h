// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Math/Utils.h"
class SparseOctree;
class Describer;

class SparseOctreeBuilder
{
public:
	static constexpr uint32 s_maxNblevels = 16;

	~SparseOctreeBuilder();
	auto Build(uint32 nbLevels, const Describer& describer) -> SparseOctree*;

private:
	struct Node
	{
		uint32 _childPtrs[8] = {};
	};
	static constexpr uint32 s_blockAllocationSize = 4 * 1024;
	static constexpr uint32 s_perBlockCapacity = s_blockAllocationSize / sizeof(Node);
	static constexpr uint32 s_indexMask = NextPowerOfTwo(s_perBlockCapacity) - 1;
	static constexpr uint32 s_blockShift = std::popcount(s_indexMask);

	void BuildOctree(uint32 nbLevels, const Describer& describer);
	auto Encode() -> SparseOctree*;
	bool IsNodeAlive(const Node& node) const;
	auto CreateNode() -> uint32;
	void DestroyNode(uint32 node);
	auto TryNodeSubstitution(uint32 nodePtr) -> uint32;
	auto GetNode(uint32 nodePtr) -> Node&;

	std::vector<Node*> _nodeBlocks;
	uint32 _firstFreeNode = uint32(-1);
	uint32 _nbNodes = 0;
};

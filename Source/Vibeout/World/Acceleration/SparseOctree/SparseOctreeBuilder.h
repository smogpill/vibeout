// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class SparseOctree;
class Describer;

class SparseOctreeBuilder
{
public:
	static constexpr uint32 s_maxNblevels = 16;

	void Reserve(uint32 nbNodes);
	void Clear();
	auto Build(uint32 nbLevels, const Describer& describer) -> SparseOctree*;

private:
	struct Node
	{
		uint32 _childPtrs[8] = {};
	};

	void BuildOctree(uint32 nbLevels, const Describer& describer);

	SparseOctree* Encode();
	bool IsNodeAlive(uint32 nodeIdx) const;
	bool IsNodeAlive(const Node& node) const;
	uint32 GetNbNodes() const;
	uint32 GetNbFreeNodes() const { return 0; }
	uint32 CreateNode();
	void DestroyNode(uint32 node);
	uint32 TryNodeSubstitution(uint32 nodePtr);
	
	std::vector<Node> _nodesSparse;
	uint32 _borderSize = 0;
	uint32 _firstFreeNode = uint32(-1);
};

// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "SparseOctreeBuilder.h"
#include "Vibeout/World/Describer/Describer.h"
#include "Vibeout/World/Acceleration/SparseOctree/SparseOctree.h"

void SparseOctreeBuilder::Reserve(uint32 nbNodes)
{
	_nodesSparse.reserve(nbNodes);
}

void SparseOctreeBuilder::Clear()
{
	_nodesSparse.clear();
	_nodesSparse.emplace_back(Node());
	_firstFreeNode = uint32(-1);
}

SparseOctree* SparseOctreeBuilder::Build(uint32 nbLevels, const Describer& describer)
{
	Clear();
	BuildBreadthFirst(nbLevels, describer);
	SparseOctree* octree = Encode();
	return octree;
}

void SparseOctreeBuilder::BuildBreadthFirst(uint32 nbLevels, const Describer& describer)
{
	struct Task
	{
		glm::ivec3 _coords;
		uint32 _node;
		uint32 _childIdx;
		uint32 _level;
	};
	Task stack[s_maxNblevels];
	int32 stackIdx = 0;

	// Add root
	{
		Task& task = stack[stackIdx];
		task._node = CreateNode();
		task._level = 0;
		task._childIdx = 0;
		task._coords = glm::ivec3(0, 0, 0);
	}

	do
	{
		Task& task = stack[stackIdx];
		Node& node = _nodesSparse[task._node];
		glm::ivec3 childCoords = task._coords << 1;
		childCoords.x |= (task._childIdx >> 0) & 1;
		childCoords.y |= (task._childIdx >> 1) & 1;
		childCoords.z |= (task._childIdx >> 2) & 1;
		if (++task._childIdx == 8)
			--stackIdx;
		if (describer.Overlaps(childCoords))
		{
			if (task._level < nbLevels)
			{
				node._childPtrs[task._childIdx] = CreateNode();
			}
			else
			{
				node._childPtrs[task._childIdx] = uint32(-1);
			}
		}
	} while (stackIdx >= 0);
}

SparseOctree* SparseOctreeBuilder::Encode()
{
	if (GetNbNodes() == 0 || !IsNodeAlive(0))
		return nullptr;
	std::vector<uint32> compressedNodes;

	const uint32 nbNodes = GetNbNodes();
	compressedNodes.reserve(nbNodes);

	std::vector<uint32> stackedNodes;
	std::vector<uint32> rawToCompressed;
	stackedNodes.reserve(256); // Arbitrary
	rawToCompressed.resize(_nodesSparse.size(), 0);

	auto PushNewCompressedNode = [&](const uint32 nodePtr)
		{
			const Node& node = _nodesSparse[nodePtr];
			uint32 mask = 0;
			uint32 nbChildren = 0;
			for (uint8 childIdx = 0; childIdx < 8; ++childIdx)
				if (node._childPtrs[childIdx])
				{
					mask |= 1 << childIdx;
					++nbChildren;
				}

			uint32 compressedNodeIdx = (uint32)compressedNodes.size();
			uint32 newSize = compressedNodeIdx + 1 + nbChildren;
			compressedNodes.resize(newSize);
			compressedNodes[compressedNodeIdx] = mask;
			rawToCompressed[nodePtr] = compressedNodeIdx;

			if (nbChildren)
				stackedNodes.push_back(nodePtr);
			return compressedNodeIdx;
		};

	// Root node
	PushNewCompressedNode(0);

	while (stackedNodes.size())
	{
		const uint32 stackedNodeIdx = stackedNodes.back();
		stackedNodes.pop_back();
		const Node& node = _nodesSparse[stackedNodeIdx];
		const uint32 compressedNodeIdx = rawToCompressed[stackedNodeIdx];
		uint32 compressedChildPtrIdx = compressedNodeIdx + 1;
		for (uint8 childIdx = 0; childIdx < 8; ++childIdx)
		{
			const uint32 childPtr = node._childPtrs[childIdx];
			if (!childPtr)
				continue;

			uint32 compressedChildIdx = SparseOctree::s_fullInteriorNodePtr;
			if (childPtr != SparseOctree::s_fullInteriorNodePtr)
			{
				compressedChildIdx = rawToCompressed[childPtr];
				if (!compressedChildIdx)
					compressedChildIdx = PushNewCompressedNode(childPtr);
			}
			VO_ASSERT(compressedChildIdx);
			compressedNodes[compressedChildPtrIdx++] = compressedChildIdx;
		}
	}
	return new SparseOctree(std::move(compressedNodes));
}

bool SparseOctreeBuilder::IsNodeAlive(uint32 nodeIdx) const
{
	return IsNodeAlive(_nodesSparse[nodeIdx]);
}

bool SparseOctreeBuilder::IsNodeAlive(const Node& node) const
{
	return node._childPtrs[1] != uint32(-2);
}

uint32 SparseOctreeBuilder::GetNbNodes() const
{
	return (uint32)_nodesSparse.size() - GetNbFreeNodes();
}

uint32 SparseOctreeBuilder::CreateNode()
{
	const uint32 index = (uint32)_nodesSparse.size();
	_nodesSparse.emplace_back();
	return index;
}

void SparseOctreeBuilder::DestroyNode(uint32 nodeIdx)
{
	Node& node = _nodesSparse[nodeIdx];
	node._childPtrs[0] = _firstFreeNode;
	VO_ASSERT(IsNodeAlive(node));
	_firstFreeNode = nodeIdx;
}

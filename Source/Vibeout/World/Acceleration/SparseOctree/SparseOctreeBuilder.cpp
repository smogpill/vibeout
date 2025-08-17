// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "SparseOctreeBuilder.h"
#include "Vibeout/World/Describer/Describer.h"
#include "Vibeout/World/Acceleration/SparseOctree/SparseOctree.h"
#include "Vibeout/Math/AABB.h"

void SparseOctreeBuilder::Reserve(uint32 nbNodes)
{
	_nodesSparse.reserve(nbNodes);
}

void SparseOctreeBuilder::Clear()
{
	_nodesSparse.clear();
	_firstFreeNode = uint32(-1);
}

SparseOctree* SparseOctreeBuilder::Build(uint32 nbLevels, const Describer& describer)
{
	Clear();
	BuildOctree(nbLevels, describer);
	SparseOctree* octree = Encode();
	return octree;
}

void SparseOctreeBuilder::BuildOctree(uint32 nbLevels, const Describer& describer)
{
	struct Task
	{
		glm::ivec3 _coords;
		glm::vec3 _origin;
		uint32 _nodePtr;
		uint32 _childIdx;
		uint32 _idxInParent;
		uint32 _level;
		float _childScale;
	};
	Task stack[s_maxNblevels];
	int32 stackIdx = 0;

	// Add root
	{
		Task& task = stack[stackIdx];
		task._nodePtr = CreateNode();
		task._level = 0;
		task._childIdx = 0;
		task._idxInParent = 0;
		task._coords = glm::ivec3(0, 0, 0);
		task._origin = glm::ivec3(0, 0, 0);
		task._childScale = 0.5f;
	}

	AABB aabb;
	do
	{
		Task& task = stack[stackIdx];
		const uint32 nodePtr = task._nodePtr;
		Node& node = _nodesSparse[nodePtr];
		if (task._childIdx == 8)
		{
			--stackIdx;
			const uint32 newNodePtr = TryNodeSubstitution(nodePtr);
			if (newNodePtr != nodePtr && stackIdx >= 0)
			{
				Task& parentTask = stack[stackIdx];
				Node& parentNode = _nodesSparse[parentTask._nodePtr];
				parentNode._childPtrs[task._idxInParent] = newNodePtr;
			}
			continue;
		}
		glm::ivec3 childCoords = task._coords << 1;
		glm::ivec3 axes;
		axes[0] = (task._childIdx >> 0) & 1;
		axes[1] = (task._childIdx >> 1) & 1;
		axes[2] = (task._childIdx >> 2) & 1;
		childCoords.x |= axes[0];
		childCoords.y |= axes[1];
		childCoords.z |= axes[2];
		glm::vec3 childOrigin = task._origin;
		childOrigin += glm::vec3(axes) * task._childScale;
		aabb.Min() = childOrigin;
		aabb.Max() = childOrigin + glm::vec3(task._childScale);
		const uint32 childLevel = task._level + 1;
		if (describer.OverlapsNormalizedAABB(aabb))
		{
			if (task._level < nbLevels)
			{
				const uint32 nodePtr = CreateNode();
				node._childPtrs[task._childIdx] = nodePtr;
				Task& childTask = stack[++stackIdx];
				childTask._coords = childCoords;
				childTask._origin = childOrigin;
				childTask._nodePtr = nodePtr;
				childTask._childIdx = 0;
				childTask._idxInParent = task._childIdx;
				childTask._level = childLevel;
				childTask._childScale = task._childScale * 0.5f;
			}
			else
			{
				node._childPtrs[task._childIdx] = uint32(-1);
			}
		}
		++task._childIdx;
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

			uint32 compressedChildIdx = SparseOctree::s_interiorNodePtr;
			if (childPtr != SparseOctree::s_interiorNodePtr)
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
	uint32 index;
	if (_firstFreeNode == (uint32)-1)
	{
		index = (uint32)_nodesSparse.size();
		_nodesSparse.emplace_back();
	}
	else
	{
		index = _firstFreeNode;
		Node& node = _nodesSparse[_firstFreeNode];
		node = Node();
		_firstFreeNode = node._childPtrs[0];
	}
	return index;
}

void SparseOctreeBuilder::DestroyNode(uint32 nodeIdx)
{
	Node& node = _nodesSparse[nodeIdx];
	VO_ASSERT(IsNodeAlive(node));
	node._childPtrs[0] = _firstFreeNode;
	node._childPtrs[1] = uint32(-2);
	_firstFreeNode = nodeIdx;
}

uint32 SparseOctreeBuilder::TryNodeSubstitution(uint32 nodePtr)
{
	const Node& node = _nodesSparse[nodePtr];
	uint32 childIdx;
	for (childIdx = 0; childIdx < 8; ++childIdx)
	{
		if (node._childPtrs[childIdx] != SparseOctree::s_interiorNodePtr)
			break;
	}
	if (childIdx == 8)
	{
		DestroyNode(nodePtr);
		return SparseOctree::s_interiorNodePtr;
	}
	for (childIdx = 0; childIdx < 8; ++childIdx)
	{
		if (node._childPtrs[childIdx] != 0)
			break;
	}
	if (childIdx == 8)
	{
		DestroyNode(nodePtr);
		return 0;
	}

	return nodePtr;
}

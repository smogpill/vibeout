// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "SparseOctreeBuilder.h"
#include "Vibeout/World/Describer/Describer.h"
#include "Vibeout/World/Acceleration/SparseOctree/SparseOctree.h"
#include "Vibeout/Math/AABB.h"

SparseOctreeBuilder::~SparseOctreeBuilder()
{
	for (Node* block : _nodeBlocks)
		delete[] (char*)block;
}

SparseOctree* SparseOctreeBuilder::Build(uint32 nbLevels, const Describer& describer)
{
	BuildOctree(nbLevels, describer);
	return Encode();
}

void SparseOctreeBuilder::BuildOctree(uint32 nbLevels, const Describer& describer)
{
	struct Task
	{
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
		task._origin = glm::ivec3(0, 0, 0);
		task._childScale = 0.5f;
	}

	AABB aabb;
	do
	{
		Task& task = stack[stackIdx];
		const uint32 nodePtr = task._nodePtr;
		Node& node = GetNode(nodePtr);
		if (task._childIdx == 8)
		{
			--stackIdx;
			const uint32 newNodePtr = TryNodeSubstitution(nodePtr);
			if (newNodePtr != nodePtr && stackIdx >= 0)
			{
				Task& parentTask = stack[stackIdx];
				Node& parentNode = GetNode(parentTask._nodePtr);
				parentNode._childPtrs[task._idxInParent] = newNodePtr;
			}
			continue;
		}
		const glm::ivec3 axes(
			(task._childIdx >> 0) & 1,
			(task._childIdx >> 1) & 1,
			(task._childIdx >> 2) & 1);
		const glm::vec3 childOrigin = task._origin + glm::vec3(axes) * task._childScale;
		aabb.Min() = childOrigin;
		aabb.Max() = childOrigin + glm::vec3(task._childScale);
		switch (describer.OverlapsNormalizedAABB(aabb))
		{
		case Describer::OverlapType::INTERSECTION:
		{
			if (task._level < nbLevels)
			{
				const uint32 childNodePtr = CreateNode();
				node._childPtrs[task._childIdx] = childNodePtr;
				Task& childTask = stack[++stackIdx];
				childTask._origin = childOrigin;
				childTask._nodePtr = childNodePtr;
				childTask._childIdx = 0;
				childTask._idxInParent = task._childIdx;
				childTask._level = task._level + 1;
				childTask._childScale = task._childScale * 0.5f;
			}
			else
			{
				node._childPtrs[task._childIdx] = SparseOctree::s_interiorNodePtr;
			}
			break;
		}
		case Describer::OverlapType::INTERIOR:
		{
			node._childPtrs[task._childIdx] = SparseOctree::s_interiorNodePtr;
			break;
		}
		}
		++task._childIdx;
	} while (stackIdx >= 0);
}

SparseOctree* SparseOctreeBuilder::Encode()
{
	std::vector<uint32> compressedNodes;

	compressedNodes.reserve(_nbNodes);

	std::vector<uint32> stackedNodes;
	std::vector<uint32> rawToCompressed;
	stackedNodes.reserve(256); // Arbitrary
	rawToCompressed.resize(_nodeBlocks.size() * s_perBlockCapacity, 0);

	auto PushNewCompressedNode = [&](const uint32 nodePtr)
		{
			const Node& node = GetNode(nodePtr);
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
		const Node& node = GetNode(stackedNodeIdx);
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

bool SparseOctreeBuilder::IsNodeAlive(const Node& node) const
{
	return node._childPtrs[1] != uint32(-2);
}

uint32 SparseOctreeBuilder::CreateNode()
{
	if (_firstFreeNode == (uint32)-1)
	{
		const uint32 blockIdx = (uint32)_nodeBlocks.size();
		const uint32 firstNodePtr = blockIdx << s_blockShift;
		uint32 nodePtr = firstNodePtr;
		char* buffer = new char[s_blockAllocationSize];
		Node* nodeBuffer = (Node*)buffer;
		for (uint32 nodeIdx = 0; nodeIdx < s_perBlockCapacity; ++nodeIdx)
		{
			Node* node = &nodeBuffer[nodeIdx];
			new (node) Node();
			node->_childPtrs[0] = ++nodePtr;
			node->_childPtrs[1] = (uint32)-2;
		}
		nodeBuffer[s_perBlockCapacity - 1]._childPtrs[0] = uint32(-1);
		_nodeBlocks.push_back(nodeBuffer);
		_firstFreeNode = firstNodePtr;
	}

	const uint32 index = _firstFreeNode;
	Node& node = GetNode(_firstFreeNode);
	_firstFreeNode = node._childPtrs[0];
	node = Node();
	++_nbNodes;
	return index;
}

void SparseOctreeBuilder::DestroyNode(uint32 nodePtr)
{
	Node& node = GetNode(nodePtr);
	VO_ASSERT(IsNodeAlive(node));
	--_nbNodes;
	node._childPtrs[0] = _firstFreeNode;
	node._childPtrs[1] = uint32(-2);
	_firstFreeNode = nodePtr;
}

uint32 SparseOctreeBuilder::TryNodeSubstitution(uint32 nodePtr)
{
	const Node& node = GetNode(nodePtr);
	uint32 childIdx;

	if (nodePtr)
	{
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

auto SparseOctreeBuilder::GetNode(uint32 nodePtr) -> Node&
{
	const uint32 blockIdx = nodePtr >> s_blockShift;
	const uint32 indexInBlock = nodePtr & s_indexMask;
	return _nodeBlocks[blockIdx][indexInBlock];
}

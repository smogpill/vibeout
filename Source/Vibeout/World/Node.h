// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Transform.h"

class Node
{
public:
	~Node();
	void SetParent(Node* parent);
	void SetLocalTransform(const Transform& transform);
	void SetWorldTransform(const Transform& transform);
	const Transform& GetLocalTransform() const;
	const Transform& GetGlobalTransform() const;
private:
	void UpdateLocalTransformIfNecessary() const;
	void UpdateWorldTransformIfNecessary() const;

	Node* _parent = nullptr;
	Node* _firstChild = nullptr;
	Node* _nextSibling = nullptr;
	mutable Transform _localTransform;
	mutable Transform _worldTransform;
	mutable bool _dirtyWorld = false;
	mutable bool _dirtyLocal = false;
};

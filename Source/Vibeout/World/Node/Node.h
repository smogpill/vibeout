// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Math/Transform.h"

class Node
{
public:
	~Node();
	void SetParent(Node* parent);
	void SetLocalTransform(const Transform& transform);
	void SetGlobalTransform(const Transform& transform);
	auto GetLocalTransform() const -> const Transform&;
	auto GetGlobalTransform() const -> const Transform&;
private:
	void UpdateLocalTransformIfNecessary() const;
	void UpdateGlobalTransformIfNecessary() const;

	Node* _parent = nullptr;
	Node* _firstChild = nullptr;
	Node* _nextSibling = nullptr;
	mutable Transform _localTransform;
	mutable Transform _globalTransform;
	mutable bool _dirtyGlobal = false;
	mutable bool _dirtyLocal = false;
};

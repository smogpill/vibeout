// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Node.h"

Node::~Node()
{
	SetParent(nullptr);
}

void Node::SetParent(Node* parent)
{
	if (_parent == parent)
		return;
	UpdateWorldTransformIfNecessary();
	_dirtyLocal = true;
	if (_parent)
	{
		Node** it = &_parent->_firstChild;
		while (*it != this)
			it = &(*it)->_nextSibling;
		*it = _nextSibling;
	}
	_parent = parent;
	if (_parent)
	{
		_nextSibling = _parent->_firstChild;
		_parent->_firstChild = this;
	}
}

void Node::SetLocalTransform(const Transform& transform)
{
	_localTransform = transform;
	_dirtyWorld = true;
}

void Node::SetWorldTransform(const Transform& transform)
{
	_worldTransform = transform;
	_dirtyLocal = true;
}

const Transform& Node::GetLocalTransform() const
{
	UpdateLocalTransformIfNecessary();
	return _localTransform;
}

const Transform& Node::GetGlobalTransform() const
{
	UpdateWorldTransformIfNecessary();
	return _worldTransform;
}

void Node::UpdateLocalTransformIfNecessary() const
{
	if (_dirtyLocal)
	{
		_dirtyLocal = false;
		if (_parent)
			_localTransform = _worldTransform * Inverse(_parent->GetGlobalTransform());
		else
			_localTransform = _worldTransform;
	}
}

void Node::UpdateWorldTransformIfNecessary() const
{
	if (_dirtyWorld)
	{
		_dirtyWorld = false;
		if (_parent)
			_worldTransform = _localTransform * _parent->GetGlobalTransform();
		else
			_worldTransform = _localTransform;
	}
}

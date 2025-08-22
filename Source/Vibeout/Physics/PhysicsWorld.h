// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Physics/Base/PhysicsBase.h"

namespace JPH
{
	class PhysicsSystem;
	class ObjectLayerPairFilter;
	class BroadPhaseLayerInterface;
	class ObjectVsBroadPhaseLayerFilter;
}

class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	void Optimize();
	
private:
	struct Impl;

	JPH::PhysicsSystem* _systemJPH = nullptr;
	Impl* _impl = nullptr;
};

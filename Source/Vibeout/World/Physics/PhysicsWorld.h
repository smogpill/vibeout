// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/World/Physics/Base/PhysicsBase.h"
#include "Vibeout/Base/Singleton.h"

namespace JPH
{
	class PhysicsSystem;
	class ObjectLayerPairFilter;
	class BroadPhaseLayerInterface;
	class ObjectVsBroadPhaseLayerFilter;
}

class PhysicsWorld : public Singleton<PhysicsWorld>
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

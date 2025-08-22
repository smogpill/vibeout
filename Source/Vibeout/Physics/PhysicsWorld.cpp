// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "PhysicsWorld.h"

static void TraceImpl(const char* inFMT, ...)
{
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);
	std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
{
	std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;
	return true;
};
#endif

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer layerA, JPH::ObjectLayer layerB) const override
	{
		return layerA == JPH::ObjectLayer(PhysicsLayer::DYNAMIC) || layerB == JPH::ObjectLayer(PhysicsLayer::DYNAMIC);
	}
};

class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	uint GetNumBroadPhaseLayers() const override
	{
		return (uint)PhysicsLayer::END;
	}

	JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
	{
		return JPH::BroadPhaseLayer((uint8)layer);
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		return "don't care";
	}
#endif
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer layerA, JPH::BroadPhaseLayer layerB) const override
	{
		return layerA == JPH::ObjectLayer(PhysicsLayer::DYNAMIC) || layerB.GetValue() == (uint8)PhysicsLayer::STATIC;
	}
};

// To keep things close in memory
struct PhysicsWorld::Impl
{
	BroadPhaseLayerInterfaceImpl _broadPhaseLayerInterface;
	ObjectVsBroadPhaseLayerFilterImpl _objectVsBroadPhaseLayerFilter;
	ObjectLayerPairFilterImpl _objectLayerPairFilter;
};

PhysicsWorld::PhysicsWorld()
{
	JPH::RegisterDefaultAllocator();
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl);
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();
	JPH::TempAllocatorImpl tempAllocator(10 * 1024 * 1024);
	JPH::JobSystemThreadPool jobSystem(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
	_impl = new Impl();
	const uint maxBodies = 1024;
	const uint nbBodyMutexes = 0; // 0 fetches the default
	const uint maxBodyPairs = 1024;
	const uint maxContactConstraints = 1024;
	_systemJPH = new JPH::PhysicsSystem();
	_systemJPH->Init(maxBodies, nbBodyMutexes, maxBodyPairs, maxContactConstraints, _impl->_broadPhaseLayerInterface, _impl->_objectVsBroadPhaseLayerFilter, _impl->_objectLayerPairFilter);
}

PhysicsWorld::~PhysicsWorld()
{
	delete _systemJPH;
	delete _impl;
	JPH::UnregisterTypes();
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

void PhysicsWorld::Optimize()
{
	_systemJPH->OptimizeBroadPhase();
}

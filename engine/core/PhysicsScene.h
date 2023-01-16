#pragma once
#include <list>

#include <Jolt/Jolt.h>

#include <Jolt\Physics\Collision\ObjectLayer.h>
#include <Jolt\Physics\Collision\BroadPhase\BroadPhaseLayer.h>

#include "FireActivationListener.h"
#include "FireContactListener.h"
#include "Layers.h"


class IFixedUpdateListener {
public:
	virtual void BeforePhysicsUpdate() = 0;
};

#pragma region Layers


/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool					ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

/// Возвращает какой-то BroadPhaseLayer для ObjectLayer
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual uint					GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer			GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		switch ((JPH::BroadPhaseLayer::Type)inLayer)
		{
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

#pragma endregion

namespace JPH {
	class PhysicsSystem;
	class BodyInterface;
}
class Rigidbody;

class PhysicsScene {

public:
	// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const JPH::uint cMaxBodies = 1024;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
	const JPH::uint cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const JPH::uint cMaxBodyPairs = 1024;

	// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
	const JPH::uint cMaxContactConstraints = 1024;

	const float cWorldScale = 20.0f;

public:
	std::list<IFixedUpdateListener*> rigidbodies;

private:
	JPH::PhysicsSystem* m_physicsSystem;
	JPH::BodyInterface* m_bodyInterface;

	FireActivationListener m_activationListener;
	FireContactListener m_contactListener;
	
	// mapping table from object layer to broadphase layer
	BPLayerInterfaceImpl broad_phase_layer_interface;

	// class that filters object vs broadphase layers
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

	// class that filters object vs object layers
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

public:
	void Init();
	void Destroy();

	void BeginUpdate();

	inline JPH::PhysicsSystem* physicsSystem() { return m_physicsSystem; }
	inline JPH::BodyInterface* bodyInterface() { return m_bodyInterface; }

};


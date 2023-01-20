#include "PhysicsScene.h"
#include "Rigidbody.h"

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt\Physics\Body\BodyInterface.h>

#include "JoltExtantions.h"

using namespace JPH;

void PhysicsScene::Init() {
	// Now we can create the actual physics system.
	m_physicsSystem = new PhysicsSystem();
	m_physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

	auto settings = m_physicsSystem->GetPhysicsSettings();
	settings.mSpeculativeContactDistance *= cWorldScale;
	settings.mPenetrationSlop *= cWorldScale;
	settings.mMaxPenetrationDistance *= cWorldScale;
	settings.mPointVelocitySleepThreshold *= cWorldScale;
	m_physicsSystem->SetPhysicsSettings(settings);

	//auto gravity = FromJolt(m_physicsSystem->GetGravity());
	//gravity *= cWorldScale;
	//m_physicsSystem->SetGravity(ToJolt(gravity));

	// A body activation listener gets notified when bodies activate and go to sleep
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	m_physicsSystem->SetBodyActivationListener(&m_activationListener);

	// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	m_physicsSystem->SetContactListener(&m_contactListener);

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	m_bodyInterface = &m_physicsSystem->GetBodyInterface(); 

	m_contactListener.Init(m_physicsSystem);
}

void PhysicsScene::Destroy() {
	delete m_physicsSystem;
	m_physicsSystem = nullptr;
}

void PhysicsScene::BeginUpdate() {
	for (auto body : rigidbodies)
		body->BeforePhysicsUpdate();
}


#pragma region ObjectLayerPairFilterImpl

bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
{
	switch (inObject1)
	{
	case Layers::NON_MOVING:
		return inObject2 == Layers::MOVING; // Non moving only collides with moving
	case Layers::MOVING:
		return true; // inObject2 != Layers::TRIGGER; // Moving collides with everything
	case Layers::TRIGGER:
		return inObject2 == Layers::MOVING;
	default:
		JPH_ASSERT(false);
		return false;
	}
}

#pragma endregion

#pragma region ObjectVsBroadPhaseLayerFilterImpl

bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
{
	switch (inLayer1)
	{
	case Layers::NON_MOVING:
		return inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::MOVING:
		return true;
	case Layers::TRIGGER:
		return inLayer2 == BroadPhaseLayers::MOVING;
	default:
		JPH_ASSERT(false);
		return false;
	}
}

#pragma endregion

#pragma region BPLayerInterfaceImpl

BPLayerInterfaceImpl::BPLayerInterfaceImpl() {
	// Create a mapping table from object to broad phase layer
	mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
	mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	mObjectToBroadPhase[Layers::TRIGGER] = BroadPhaseLayers::NON_MOVING;
}

JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const {
	return BroadPhaseLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const {
	JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
	return mObjectToBroadPhase[inLayer];
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const {

	switch ((JPH::BroadPhaseLayer::Type)inLayer)
	{
	case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
	case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
	default:														JPH_ASSERT(false); return "INVALID";
	}
}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED


#pragma endregion



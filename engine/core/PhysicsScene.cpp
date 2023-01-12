#include "PhysicsScene.h"

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt\Physics\Body\BodyInterface.h>

using namespace JPH;

void PhysicsScene::Init() {
	// Now we can create the actual physics system.
	m_physicsSystem = new PhysicsSystem();
	m_physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

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
}

void PhysicsScene::Destroy() {
	delete m_physicsSystem;
	m_physicsSystem = nullptr;
}

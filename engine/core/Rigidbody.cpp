#include "Rigidbody.h"
#include "BoxCollider.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt\Physics\Body\BodyInterface.h>
#include <Jolt\Physics\Body\Body.h>

#include "Game.h"
#include "Scene.h"
#include "PhysicsScene.h"
#include "JoltExtantions.h"

using namespace JPH;

DEF_COMPONENT(Rigidbody, Engine.Rigidbody, 0, RunMode::PlayOnly) {

}

void Rigidbody::OnInit() {
	m_collider = GetComponent<Collider>();
	if (m_collider == nullptr)
		throw std::exception("Rigidbody: collider == nullptr");
}

void Rigidbody::OnStart() {
	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	m_rigidbodyIter = physicsScene->rigidbodies.insert(physicsScene->rigidbodies.end(), this);

	// Create the actual rigid body
	// Note that if we run out of bodies this can return nullptr
	m_collider->settings.mGravityFactor = 20;
	m_body = bodyInterface->CreateBody(m_collider->settings);
	
	// Add it to the world
	bodyInterface->AddBody(m_body->GetID(), EActivation::Activate);
}

void Rigidbody::OnDestroy() {
	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	bodyInterface->RemoveBody(m_body->GetID());
	bodyInterface->DestroyBody(m_body->GetID());

	//m_body->GetShape()->ScaleShape({ 2,2,2 });

	physicsScene->rigidbodies.erase(m_rigidbodyIter);
}

void Rigidbody::OnBeginPhysicsUpdate() {
	if (m_body->IsStatic())
		return;

	auto position = ToJolt(worldPosition()); 
	auto rotation = ToJolt(worldRotationQ());

	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	bodyInterface->SetPositionAndRotationWhenChanged(m_body->GetID(), position, rotation, EActivation::Activate);
}

void Rigidbody::OnFixedUpdate() {
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	worldPosition(FromJolt(m_body->GetPosition()));
	worldRotationQ(FromJolt(m_body->GetRotation()));
}



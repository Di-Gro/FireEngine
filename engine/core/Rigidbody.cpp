#include "Rigidbody.h"
#include "BoxCollider.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt\Physics\Body\BodyInterface.h>
#include <Jolt\Physics\Body\Body.h>

#include "Scene.h"
#include "PhysicsScene.h"

using namespace JPH;

DEF_COMPONENT(Rigidbody, Engine.Rigidbody, 0, RunMode::PlayOnly) {

}

void Rigidbody::OnInit() {
	m_collider = GetComponent<Collider>();
	if (m_collider == nullptr)
		throw std::exception("Rigidbody: collider == nullptr");
}

void Rigidbody::OnStart() {
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	// Create the actual rigid body
	// Note that if we run out of bodies this can return nullptr
	m_body = bodyInterface->CreateBody(m_collider->settings);

	// Add it to the world
	bodyInterface->AddBody(m_body->GetID(), EActivation::Activate);
}

void Rigidbody::OnDestroy() {
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	bodyInterface->RemoveBody(m_body->GetID());
	bodyInterface->DestroyBody(m_body->GetID());
}

void Rigidbody::OnFixedUpdate() {
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	auto wpos = m_body->GetPosition();
	auto wrot = m_body->GetRotation();

	worldPosition({ wpos.GetX(), wpos.GetY(), wpos.GetZ() });
	worldRotationQ(Quaternion::CreateFromYawPitchRoll({ wrot.GetX(), wrot.GetY(), wrot.GetZ() }));

}

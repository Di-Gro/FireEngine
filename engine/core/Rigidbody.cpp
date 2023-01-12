#include "Rigidbody.h"
#include "BoxCollider.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt\Physics\Body\BodyInterface.h>
#include <Jolt\Physics\Body\Body.h>

#include "Game.h"
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
	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	m_rigidbodyIter = physicsScene->rigidbodies.insert(physicsScene->rigidbodies.end(), this);

	// Create the actual rigid body
	// Note that if we run out of bodies this can return nullptr
	m_body = bodyInterface->CreateBody(m_collider->settings);

	// Add it to the world
	bodyInterface->AddBody(m_body->GetID(), EActivation::Activate);
}

void Rigidbody::OnDestroy() {
	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	bodyInterface->RemoveBody(m_body->GetID());
	bodyInterface->DestroyBody(m_body->GetID());

	physicsScene->rigidbodies.erase(m_rigidbodyIter);
}

void Rigidbody::OnBeginPhysicsUpdate() {
	if (m_body->IsStatic())
		return;

	auto wpos = worldPosition();
	auto wrot = worldRotationQ();

	auto position = RVec3Arg(wpos.x, wpos.y, wpos.z);
	auto rotation = QuatArg(wrot.x, wrot.y, wrot.z, wrot.w);

	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	bodyInterface->SetPositionAndRotationWhenChanged(m_body->GetID(), position, rotation, EActivation::Activate);

}

void Rigidbody::OnFixedUpdate() {
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	auto wpos = m_body->GetPosition();
	auto wrot = m_body->GetRotation();

	worldPosition({ wpos.GetX(), wpos.GetY(), wpos.GetZ() });
	worldRotationQ(Quaternion( wrot.GetX(), wrot.GetY(), wrot.GetZ(), wrot.GetW() ));
}



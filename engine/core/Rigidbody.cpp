#include "Rigidbody.h"
#include "BoxCollider.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt\Physics\Body\BodyInterface.h>
#include <Jolt\Physics\Body\Body.h>
#include <Jolt/Physics/Body/MotionType.h>

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


void Rigidbody::AddForce(Vector3 inForce) {

	if (m_body) {
		m_body->AddForce(ToJolt(inForce));
	}

}

void Rigidbody::AddForce(Vector3 inForce, Vector3 inPosition) {
	if (m_body) {
		m_body->AddForce(ToJolt(inForce), ToJolt(inPosition));
	}
}

Vector3 Rigidbody::GetLinearVelocity() const {
	if (m_body) {
		return FromJolt(m_body->GetLinearVelocity());
	}
}
void Rigidbody::SetLinearVelocityClamped(Vector3 inLinearVelocity) {
	if (m_body) {
		m_body->SetLinearVelocity(ToJolt(inLinearVelocity));
	}
}
void Rigidbody::SetMass(float mass) { //TODO Хз можно ли так, есть еще метод SetShapeInternal, возможном нужно через него
	if (m_body) {
		auto body_shape = m_body->GetShape();
		auto mass_prop = body_shape->GetMassProperties();
		mass_prop.mMass = mass;
	}
}


void Rigidbody::AddImpulse(Vector3 inImpulse) {

	if (m_body) {
		m_body->AddImpulse(ToJolt(inImpulse));
	}

}

void Rigidbody::AddImpulse(Vector3 inImpulse, Vector3 inPosition) {
	if (m_body) {
		m_body->AddImpulse(ToJolt(inImpulse), ToJolt(inPosition));
	}
}
bool  Rigidbody::IsKinematic() const {
	if (m_body)
		return m_body->IsKinematic();
}
bool  Rigidbody::IsDynamic() const {
	if (m_body)
		return m_body->IsDynamic();
}
bool Rigidbody::IsStatic() const {
	if (m_body)
		return m_body->IsStatic();
}
float Rigidbody::GetFriction() const {
	if (m_body)
		return m_body->GetFriction();
}
void Rigidbody::SetFriction(float inFriction) {
	if (m_body)
		return m_body->SetFriction(inFriction);
}
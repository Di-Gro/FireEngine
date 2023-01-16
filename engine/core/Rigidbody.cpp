#include "Rigidbody.h"

#include <Jolt\Physics\Body\BodyInterface.h>
#include <Jolt\Physics\Body\Body.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include "Game.h"
#include "UI\UserInterface.h"
#include "Scene.h"
#include "HotKeys.h"
#include "PhysicsScene.h"
#include "JoltExtantions.h"
#include "Layers.h"
#include "Colliders.h"

using namespace JPH;


DEF_COMPONENT(Rigidbody, Engine.Rigidbody, 7, RunMode::PlayOnly) {
	OFFSET(0, Rigidbody, mass);
	OFFSET(1, Rigidbody, maxLinearVelocity);
	OFFSET(2, Rigidbody, linearDamping);
	OFFSET(3, Rigidbody, angularDamping);
	OFFSET(4, Rigidbody, friction);
	OFFSET(5, Rigidbody, bounciness);
	OFFSET(6, Rigidbody, mAllowSleeping);
}

void Rigidbody::OnInit() {
	m_collider = GetComponent<Collider>();
	if (m_collider == nullptr)
		throw std::exception("Rigidbody: collider == nullptr");
}

void Rigidbody::OnStart() {
	if (!active())
		return;

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	m_rigidbodyIter = physicsScene->rigidbodies.insert(physicsScene->rigidbodies.end(), this);

	auto shapeSettings = m_collider->settings;
	//m_center = m_collider->center;
	auto layer = Layers::GetLayerFromMotionType(motion());
	auto position = worldPosition();
	auto rotation = localRotationQ();

	m_bodySettings = BodyCreationSettings(shapeSettings, ToJolt(position), ToJolt(rotation), motion(), layer);
	m_bodySettings.mGravityFactor = m_gravity * physicsScene->cWorldScale;
	m_bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	m_bodySettings.mMassPropertiesOverride.mMass = mass;
	m_bodySettings.mIsSensor = m_collider->isTrigger;
	m_bodySettings.mLinearDamping = linearDamping;
	m_bodySettings.mAngularDamping = angularDamping;
	m_bodySettings.mMaxLinearVelocity = maxLinearVelocity;
	m_bodySettings.mFriction = friction;
	m_bodySettings.mRestitution = bounciness;
	m_bodySettings.mMotionQuality = quality();
	m_bodySettings.mAllowDynamicOrKinematic = true;
	m_bodySettings.mAllowSleeping = mAllowSleeping;

	m_body = bodyInterface->CreateBody(m_bodySettings);
	assert(m_body != nullptr);

	m_lastScale = localScale();
	m_body->GetShape()->ScaleShape(ToJolt(m_lastScale));
		
	bodyInterface->AddBody(m_body->GetID(), EActivation::Activate);

}

void Rigidbody::OnDestroy() {
	if (m_body == nullptr)
		return; 

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	bodyInterface->RemoveBody(m_body->GetID());
	bodyInterface->DestroyBody(m_body->GetID());
	m_body = nullptr;

	physicsScene->rigidbodies.erase(m_rigidbodyIter);
}

void Rigidbody::OnUpdate() {
	if (game()->ui()->GetActor() == actor()) {
		if (game()->hotkeys()->GetButtonDown(Keys::G)) {
			AddForce({ 0, 1000, 0 });
		}
	}
}

void Rigidbody::BeforePhysicsUpdate() {
	if (m_body == nullptr || m_body->IsStatic() || !active())
		return;

	auto position = worldPosition();
	auto rotation = localRotationQ();
	auto scale = localScale();

	if (m_lastScale != scale) {
		m_lastScale = scale;

		auto res = m_body->GetShape()->ScaleShape(ToJolt(localScale()));
	}
		
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	bodyInterface->SetPositionAndRotationWhenChanged(m_body->GetID(), ToJolt(position), ToJolt(rotation), EActivation::Activate);

}

void Rigidbody::OnFixedUpdate() {
	if (m_body == nullptr || m_body->IsStatic() || !active())
		return;

	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	//auto pos = m_body->GetCenterOfMassPosition(); 
	//auto tr = m_body->GetCenterOfMassTransform();

	//SetLocalMatrix((Matrix&)tr);
	 
	auto pos = m_body->GetPosition();
	auto rot = m_body->GetRotation();

	worldPosition(FromJolt(pos));
	localRotationQ(FromJolt(rot));
}

void Rigidbody::active(bool value) {
	//if (m_body != nullptr) {

		auto isActive = m_active;
		m_active = value;

		auto bodyInterface = scene()->physicsScene()->bodyInterface();

		if (value && !isActive)
			OnStart();
			//bodyInterface->AddBody(m_body->GetID(), EActivation::Activate);

		if (!value && isActive)
			OnDestroy();
		//{
		//	bodyInterface->RemoveBody(m_body->GetID());
		//	bodyInterface->DestroyBody(m_body->GetID());
		//	m_body = nullptr;
		//}
	//}
	

	//if (m_body == nullptr)
	//	return;

	//auto isActive = m_body->IsActive();
	//auto bodyInterface = scene()->physicsScene()->bodyInterface(); 

	//if (isActive && !m_active) 
	//	bodyInterface->DeactivateBody(m_body->GetID());

	//if (!isActive && m_active)
	//	bodyInterface->ActivateBody(m_body->GetID());
}

void Rigidbody::motion(EMotionType value) {
	m_motion = value;

	if (m_body == nullptr || !active())
		return;
	
	auto bodyInterface = scene()->physicsScene()->bodyInterface();
	auto currentMotion = bodyInterface->GetMotionType(m_body->GetID());

	if (currentMotion != m_motion)
		bodyInterface->SetMotionType(m_body->GetID(), m_motion, EActivation::Activate);
}

void Rigidbody::gravity(float value) {
	m_gravity = value;

	if (m_body == nullptr || m_body->IsStatic() || !active())
		return;

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	bodyInterface->SetGravityFactor(m_body->GetID(), m_gravity * physicsScene->cWorldScale);
}

void Rigidbody::AddForce(Vector3 inForce) {
	if (m_body == nullptr || !m_body->IsDynamic() || !active())
		return;

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->AddForce(ToJolt(inForce * physicsScene->cWorldScale));
}

void Rigidbody::AddForce(Vector3 inForce, Vector3 inPosition) {
	if (m_body == nullptr || !m_body->IsDynamic() || !active())
		return;

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->AddForce(ToJolt(inForce * physicsScene->cWorldScale), ToJolt(inPosition));
}

Vector3 Rigidbody::GetLinearVelocity() const {
	if (m_body == nullptr)
		return Vector3::Zero;

	return FromJolt(m_body->GetLinearVelocity());
}

void Rigidbody::SetLinearVelocityClamped(Vector3 inLinearVelocity) {
	if (m_body == nullptr || m_body->IsStatic() || !active())
		return;

	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->SetLinearVelocityClamped(ToJolt(inLinearVelocity));
}

//void Rigidbody::SetMass(float mass) { //TODO Хз можно ли так, есть еще метод SetShapeInternal, возможном нужно через него
//	if (m_body) {
//		auto body_shape = m_body->GetShape();
//		auto mass_prop = body_shape->GetMassProperties();
//		mass_prop.mMass = mass;
//	}
//}

void Rigidbody::AddImpulse(Vector3 inImpulse) {
	if (m_body == nullptr || !m_body->IsDynamic() || !active())
		return;

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->AddImpulse(ToJolt(inImpulse * physicsScene->cWorldScale));
}

void Rigidbody::AddImpulse(Vector3 inImpulse, Vector3 inPosition) {
	if (m_body == nullptr || !m_body->IsDynamic() || !active())
		return;

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->AddImpulse(ToJolt(inImpulse * physicsScene->cWorldScale), ToJolt(inPosition));
}
//
//bool  Rigidbody::IsKinematic() const {
//	if (m_body)
//		return m_body->IsKinematic();
//}
//bool  Rigidbody::IsDynamic() const {
//	if (m_body)
//		return m_body->IsDynamic();
//}
//bool Rigidbody::IsStatic() const {
//	if (m_body)
//		return m_body->IsStatic();
//}

float Rigidbody::GetFriction() const {
	if (m_body)
		return m_body->GetFriction();
}
void Rigidbody::SetFriction(float inFriction) {
	if (m_body == nullptr || !active())
		return;

	if (inFriction < 0)
		inFriction = 0;

	return m_body->SetFriction(inFriction);
}


DEF_PROP_GETSET(Rigidbody, bool, active);
DEF_PROP_GETSET(Rigidbody, float, gravity);

DEF_PROP_GETSET_E(Rigidbody, EMotionType, motion);
DEF_PROP_GETSET_E(Rigidbody, EMotionQuality, quality);

DEF_FUNC(Rigidbody, AddForce, void)(CppRef bodyRef, Vector3 inForce) {
	CppRefs::ThrowPointer<Rigidbody>(bodyRef)->AddForce(inForce);
}

DEF_FUNC(Rigidbody, AddForceInPos, void)(CppRef bodyRef, Vector3 inForce, Vector3 inPosition) {
	CppRefs::ThrowPointer<Rigidbody>(bodyRef)->AddForce(inForce, inPosition);
}

DEF_FUNC(Rigidbody, GetLinearVelocity, Vector3)(CppRef bodyRef) {
	return CppRefs::ThrowPointer<Rigidbody>(bodyRef)->GetLinearVelocity();
}

DEF_FUNC(Rigidbody, SetLinearVelocityClamped, void)(CppRef bodyRef, Vector3 inLinearVelocity) {
	CppRefs::ThrowPointer<Rigidbody>(bodyRef)->SetLinearVelocityClamped(inLinearVelocity);
}

DEF_FUNC(Rigidbody, AddImpulse, void)(CppRef bodyRef, Vector3 inImpulse) {
	CppRefs::ThrowPointer<Rigidbody>(bodyRef)->AddImpulse(inImpulse);
}

DEF_FUNC(Rigidbody, AddImpulseInPos, void)(CppRef bodyRef, Vector3 inImpulse, Vector3 inPosition) {
	CppRefs::ThrowPointer<Rigidbody>(bodyRef)->AddImpulse(inImpulse, inPosition);
}

DEF_FUNC(Rigidbody, GetFriction, float)(CppRef bodyRef) {
	return CppRefs::ThrowPointer<Rigidbody>(bodyRef)->GetFriction();
}

DEF_FUNC(Rigidbody, SetFriction, void)(CppRef bodyRef, float inFriction) {
	CppRefs::ThrowPointer<Rigidbody>(bodyRef)->SetFriction(inFriction);
}
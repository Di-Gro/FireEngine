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


DEF_COMPONENT(Rigidbody, Engine.Rigidbody, 8, RunMode::PlayOnly) {
	OFFSET(0, Rigidbody, mass);
	OFFSET(1, Rigidbody, maxLinearVelocity);
	OFFSET(2, Rigidbody, linearDamping);
	OFFSET(3, Rigidbody, angularDamping);
	OFFSET(4, Rigidbody, friction);
	OFFSET(5, Rigidbody, bounciness);
	OFFSET(6, Rigidbody, mAllowSleeping);
	OFFSET(7, Rigidbody, isSensor);
}

void Rigidbody::OnInit() {
	m_collider = GetComponent<Collider>();
	if (m_collider == nullptr)
		throw std::exception("Rigidbody: collider == nullptr");
}

void Rigidbody::OnActivate() {
	if (simulate())
		m_CreateBody();

	m_onPhysicsUpdate = false;
}

void Rigidbody::OnStart() {

}

void Rigidbody::OnDeactivate() {
	if (m_body != nullptr)
		m_RemoveBody();

	m_onPhysicsUpdate = false;
}

void Rigidbody::OnDestroy() {
	
}

void Rigidbody::m_CreateBody() {
	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	m_rigidbodyIter = physicsScene->rigidbodies.insert(physicsScene->rigidbodies.end(), this);

	m_collider->settings = m_collider->CreateShapeSettings();
	auto shapeSettings = m_collider->settings;
	//m_center = m_collider->center;
	auto layer = isSensor ? Layers::TRIGGER : Layers::GetLayerFromMotionType(motion());
	auto position = worldPosition();
	auto rotation = worldRotationQ();//localRotationQ();

	m_bodySettings = BodyCreationSettings(shapeSettings, ToJolt(position), ToJolt(rotation), motion(), layer);
	m_bodySettings.mGravityFactor = m_gravity * physicsScene->cWorldScale;
	m_bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	m_bodySettings.mMassPropertiesOverride.mMass = mass;
	//m_bodySettings.mIsSensor = m_collider->isTrigger;
	m_bodySettings.mLinearDamping = linearDamping;
	m_bodySettings.mAngularDamping = angularDamping;
	m_bodySettings.mMaxLinearVelocity = maxLinearVelocity;
	m_bodySettings.mFriction = friction;
	m_bodySettings.mRestitution = bounciness;
	m_bodySettings.mMotionQuality = quality();
	m_bodySettings.mAllowDynamicOrKinematic = true;
	m_bodySettings.mAllowSleeping = mAllowSleeping;
	m_bodySettings.mUserData = (uint64)actor();
	m_bodySettings.mIsSensor = isSensor;

	if (isSensor)
		actor()->bodyTag = BodyTag::Trigger;

	m_body = bodyInterface->CreateBody(m_bodySettings);
	assert(m_body != nullptr);

	bool tes = m_body->IsSensor();

	m_lastScale = localScale();
	m_body->GetShape()->ScaleShape(ToJolt(m_lastScale));

	bodyInterface->AddBody(m_body->GetID(), EActivation::Activate);
}

void Rigidbody::m_RemoveBody() {
	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	bodyInterface->RemoveBody(m_body->GetID());
	bodyInterface->DestroyBody(m_body->GetID());
	m_body = nullptr;

	physicsScene->rigidbodies.erase(m_rigidbodyIter);
}

void Rigidbody::BeforePhysicsUpdate() {
	if (m_body == nullptr || m_body->IsStatic() || !simulate())
		return;

	auto position = worldPosition();
	auto rotation = worldRotationQ();// localRotationQ();
	auto scale = localScale();

	if (m_lastScale != scale) {
		m_lastScale = scale;

		auto res = m_body->GetShape()->ScaleShape(ToJolt(localScale()));
	}

	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (m_body->IsKinematic()) 
		bodyInterface->MoveKinematic(m_body->GetID(), ToJolt(position), ToJolt(rotation), game()->deltaFixedTime());
	else 
		bodyInterface->SetPositionAndRotationWhenChanged(m_body->GetID(), ToJolt(position), ToJolt(rotation), EActivation::Activate);

	m_UpdateState();
	m_onPhysicsUpdate = true;
}

void Rigidbody::OnFixedUpdate() {
	if (m_body == nullptr || m_body->IsStatic() || !simulate())
		return;

	auto bodyInterface = scene()->physicsScene()->bodyInterface();
	 
	auto pos = m_body->GetPosition();
	auto rot = m_body->GetRotation();

	worldPosition(FromJolt(pos));
	worldRotationQ(FromJolt(rot));
	//localRotationQ(FromJolt(rot));

	m_onPhysicsUpdate = false;
}

void Rigidbody::m_UpdateState() {
	if (m_onPhysicsUpdate)
		return;

	if (m_updateSimulate) {
		m_updateSimulate = false;
		simulate(m_initSimulate);
	}
	if (m_updateMotion) {
		m_updateMotion = false;
		motion(m_initMotion);
	}
	if (m_updateGravity) {
		m_updateGravity = false;
		gravity(m_initGravity);
	}
	if (m_updateFriction) {
		m_updateFriction = false;
		SetFriction(m_initFriction);
	}
	if (m_updateForce) {
		m_updateForce = false;
		AddForce(m_initForce);
	}
	if (m_updateForceAtPoint) {
		m_updateForceAtPoint = false;
		AddForce(m_initForceAtPoint, m_initForcePosition);
	}
	if (m_updateImpulse) {
		m_updateImpulse = false;
		AddImpulse(m_initImpulse);
	}
	if (m_updateImpulseAtPoint) {
		m_updateImpulseAtPoint = false;
		AddImpulse(m_initImpulseAtPoint, m_initImpulsePosition);
	}
	if (m_updateVelsity) {
		m_updateVelsity = false;
		SetLinearVelocityClamped(m_initVelsity);
	}
}

void Rigidbody::simulate(bool value) {
	if (m_onPhysicsUpdate) {
		m_initSimulate = value;
		m_updateSimulate = true;;
		return;
	}

	m_simulate = value;

	if (m_body == nullptr && m_simulate && IsActivated())
		m_CreateBody();

	if (m_body != nullptr && (!m_simulate || !IsActivated()))
		m_RemoveBody();
}

void Rigidbody::motion(EMotionType value) {
	if (m_onPhysicsUpdate) {
		m_initMotion = value;
		m_updateMotion = true;
		return;
	}

	m_motion = value;

	if (m_body == nullptr || !simulate())
		return;
	
	auto bodyInterface = scene()->physicsScene()->bodyInterface();
	auto currentMotion = bodyInterface->GetMotionType(m_body->GetID());

	if (currentMotion != m_motion)
		bodyInterface->SetMotionType(m_body->GetID(), m_motion, EActivation::Activate);
}

void Rigidbody::gravity(float value) {
	if (m_onPhysicsUpdate) {
		m_initGravity = value;
		m_updateGravity = true;;
		return;
	}

	m_gravity = value;

	if (m_body == nullptr || m_body->IsStatic() || !simulate())
		return;

	if (m_onPhysicsUpdate) {
		m_updateGravity = true;
		return;
	}

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	bodyInterface->SetGravityFactor(m_body->GetID(), m_gravity * physicsScene->cWorldScale);
}

void Rigidbody::AddForce(Vector3 inForce) {

	if (m_body == nullptr || !m_body->IsDynamic() || !simulate() || m_onPhysicsUpdate) {
		m_initForce = inForce;
		m_updateForce = true;
		return;
	}

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->AddForce(ToJolt(inForce * physicsScene->cWorldScale));
}

void Rigidbody::AddForce(Vector3 inForce, Vector3 inPosition) {
	if (m_body == nullptr || !m_body->IsDynamic() || !simulate()) {
		m_initForceAtPoint = inForce;
		m_initForcePosition = inPosition;
		m_updateForceAtPoint = true;
		return;
	}

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->AddForce(ToJolt(inForce * physicsScene->cWorldScale), ToJolt(inPosition));
}

Vector3 Rigidbody::GetLinearVelocity() const {
	if (m_body == nullptr)
		return m_initVelsity;

	return FromJolt(m_body->GetLinearVelocity());
}

void Rigidbody::SetLinearVelocityClamped(Vector3 inLinearVelocity) {
	if (m_body == nullptr || m_body->IsStatic() || !simulate()) {
		m_initVelsity = inLinearVelocity;
		m_updateVelsity = true;
		return;
	}

	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->SetLinearVelocityClamped(ToJolt(inLinearVelocity));
}

void Rigidbody::AddImpulse(Vector3 inImpulse) {
	if (m_body == nullptr || !m_body->IsDynamic() || !simulate()) {
		m_initImpulse = inImpulse;
		m_updateImpulse = true;
		return;
	}

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->AddImpulse(ToJolt(inImpulse * physicsScene->cWorldScale));
}

void Rigidbody::AddImpulse(Vector3 inImpulse, Vector3 inPosition) {
	if (m_body == nullptr || !m_body->IsDynamic() || !simulate()) {
		m_initImpulseAtPoint = inImpulse;
		m_initImpulsePosition = inPosition;
		m_updateImpulseAtPoint = true;
		return;
	}

	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = scene()->physicsScene()->bodyInterface();

	if (!m_body->IsActive())
		bodyInterface->ActivateBody(m_body->GetID());

	m_body->AddImpulse(ToJolt(inImpulse * physicsScene->cWorldScale), ToJolt(inPosition));
}

float Rigidbody::GetFriction() const {
	if (m_body)
		return m_body->GetFriction();
	else
		return friction;
}

void Rigidbody::SetFriction(float inFriction) {
	if (m_onPhysicsUpdate) {
		m_initFriction = inFriction;
		m_updateFriction = true;
		return;
	}

	friction = inFriction;

	if (m_body == nullptr || !simulate())
		return;

	if (inFriction < 0)
		inFriction = 0;

	return m_body->SetFriction(inFriction);
}


DEF_PROP_GETSET(Rigidbody, bool, simulate);
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
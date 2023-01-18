#include "Character.h"

#include <Jolt\Physics\Body\BodyInterface.h>
#include <Jolt\Physics\Body\Body.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt\Geometry\Plane.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

#include "Game.h"
#include "UI\UserInterface.h"
#include "Scene.h"
#include "HotKeys.h"
#include "PhysicsScene.h"
#include "JoltExtantions.h"
#include "Layers.h"
#include "Colliders.h"

using namespace JPH;


DEF_COMPONENT(FireCharacter, Engine.Character, 3, RunMode::PlayOnly) {
	OFFSET(0, FireCharacter, maxSlopeAngle);
	OFFSET(1, FireCharacter, friction);
	OFFSET(2, FireCharacter, mass);
}

void FireCharacter::OnInit() {
	m_collider = GetComponent<CapsuleCollider>();
	if (m_collider == nullptr)
		throw std::exception("Character: collider == nullptr");
}

void FireCharacter::OnActivate() {
	if (simulate())
		m_CreateBody();
}

void FireCharacter::OnStart() {

}

void FireCharacter::OnDeactivate() {
	if (m_�haracter != nullptr)
		m_RemoveBody();
}

void FireCharacter::OnDestroy() {
	
}

void FireCharacter::m_CreateBody() {

	auto physicsScene = scene()->physicsScene();
	auto physicsSystem = physicsScene->physicsSystem();
	auto bodyInterface = physicsScene->bodyInterface();

	m_rigidbodyIter = physicsScene->rigidbodies.insert(physicsScene->rigidbodies.end(), this);

	auto shapeSettings = m_collider->settings;
	auto position = worldPosition();
	auto rotation = worldRotationQ();

	float cCharacterRadiusStanding = m_collider->scaledRadius();

	m_�haracterSettings = new CharacterSettings();
	m_�haracterSettings->mMaxSlopeAngle = DegreesToRadians(maxSlopeAngle);
	m_�haracterSettings->mLayer = Layers::MOVING;
	m_�haracterSettings->mShape = shapeSettings;
	m_�haracterSettings->mFriction = friction;
	m_�haracterSettings->mMass = mass;
	m_�haracterSettings->mGravityFactor = m_gravity * physicsScene->cWorldScale;
	m_�haracterSettings->mSupportingVolume = JPH::Plane(Vec3::sAxisY(), -cCharacterRadiusStanding);

	m_�haracter = new Character(m_�haracterSettings, ToJolt(position), ToJolt(rotation), (uint64)actor(), physicsSystem);

	m_�haracter->AddToPhysicsSystem(EActivation::Activate);

	m_�haracter->GetBodyID();
}

void FireCharacter::m_RemoveBody() {
	auto physicsScene = scene()->physicsScene();
	auto bodyInterface = physicsScene->bodyInterface();

	m_�haracter->RemoveFromPhysicsSystem();
	delete m_�haracter;
	m_�haracter = nullptr;

	physicsScene->rigidbodies.erase(m_rigidbodyIter);
}

void FireCharacter::BeforePhysicsUpdate() {
	if (m_�haracter == nullptr)
		return;

	auto position = worldPosition();
	auto rotation = localRotationQ();

	m_�haracter->SetPositionAndRotation(ToJolt(position), ToJolt(rotation));
}

void FireCharacter::OnFixedUpdate() {
	if (m_�haracter == nullptr || !simulate())
		return;
	 
	auto pos = m_�haracter->GetPosition();
	auto rot = m_�haracter->GetRotation();

	worldPosition(FromJolt(pos));
	localRotationQ(FromJolt(rot));
}

void FireCharacter::simulate(bool value) {
	m_simulate = value;

	if (m_�haracter == nullptr && m_simulate && IsActivated())
		m_CreateBody();

	if (m_�haracter != nullptr && (!m_simulate || !IsActivated()))
		m_RemoveBody();
}

void FireCharacter::gravity(float value) {
	m_gravity = value;

	if (m_�haracter == nullptr)
		return;

	auto physicsScene = scene()->physicsScene();

	m_�haracterSettings->mGravityFactor = m_gravity * physicsScene->cWorldScale;
}

Vector3 FireCharacter::GetLinearVelocity() const {
	if (m_�haracter == nullptr)
		return Vector3::Zero;

	return FromJolt(m_�haracter->GetLinearVelocity());
}

void FireCharacter::SetLinearVelocityClamped(Vector3 inLinearVelocity) {
	if (m_�haracter == nullptr)
		return;

	m_�haracter->SetLinearVelocity(ToJolt(inLinearVelocity));
}

void FireCharacter::AddImpulse(Vector3 inImpulse) {
	if (m_�haracter == nullptr)
		return;

	auto physicsScene = scene()->physicsScene();

	m_�haracter->AddImpulse(ToJolt(inImpulse));
}

float FireCharacter::GetFriction() const {
	if (m_�haracter == nullptr)
		return 0;

	return m_�haracterSettings->mFriction;
}

void FireCharacter::SetFriction(float inFriction) {
	if (m_�haracter == nullptr)
		return;

	if (inFriction < 0)
		inFriction = 0;

	m_�haracterSettings->mFriction = inFriction;
}

#define DONE


DEF_PROP_GETSET(FireCharacter, bool, simulate);
DEF_PROP_GETSET(FireCharacter, float, gravity);


DEF_FUNC(FireCharacter, GetLinearVelocity, Vector3)(CppRef bodyRef) {
	return CppRefs::ThrowPointer<FireCharacter>(bodyRef)->GetLinearVelocity();
}

DEF_FUNC(FireCharacter, SetLinearVelocityClamped, void)(CppRef bodyRef, Vector3 inLinearVelocity) {
	CppRefs::ThrowPointer<FireCharacter>(bodyRef)->SetLinearVelocityClamped(inLinearVelocity);
}

DEF_FUNC(FireCharacter, AddImpulse, void)(CppRef bodyRef, Vector3 inImpulse) {
	CppRefs::ThrowPointer<FireCharacter>(bodyRef)->AddImpulse(inImpulse);
}

DEF_FUNC(FireCharacter, GetFriction, float)(CppRef bodyRef) {
	return CppRefs::ThrowPointer<FireCharacter>(bodyRef)->GetFriction();
}

DEF_FUNC(FireCharacter, SetFriction, void)(CppRef bodyRef, float inFriction) {
	CppRefs::ThrowPointer<FireCharacter>(bodyRef)->SetFriction(inFriction);
}
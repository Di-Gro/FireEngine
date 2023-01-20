#include "Character.h"

#include <Jolt\Physics\Body\BodyInterface.h>
#include <Jolt\Physics\Body\Body.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt\Geometry\Plane.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt\Physics\Collision\CollideShape.h>

#include "Game.h"
#include "UI\UserInterface.h"
#include "Scene.h"
#include "HotKeys.h"
#include "PhysicsScene.h"
#include "JoltExtantions.h"
#include "Layers.h"
#include "Colliders.h"

using namespace JPH;


DEF_COMPONENT(FireCharacter, Engine.Character, 7, RunMode::PlayOnly) {
	OFFSET(0, FireCharacter, maxSlopeAngle);
	OFFSET(1, FireCharacter, friction);
	OFFSET(2, FireCharacter, mass);
	OFFSET(3, FireCharacter, walkSpeed);
	OFFSET(4, FireCharacter, jumpSpeed);
	OFFSET(5, FireCharacter, velosity);
	OFFSET(6, FireCharacter, runSpeed);
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

	float cCollisionTolerance = 0.05f;
	m_�haracter->PostSimulation(cCollisionTolerance);
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

class ClosestPointShapeCollector : public CollideShapeCollector {
public:
	Vec3 point;
	Vec3 closestPoint;
	float dist = 1000000;
	Vec3 penetrationAxis;
	float penetrationDepth;

	bool hasHit = false;

public:
	ClosestPointShapeCollector(Vec3 position) : point(position) { }

	virtual void AddHit(const CollideShapeResult& inResult) override
	{
		auto hitDist = (point - inResult.mContactPointOn2).LengthSq();
		if (hitDist < dist) {
			dist = hitDist;
			closestPoint = inResult.mContactPointOn2;
			penetrationAxis = inResult.mPenetrationAxis;
			penetrationDepth = inResult.mPenetrationDepth;
		}

		hasHit = true;
	}
};

void FireCharacter::HandleInput(Vector3 movementDirection, bool jump, bool run, float deltaTime)
{
	if (m_�haracter == nullptr)
		return;

	using GroundState = Character::EGroundState;

	auto direction = ToJolt(movementDirection.Normalized());
	auto speed = run ? runSpeed : walkSpeed;

	// Cancel movement in opposite direction of normal when touching something we can't walk up
	auto ground_state = m_�haracter->GetGroundState();
	if (ground_state == GroundState::OnSteepGround
		|| ground_state == GroundState::NotSupported)
	{
		auto actorPtr = (Actor*)m_�haracter->GetGroundUserData();
		assert(actorPtr != nullptr);

		if (actorPtr->bodyTag != BodyTag::Trigger) {
			auto normal = m_�haracter->GetGroundNormal();
			normal.SetY(0.0f);
			float dot = normal.Dot(direction);
			if (dot < 0.0f)
				direction -= (dot * normal) / normal.LengthSq();
		}
	}

	// Update velocity
	auto current_velocity = m_�haracter->GetLinearVelocity();
	auto desired_velocity = speed * direction;
	desired_velocity.SetY(current_velocity.GetY());
	auto new_velocity = 0.75f * current_velocity + 0.25f * desired_velocity;

	// Jump
	if (jump)
		int j = 0;

	if (jump && ground_state == Character::EGroundState::OnGround)
		new_velocity += {0, jumpSpeed, 0};

	auto vel = FromJolt(new_velocity);
	FixNaN(vel);
	new_velocity = ToJolt(vel);

	// Update the velocity
	m_�haracter->SetLinearVelocity(new_velocity);
}

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

DEF_FUNC(FireCharacter, HandleInput, void)(CppRef bodyRef, Vector3 movementDirection, bool jump, bool run, float deltaTime) {
	CppRefs::ThrowPointer<FireCharacter>(bodyRef)->HandleInput(movementDirection, jump, run, deltaTime);
}


//auto current_position = FromJolt(m_�haracter->GetPosition());
//auto next_position = current_position + (new_velocity * deltaTime);
//auto movement_direction = new_velocity.Normalized();

//ClosestPointShapeCollector collector(m_�haracter->GetPosition());

//m_�haracter->CheckCollision(
//	ToJolt(next_position),
//	m_�haracter->GetRotation(),
//	ToJolt(movement_direction),
//	0,
//	m_�haracter->GetShape(),
//	{ 0, 0, 0 },
//	collector,
//	false
//);

//if (collector.hasHit) {
//	//auto radius = m_collider->scaledRadius();
//	auto normal = ToJolt(-movement_direction);
//	auto separationVector = normal * collector.penetrationDepth /*+ normal * radius*/;
//	//auto avoidencePosition = collector.closestPoint + separationVector;

//	auto avoidenceDeltaPosition = (ToJolt(next_position) + separationVector) - m_�haracter->GetPosition();

//	auto avoidenceVelocity = avoidenceDeltaPosition / deltaTime;

//	//new_velocity = FromJolt(avoidenceVelocity);
//}
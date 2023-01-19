#pragma once

#include <list>
#include "Actor.h"
#include "PhysicsScene.h"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Character/Character.h>

class CapsuleCollider;
namespace JPH { class Body;}

class FireCharacter : public Component, public IFixedUpdateListener {
	COMPONENT(FireCharacter);

public:
	float maxSlopeAngle = 45.0f;
	float friction = 0.2f;
	float mass = 1.0f;
	float walkSpeed = 100;
	float runSpeed = 200;
	float jumpSpeed = 100;
	Vector3 velosity;
	Vector3 forward;

private:
	bool m_simulate = true;
	float m_gravity = 1.0f;

	CapsuleCollider* m_collider;
	
	JPH::Character* m_ñharacter = nullptr;
	JPH::Ref<JPH::CharacterSettings> m_ñharacterSettings;

	std::list<IFixedUpdateListener*>::iterator m_rigidbodyIter;

public:
	bool simulate() const { return m_simulate; };
	void simulate(bool value);

	float gravity() const { return m_gravity; };
	void gravity(float value);

	void OnInit() override;
	void OnStart() override;
	void OnDestroy() override;
	void OnFixedUpdate() override;
	void BeforePhysicsUpdate() override;

	void OnActivate() override;
	void OnDeactivate() override;

	Vector3 GetLinearVelocity() const;
	void SetLinearVelocityClamped(Vector3 inLinearVelocity);

	void AddImpulse(Vector3 inImpulse);
	void AddImpulse(Vector3 inImpulse, Vector3 inPosition);

	float GetFriction() const;
	void SetFriction(float inFriction);

	void HandleInput(Vector3 movementDirection, bool jump, bool run, float deltaTime);

private:
	void m_CreateBody();
	void m_RemoveBody();

};

DEC_COMPONENT(FireCharacter);

PROP_GETSET(FireCharacter, bool, simulate);
PROP_GETSET(FireCharacter, float, gravity);

FUNC(FireCharacter, GetLinearVelocity, Vector3)(CppRef bodyRef);
FUNC(FireCharacter, SetLinearVelocityClamped, void)(CppRef bodyRef, Vector3 inLinearVelocity);

FUNC(FireCharacter, AddImpulse, void)(CppRef bodyRef, Vector3 inImpulse);

FUNC(FireCharacter, GetFriction, float)(CppRef bodyRef);
FUNC(FireCharacter, SetFriction, void)(CppRef bodyRef, float inFriction);

FUNC(FireCharacter, HandleInput, void)(CppRef bodyRef, Vector3 movementDirection, bool jump, bool run, float deltaTime);
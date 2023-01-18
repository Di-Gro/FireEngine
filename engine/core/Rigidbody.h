#pragma once

#include <list>
#include "Actor.h"
#include "PhysicsScene.h"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MotionType.h>

class Collider;
namespace JPH { class Body;}

class Rigidbody : public Component, public IFixedUpdateListener {
	COMPONENT(Rigidbody);

public:
	float maxLinearVelocity = 500.0f;
	float linearDamping = 0.05f;
	float angularDamping = 0.05f;
	float friction = 0.2f;
	float mass = 1.0f;
	float bounciness = 0.2f;
	bool mAllowSleeping = true;
	bool isSensor = false;

private:
	bool m_simulate = true;
	JPH::EMotionType m_motion;
	//float m_mass = 1.0f;
	float m_gravity = 1.0f;

	JPH::EMotionQuality m_quality = JPH::EMotionQuality::Discrete;

	JPH::Body* m_body;
	Collider* m_collider;
	
	JPH::BodyCreationSettings m_bodySettings;

	std::list<IFixedUpdateListener*>::iterator m_rigidbodyIter;

	Vector3 m_lastScale;
	//Vector3 m_center;

public:
	bool simulate() const { return m_simulate; };
	void simulate(bool value);

	JPH::EMotionType motion() { return m_motion; };
	void motion(JPH::EMotionType value);

	float gravity() const { return m_gravity; };
	void gravity(float value);

	JPH::EMotionQuality quality() const { return m_quality; };
	void quality(JPH::EMotionQuality value) { m_quality = value; };

	void OnInit() override;
	void OnStart() override;
	void OnDestroy() override;
	void OnFixedUpdate() override;
	void BeforePhysicsUpdate() override;

	void OnActivate() override;
	void OnDeactivate() override;

	//void OnCollisionEnter(Actor* otherActor) override;
	//void OnCollisionExit(Actor* otherActor) override;

	//void OnTriggerEnter(Actor* otherActor) override;
	//void OnTriggerExit(Actor* otherActor) override;

	void AddForce(Vector3 inForce);
	void AddForce(Vector3 inForce, Vector3 inPosition);
	Vector3 GetLinearVelocity() const;
	void SetLinearVelocityClamped(Vector3 inLinearVelocity);
	//void SetMass(float mass);
	void AddImpulse(Vector3 inImpulse);
	void AddImpulse(Vector3 inImpulse, Vector3 inPosition);
	//bool IsKinematic() const;
	//bool IsDynamic() const;
	//bool IsStatic() const;
	float GetFriction() const;
	void SetFriction(float inFriction);

private:
	void m_CreateBody();
	void m_RemoveBody();
};
DEC_COMPONENT(Rigidbody);

PROP_GETSET(Rigidbody, bool, simulate);
PROP_GETSET(Rigidbody, float, gravity);
PROP_GETSET_E(Rigidbody, JPH::EMotionType, motion);
PROP_GETSET_E(Rigidbody, JPH::EMotionQuality, quality);

FUNC(Rigidbody, AddForce, void)(CppRef bodyRef, Vector3 inForce);
FUNC(Rigidbody, AddForceInPos, void)(CppRef bodyRef, Vector3 inForce, Vector3 inPosition);

FUNC(Rigidbody, GetLinearVelocity, Vector3)(CppRef bodyRef);
FUNC(Rigidbody, SetLinearVelocityClamped, void)(CppRef bodyRef, Vector3 inLinearVelocity);

FUNC(Rigidbody, AddImpulse, void)(CppRef bodyRef, Vector3 inImpulse);
FUNC(Rigidbody, AddImpulseInPos, void)(CppRef bodyRef, Vector3 inImpulse, Vector3 inPosition);

FUNC(Rigidbody, GetFriction, float)(CppRef bodyRef);
FUNC(Rigidbody, SetFriction, void)(CppRef bodyRef, float inFriction);
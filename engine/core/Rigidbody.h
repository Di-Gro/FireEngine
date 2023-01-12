#pragma once

#include <list>
#include "Actor.h"

#include <Jolt/Jolt.h>

//#include <Jolt/Physics/Body/MotionType.h>
class Collider;
namespace JPH { class Body;}

class Rigidbody : public Component {
	COMPONENT(Rigidbody);

private:
	Collider* m_collider;
	JPH::Body* m_body;
	std::list<Rigidbody*>::iterator m_rigidbodyIter;

public:
	void OnInit() override;
	void OnStart() override;
	void OnDestroy() override;
	void OnFixedUpdate() override;
	void AddForce(Vector3 inForce);
	void AddForce(Vector3 inForce, Vector3 inPosition);
	Vector3 GetLinearVelocity() const;
	void SetLinearVelocityClamped(Vector3 inLinearVelocity);
	void SetMass(float mass);
	void AddImpulse(Vector3 inImpulse);
	void AddImpulse(Vector3 inImpulse, Vector3 inPosition);
	bool IsKinematic() const;
	bool IsDynamic() const;
	bool IsStatic() const;
	void OnBeginPhysicsUpdate();
	float GetFriction() const;
	void SetFriction(float inFriction);
};
DEC_COMPONENT(Rigidbody);

#pragma once

#include <Jolt/Jolt.h>
#include "Jolt\Physics\Collision\ContactListener.h"

namespace JPH {
	class PhysicsSystem;
	class BodyLockInterfaceNoLock;
}

class FireContactListener : public JPH::ContactListener {

private:
	JPH::PhysicsSystem* m_physicsSystem;
	const JPH::BodyLockInterfaceNoLock* m_bodyLockInterfaceNoLock;

public:
	void Init(JPH::PhysicsSystem* physicsSystem);

	//virtual ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override;

	virtual void			OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
	virtual void			OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

	virtual void			OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;
};
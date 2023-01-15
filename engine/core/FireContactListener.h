#pragma once

#include <Jolt/Jolt.h>
#include "Jolt\Physics\Collision\ContactListener.h"

namespace {
	using namespace JPH;
}

class FireContactListener : public ContactListener
{
public:

	//virtual ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override;

	virtual void			OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override;
	virtual void			OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override;

	virtual void			OnContactRemoved(const SubShapeIDPair& inSubShapePair) override;
};
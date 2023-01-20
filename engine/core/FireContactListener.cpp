#include "FireContactListener.h"

#include <iostream>

#include <Jolt\Physics\Body\Body.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "Rigidbody.h"
#include "Actor.h"
#include "JoltExtantions.h"
#include "Contact.h"

using namespace JPH;

//
//// See: ContactListener
//ValidateResult	FireContactListener::OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult)
//{
//	std::cout << "Contact validate callback" << std::endl;
//
//	// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
//	return ValidateResult::AcceptAllContactsForThisBodyPair;
//}

void FireContactListener::Init(PhysicsSystem* physicsSystem) {
	m_physicsSystem = physicsSystem; 
	m_bodyLockInterfaceNoLock = &m_physicsSystem->GetBodyLockInterfaceNoLock();
}

void			FireContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	//std::cout << "OnContactAdded:\n";

	auto actor1 = (Actor*)inBody1.GetUserData();
	auto actor2 = (Actor*)inBody2.GetUserData();

	bool isSensor1 = inBody1.IsSensor();
	bool isSensor2 = inBody2.IsSensor();

	Contact contact1;
	contact1.point1 = FromJolt(inManifold.GetWorldSpaceContactPointOn1(0));
	contact1.point2 = FromJolt(inManifold.GetWorldSpaceContactPointOn2(0));
	contact1.relativePoint1 = FromJolt(inManifold.mRelativeContactPointsOn1[0]);
	contact1.relativePoint2 = FromJolt(inManifold.mRelativeContactPointsOn2[0]);
	contact1.normal = FromJolt(inManifold.mWorldSpaceNormal);
	contact1.offset = FromJolt(inManifold.mBaseOffset);
	contact1.depth = inManifold.mPenetrationDepth;

	Contact contact2;
	contact2.point1 = FromJolt(inManifold.GetWorldSpaceContactPointOn2(0));
	contact2.point2 = FromJolt(inManifold.GetWorldSpaceContactPointOn1(0));
	contact2.relativePoint1 = FromJolt(inManifold.mRelativeContactPointsOn2[0]);
	contact2.relativePoint2 = FromJolt(inManifold.mRelativeContactPointsOn1[0]);
	contact2.normal = FromJolt(-inManifold.mWorldSpaceNormal);
	contact2.offset = FromJolt(inManifold.mBaseOffset);
	contact2.depth = inManifold.mPenetrationDepth;

	if (isSensor1 || isSensor2) {
		actor1->f_EnterTrigger(actor2, contact1);
		actor2->f_EnterTrigger(actor1, contact2);
		return;
	}
	actor1->f_EnterCollision(actor2, contact1);
	actor2->f_EnterCollision(actor1, contact2);

}

void			FireContactListener::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	//auto body1 = (Rigidbody*)inBody1.GetUserData();
	//auto body2 = (Rigidbody*)inBody2.GetUserData();

	//std::cout << "Contact stay: " << body1->actor()->name() << " -> " << body2->actor()->name() << std::endl;
}

void			FireContactListener::OnContactRemoved(const SubShapeIDPair& inSubShapePair)
{
	//std::cout << "OnContactRemoved:\n";

	Actor* actor1 = nullptr;
	Actor* actor2 = nullptr;

	bool isSensor1 = false;
	bool isSensor2 = false;

	{
		JPH::BodyLockRead lock(*m_bodyLockInterfaceNoLock, inSubShapePair.GetBody1ID());
		if (lock.Succeeded()) {
			auto& body = lock.GetBody();

			actor1 = (Actor*)body.GetUserData();
			isSensor1 = body.IsSensor();
		}
	}
	{
		JPH::BodyLockRead lock(*m_bodyLockInterfaceNoLock, inSubShapePair.GetBody2ID());
		if (lock.Succeeded()) {
			auto& body = lock.GetBody();

			actor2 = (Actor*)body.GetUserData();
			isSensor2 = body.IsSensor();
		}
	}

	if (actor1 == nullptr || actor2 == nullptr)
		return;

	if (isSensor1 || isSensor2) {
		actor1->f_ExitTrigger(actor2);
		actor2->f_ExitTrigger(actor1);
		return;
	}
	actor1->f_ExitCollision(actor2);
	actor2->f_ExitCollision(actor1);
}
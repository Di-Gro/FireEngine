#include "FireContactListener.h"

#include <iostream>
//
//// See: ContactListener
//ValidateResult	FireContactListener::OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult)
//{
//	std::cout << "Contact validate callback" << std::endl;
//
//	// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
//	return ValidateResult::AcceptAllContactsForThisBodyPair;
//}

void			FireContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	std::cout << "A contact was added" << std::endl;
}

void			FireContactListener::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	std::cout << "A contact was persisted" << std::endl;
}

void			FireContactListener::OnContactRemoved(const SubShapeIDPair& inSubShapePair)
{
	std::cout << "A contact was removed" << std::endl;
}
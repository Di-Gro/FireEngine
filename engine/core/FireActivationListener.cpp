#include "FireActivationListener.h"

#include <iostream>



void FireActivationListener::OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
{
	//std::cout << "A body got activated" << std::endl;
}

void FireActivationListener::OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
{
	//std::cout << "A body went to sleep" << std::endl;
}
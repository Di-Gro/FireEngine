#pragma once

#include "Actor.h"

//#include <Jolt/Jolt.h>
//#include <Jolt/Physics/Body/MotionType.h>


class Rigidbody : public Component {
	COMPONENT(Rigidbody);
public:
	//JPH::EMotionType motionType = JPH::EMotionType::Static;

public:
	void OnInit() override;
	void OnDestroy() override;
	void OnFixedUpdate() override;

};
DEC_COMPONENT(Rigidbody);

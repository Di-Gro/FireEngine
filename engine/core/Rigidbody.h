#pragma once

#include "Actor.h"

#include <Jolt/Jolt.h>

//#include <Jolt/Physics/Body/MotionType.h>
class Collider;
namespace JPH { class Body; }

class Rigidbody : public Component {
	COMPONENT(Rigidbody);

private:
	Collider* m_collider;
	JPH::Body* m_body;

public:
	void OnInit() override;
	void OnStart() override;
	void OnDestroy() override;
	void OnFixedUpdate() override;

};
DEC_COMPONENT(Rigidbody);

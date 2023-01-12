#pragma once

#include <list>
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
	std::list<Rigidbody*>::iterator m_rigidbodyIter;

public:
	void OnInit() override;
	void OnStart() override;
	void OnDestroy() override;
	void OnFixedUpdate() override;

	void OnBeginPhysicsUpdate();

};
DEC_COMPONENT(Rigidbody);

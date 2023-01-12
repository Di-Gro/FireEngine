#pragma once

#include "Actor.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

class Collider : public Component {
	PURE_COMPONENT(Collider);

public:
	JPH::BodyCreationSettings settings;

};

class BoxCollider : public Collider {
	COMPONENT(BoxCollider);

public:
	Vector3 halfExtent = {50, 50, 50};

public:
	void OnInit() override;

};
DEC_COMPONENT(BoxCollider);



class SphereCollider : public Collider {
	COMPONENT(SphereCollider);

public:
	void OnInit() override;
	//void OnStart() override;
	//void OnUpdate() override;
	//void OnFixedUpdate() override;

};
DEC_COMPONENT(SphereCollider);



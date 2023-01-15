#pragma once

#include "Actor.h"

#include <Jolt/Jolt.h>
#include <Jolt\Physics\Collision\Shape\Shape.h>

#include "Forms.h"

class MeshComponent;
class Mesh4;

class Collider : public Component {
public:
	//bool isTrigger = false;
	//PhysicsMaterial material = nullptr;

	bool isTrigger = false;
	//Vector3 center = { 0, 0, 0 };	

	//float mFriction = 0.2f;
	//float mRestitution = 0.0f;

	JPH::ShapeRefC settings;

public:
	bool drawDebug = false;

protected:
	MeshComponent* debugMesh = nullptr;

public:
	// Do not override in children
	void OnInit() override; 

	// Can be override in children
	void OnUpdate() override;

	virtual JPH::ShapeRefC CreateShapeSettings() = 0;
	virtual const Mesh4* CreateDebugMesh() = 0;
};

class BoxCollider : public Collider {
	COMPONENT(BoxCollider);

public:
	Vector3 size = {1, 1, 1};

public:
	JPH::ShapeRefC CreateShapeSettings() override;
	const Mesh4* CreateDebugMesh() override;

	void OnUpdate() override;

};
DEC_COMPONENT(BoxCollider);

class SphereCollider : public Collider {
	COMPONENT(SphereCollider);

public:
	float radius = 0.5f;

public:
	JPH::ShapeRefC CreateShapeSettings() override;
	const Mesh4* CreateDebugMesh() override;

	void OnUpdate() override;

};
DEC_COMPONENT(SphereCollider);


class CapsuleCollider : public Collider {
	COMPONENT(CapsuleCollider);
private:
	static Forms4::Form HalfSphereForm;

public:
	float radius = 0.5f;
	float height = 1.0f;

private:
	MeshComponent* debugMesh2 = nullptr;

public:
	JPH::ShapeRefC CreateShapeSettings() override;
	const Mesh4* CreateDebugMesh() override;

	void OnUpdate() override;

};
DEC_COMPONENT(CapsuleCollider);




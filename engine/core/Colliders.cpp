#include "Colliders.h"

#include <Jolt\Physics\Collision\Shape\BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt\Physics\Collision\PhysicsMaterial.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>


#include "Layers.h"

#include "Game.h"
#include "MeshAsset.h"
#include "MeshComponent.h"
#include "JoltExtantions.h"
#include "Mesh.h"


using namespace JPH;
using namespace JPH::literals;

#pragma region Collider

void Collider::OnInit() {
	settings = CreateShapeSettings();
}

void Collider::OnUpdate() {
	if (drawDebug && debugMesh == nullptr) {
		debugMesh = AddComponent<MeshComponent>(true);
		debugMesh->mesh(CreateDebugMesh());
	}
	if (!drawDebug && debugMesh != nullptr) {
		debugMesh->Destroy();
		debugMesh = nullptr;
		return;
	}
}

#pragma endregion

#pragma region BoxCollider

DEF_COMPONENT(BoxCollider, Engine.BoxCollider, 2, RunMode::EditPlay) {
	//OFFSET(0, BoxCollider, center);
	OFFSET(0, BoxCollider, size);
	OFFSET(1, BoxCollider, drawDebug);
}

JPH::ShapeRefC BoxCollider::CreateShapeSettings() {
	// Next we can create a rigid body to serve as the floor, we make a large box
	// Create the settings for the collision volume (the shape). 
	// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
	auto lscale = localScale();
	auto wscale = worldScale();
	auto extent = wscale * size;
	float convRadius = VecMin(extent) * 0.1f;

	BoxShapeSettings shapeSettings(ToJolt(extent / 2), convRadius);

	// Create the shape
	ShapeSettings::ShapeResult result = shapeSettings.Create();
	assert(!result.HasError());

	auto position = worldPosition();
	auto rotation = worldRotationQ();
	auto scale = localScale();

	auto shape = result.Get();

	//RotatedTranslatedShapeSettings rotShape(ToJolt(position), ToJolt(rotation), shape);
	//result = rotShape.Create();
	//assert(!result.HasError());

	shape = result.Get();

	//ScaledShapeSettings scaleShape(shape, ToJolt(scale));
	//result = scaleShape.Create();
	//assert(!result.HasError());

	return shape;
}

const Mesh4* BoxCollider::CreateDebugMesh() {
	return game()->meshAsset()->GetMesh(MeshAsset::formBoxLined);
}

void BoxCollider::OnUpdate() {
	Collider::OnUpdate();

	if (drawDebug && debugMesh != nullptr) {
		debugMesh->meshScale = size;
	}
}

#pragma endregion

#pragma region SphereCollider
	
DEF_COMPONENT(SphereCollider, Engine.SphereCollider, 2, RunMode::EditPlay) {
	//OFFSET(0, SphereCollider, center);
	OFFSET(0, SphereCollider, radius);
	OFFSET(1, SphereCollider, drawDebug);
}

JPH::ShapeRefC SphereCollider::CreateShapeSettings() {

	auto scale = localScale();
	auto minScale = VecMin(scale);
	auto scaledRadius = minScale * radius;

	SphereShapeSettings shapeSettings(scaledRadius);

	ShapeSettings::ShapeResult result = shapeSettings.Create();
	assert(!result.HasError());

	return result.Get();
}

const Mesh4* SphereCollider::CreateDebugMesh() {
	return game()->meshAsset()->GetMesh(MeshAsset::formSphereLined);
}

void SphereCollider::OnUpdate() {
	Collider::OnUpdate();

	//if (drawDebug && debugMesh != nullptr) {
	//	debugMesh->meshScale = Vector3::One * radius * 2;
	//	debugMesh->meshOffset = center;
	//}
}

#pragma endregion

#pragma region CapsuleCollider

Forms4::Form CapsuleCollider::HalfSphereForm = Forms4::HalfSphereLined(0.5f, 24, 24);;

DEF_COMPONENT(CapsuleCollider, Engine.CapsuleCollider, 3, RunMode::EditPlay) {
	//OFFSET(0, CapsuleCollider, center);
	OFFSET(0, CapsuleCollider, radius);
	OFFSET(1, CapsuleCollider, height);
	OFFSET(2, CapsuleCollider, drawDebug);
}

JPH::ShapeRefC CapsuleCollider::CreateShapeSettings() {

	auto scale = localScale();
	auto minScale = VecMin(scale);
	auto scaledRadius = minScale * radius;
	auto scaledHeight = scale.y * height;

	CapsuleShapeSettings shapeSettings(scaledHeight / 2, scaledRadius);

	ShapeSettings::ShapeResult result = shapeSettings.Create();
	assert(!result.HasError());

	return result.Get();
}

const Mesh4* CapsuleCollider::CreateDebugMesh() {
	return nullptr;
}

void CapsuleCollider::OnUpdate() {
	Collider::OnUpdate();

	if (drawDebug && debugMesh2 == nullptr) {
		debugMesh2 = AddComponent<MeshComponent>(true);
		
		debugMesh->AddShape(&HalfSphereForm.verteces, &HalfSphereForm.indexes);
		debugMesh2->AddShape(&HalfSphereForm.verteces, &HalfSphereForm.indexes);

		debugMesh->mesh()->topology = HalfSphereForm.topology;
		debugMesh2->mesh()->topology = HalfSphereForm.topology;
	}
	if (!drawDebug && debugMesh2 != nullptr) {
		debugMesh2->Destroy();
		debugMesh2 = nullptr;
		return;
	}
	//if (drawDebug && debugMesh != nullptr) {
	//	debugMesh->meshScale = Vector3::One * radius * 2;
		//debugMesh->meshOffset = center + Vector3(0, height / 2, 0);

	//	debugMesh2->meshScale = Vector3::One * radius * 2 * Vector3(1, -1, 1);
	//	debugMesh2->meshOffset = center - Vector3(0, height / 2, 0);
	//}
}

#pragma endregion


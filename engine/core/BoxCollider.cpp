#include "BoxCollider.h"

#include <Jolt\Physics\Collision\Shape\BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>


#include "Layers.h"

#include "Game.h"
#include "MeshAsset.h"
#include "MeshComponent.h"

using namespace JPH;
using namespace JPH::literals;

DEF_PURE_COMPONENT(Collider, RunMode::EditPlay);

DEF_COMPONENT(BoxCollider, Engine.BoxCollider, 0, RunMode::EditPlay) {

}

void BoxCollider::OnInit() {
	float tmp_scale = 20;

	// Next we can create a rigid body to serve as the floor, we make a large box
	// Create the settings for the collision volume (the shape). 
	// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
	BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));

	// Create the shape
	ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	assert(!floor_shape_result.HasError());

	// We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
	ShapeRefC floor_shape = floor_shape_result.Get();

	// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
	settings = BodyCreationSettings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

	/// tmp mesh
	//auto meshComp = AddComponent< MeshComponent>();
	//meshComp->mesh(game()->meshAsset()->GetMesh(MeshAsset::formBox));
	//localScale({ 200 * tmp_scale, 2 * tmp_scale, 200 * tmp_scale });
}


DEF_COMPONENT(SphereCollider, Engine.SphereCollider, 0, RunMode::EditPlay) {

}

void SphereCollider::OnInit() {
	//std::cout << "OnInit" << "\n";
	float tmp_scale = 40;

	settings = BodyCreationSettings(new SphereShape(0.5f * tmp_scale), RVec3(0.0_r, 2.0_r * tmp_scale, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

	/// tmp mesh
	//auto meshComp = AddComponent< MeshComponent>();
	//meshComp->mesh(game()->meshAsset()->GetMesh(MeshAsset::formSphere));
	//localScale({ 0.5f * tmp_scale, 0.5f * tmp_scale, 0.5f * tmp_scale });
	//localPosition({ 0, 2.0_r * tmp_scale, 0 });
}

//void SphereCollider::OnStart() { std::cout << "OnStart" << "\n"; }
//void SphereCollider::OnUpdate() { std::cout << "OnUpdate" << "\n"; }
//void SphereCollider::OnFixedUpdate() { std::cout << "OnFixedUpdate" << "\n"; }
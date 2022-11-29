#include "MoveController.h"

#include <d3d11.h>
#include <SimpleMath.h>

#include "Game.h"

using namespace DirectX::SimpleMath;

void MoveController::OnInit() {
	//m_mouseMoveHandle = game()->input()->MouseMove.AddRaw(this, &MoveController::m_OnMouseMove);
}

void MoveController::OnDestroy() {
	//game()->input()->MouseMove.Remove(m_mouseMoveHandle);
}

void MoveController::OnUpdate() {
	auto input = game()->input();
	if (input->IsKeyDown(Keys::RightButton))
		return;

	auto axis = Vector3::Zero;

	if (input->IsKeyDown(Keys::W)) axis += Vector3::Right;
	if (input->IsKeyDown(Keys::S)) axis += Vector3::Left;
	if (input->IsKeyDown(Keys::A)) axis += Vector3::Forward;
	if (input->IsKeyDown(Keys::D)) axis += Vector3::Backward;
	if (input->IsKeyDown(Keys::Space)) axis += Vector3::Up;
	if (input->IsKeyDown(Keys::LeftShift)) axis += Vector3::Down;

	axis.Normalize();

	auto direction = Vector3::Forward * axis.x + Vector3::Up * axis.y + Vector3::Right * axis.z;

	direction.Normalize();
	auto newPos = transform->localPosition() + direction * speed * game()->deltaTime();
	transform->localPosition(newPos);

	//transform->position += m_delta;
	//m_delta = Vector3::Zero;
}


//void MoveController::m_OnMouseMove(const InputDevice::MouseMoveArgs& args) {
//	auto input = game()->input();
//	if (!game()->input()->IsKeyDown(Keys::LeftButton))
//		return;
//
//	auto direction = Vector3::Forward * -input->MouseOffset.y + Vector3::Up * 0 + Vector3::Right * input->MouseOffset.x;
//	direction.Normalize();
//
//
//	transform->position += direction * speed * game()->deltaTime() * mouseSense;
//}
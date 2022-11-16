#include "FlyingCamera.h"

#include "Game.h"


void FlyingCamera::OnInit() {
	m_mouseMoveHandle = game()->input()->MouseMove.AddRaw(this, &FlyingCamera::m_OnMouseMove);
	UpdateProjectionMatrix(game()->window());

	game()->hotkeys()->RegisterHotkey(Keys::O);
}

void FlyingCamera::OnDestroy() {
	game()->input()->MouseMove.Remove(m_mouseMoveHandle);
	game()->hotkeys()->UnregisterHotkey(Keys::O);
}

void FlyingCamera::OnUpdate() {
	if (!IsAttached())
		return;

	if (game()->hotkeys()->Is(Keys::O, KeyState::Press))
		orthographic(!orthographic());

	if (m_updateRotation) {
		auto rot = transform.localRotation();
		rot += m_rotationDelta;
		transform.localRotation(rot);
		m_rotationDelta = Vector3::Zero;
		m_updateRotation = false;
	}

	auto rot = transform.localRotation();
	auto rotator = Matrix::CreateFromYawPitchRoll(rot.y, rot.x, 0);

	auto input = game()->input();
	if (input->IsKeyDown(Keys::RightButton)) {

		auto axis = Vector3::Zero;
		if (input->IsKeyDown(Keys::W)) axis += Vector3::Right;
		if (input->IsKeyDown(Keys::S)) axis += Vector3::Left;
		if (input->IsKeyDown(Keys::A)) axis += Vector3::Forward;
		if (input->IsKeyDown(Keys::D)) axis += Vector3::Backward;
		if (input->IsKeyDown(Keys::Space)) axis += Vector3::Up;
		if (input->IsKeyDown(Keys::LeftShift)) axis += Vector3::Down;
		axis.Normalize();

		auto direction = rotator.Forward() * axis.x + Vector3::Up * axis.y + rotator.Right() * axis.z;
		direction.Normalize();

		auto newPos = transform.localPosition() + direction * m_speed * game()->deltaTime();
		transform.localPosition(newPos);
	}

	//auto m = transform.GetWorldMatrix();
	auto newMatrix = Matrix::CreateLookAt(transform.worldPosition(), transform.worldPosition() + rotator.Forward(), rotator.Up());
	viewMatrix(newMatrix);
	UpdateProjectionMatrix(game()->window());

	if (printTransform)
		game()->SendGameMessage(std::to_string(gameObject()->Id()) + " tr");
}

void FlyingCamera::m_OnMouseMove(const InputDevice::MouseMoveArgs& args) {
	if (!IsAttached())
		return;

	if (!game()->input()->IsKeyDown(Keys::RightButton))
		return;

	m_rotationDelta.y -= args.Offset.x * 0.003f * m_mouseSense;
	m_rotationDelta.x -= args.Offset.y * 0.003f * m_mouseSense;
	m_updateRotation = true;

	if (args.WheelDelta > 0)
		m_speed += 1;
	if (args.WheelDelta < 0)
		m_speed -= 1;
}

void FlyingCamera::RecieveGameMessage(const std::string& msg) {
	if (msg == "camera.print.tr")
		printTransform = !printTransform;
}


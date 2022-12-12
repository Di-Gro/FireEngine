#include "PlayerCamera.h"

#include <iostream>


DEF_PURE_COMPONENT(PlayerCamera);


void PlayerCamera::OnInit() {
	m_cameraRoot = parent();
	if(!m_cameraRoot)
		std::cout << "PlayerCamera haven't got a parent\n";

	//nearPlane *= 100;
	//farPlane *= 100;

	m_mouseMoveHandle = game()->input()->MouseMove.AddRaw(this, &PlayerCamera::m_OnMouseMove);
	UpdateProjectionMatrix();
	Attach();
}

void PlayerCamera::OnDestroy() {
	game()->input()->MouseMove.Remove(m_mouseMoveHandle);
}

void PlayerCamera::OnUpdate() {
	if (!IsAttached())
		return;

	if (m_updateRotation) {
		auto rot = m_cameraRoot->localRotation();
		rot += m_rotationDelta;
		m_cameraRoot->localRotation(rot);
		m_rotationDelta = Vector3::Zero;
		m_updateRotation = false;
	}

	if (m_updateDistance) {
		auto pos = localPosition();
		pos.z += m_distanceDelta;
		localPosition(pos);
		m_distanceDelta = 0;
		m_updateDistance = false;
	}

	auto rot = m_cameraRoot->localRotation();
	auto rotator = Matrix::CreateFromYawPitchRoll(rot.y, rot.x, 0);

	auto wpos = worldPosition();// *100;
	auto newMatrix = Matrix::CreateLookAt(wpos, wpos + rotator.Forward(), rotator.Up());
	viewMatrix(newMatrix);
	UpdateProjectionMatrix();

}

void PlayerCamera::m_OnMouseMove(const InputDevice::MouseMoveArgs& args) {
	if (!IsAttached() || !game()->inFocus)
		return;

	bool hasKey1 = game()->input()->IsKeyDown(Keys::Key1);
	bool hasKey2 = game()->input()->IsKeyDown(Keys::Key2);
	bool hasKey3 = game()->input()->IsKeyDown(Keys::Key3);
	bool hasKey4 = game()->input()->IsKeyDown(Keys::Key4);
	if (hasKey1 || hasKey2 || hasKey3 || hasKey4)
		return;

	m_rotationDelta.y -= args.Offset.x * 0.003f * m_rotationSense;
	m_rotationDelta.x -= args.Offset.y * 0.003f * m_rotationSense;
	m_updateRotation = true;
	
	if (args.WheelDelta > 0) {
		m_distanceDelta -= 2 * m_scrollSense;
		m_updateDistance = true;
	}
	if (args.WheelDelta < 0) {
		m_distanceDelta += 2 * m_scrollSense;
		m_updateDistance = true;
	}
}
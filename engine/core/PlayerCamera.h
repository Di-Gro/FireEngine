#pragma once

#include "CameraComponent.h"
#include "InputDevice.h"

class PlayerCamera : public CameraComponent {
	PURE_COMPONENT(PlayerCamera)

private:
	float m_rotationSense = 1.0f;
	float m_scrollSense = 1.0f;

	bool m_updateRotation = false;
	bool m_updateDistance = false;

	Vector3 m_rotationDelta;
	float m_distanceDelta;

	Actor* m_cameraRoot = nullptr;

	DelegateHandle m_mouseMoveHandle;

public:
	void OnInit() override;
	void OnDestroy() override;
	void OnUpdate() override;

private:
	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);
};

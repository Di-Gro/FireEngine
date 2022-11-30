#pragma once

#include "Game.h"
#include "CameraComponent.h"

class PlayerCamera : public CameraComponent {
	PURE_COMPONENT;

private:
	float m_rotationSense = 1.0f;
	float m_scrollSense = 1.0f;

	bool m_updateRotation = false;
	bool m_updateDistance = false;

	Vector3 m_rotationDelta;
	float m_distanceDelta;

	GameObject* m_cameraRoot = nullptr;

	DelegateHandle m_mouseMoveHandle;

public:
	//PlayerCamera(GameObject* gameObject) : CameraComponent(gameObject) { }

	void OnInit() override;
	void OnDestroy() override;
	void OnUpdate() override;

private:
	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);
};

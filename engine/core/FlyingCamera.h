#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "CameraComponent.h"
#include "Game.h"

using namespace DirectX::SimpleMath;


class FlyingCamera : public CameraComponent {

private:

	bool m_updateRotation = false;

	Vector3 m_rotationDelta;

	float m_speed = 100.0f;
	float m_mouseSense = 1.0f;

	bool printTransform = false;

	DelegateHandle m_mouseMoveHandle;

public:

	FlyingCamera(GameObject* gameObject) : CameraComponent(gameObject) {}

	void OnInit() override;
	void OnDestroy() override;
	void OnUpdate() override;

	void RecieveGameMessage(const std::string& msg) override;

private:

	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);

};


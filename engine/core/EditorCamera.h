#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "CSBridge.h"

#include "CameraComponent.h"
#include "InputDevice.h"

using namespace DirectX::SimpleMath;


class EditorCamera : public CameraComponent {
	PURE_COMPONENT(EditorCamera)

private:

	bool m_updateRotation = false;

	Vector3 m_rotationDelta;

	float m_speed = 100.0f;
	float m_mouseSense = 1.0f;

	bool printTransform = false;

	DelegateHandle m_mouseMoveHandle;

public:

	//FlyingCamera(Actor* actor) : CameraComponent(actor) {}

	void OnInit() override;
	void OnDestroy() override;
	void OnUpdate() override;

	void RecieveGameMessage(const std::string& msg) override;

private:

	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);

};


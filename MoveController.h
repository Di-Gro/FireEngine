#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "Game.h"

using namespace DirectX::SimpleMath;


class MoveController : public Component {
public:
	float speed = 100.0f;
	float mouseSense = 1.0f;

private:
	DelegateHandle m_mouseMoveHandle;
	Vector3 m_delta;

public:
	MoveController(GameObject* gameObject) : Component(gameObject) {}

	void OnInit() override;
	void OnUpdate() override;
	void OnDestroy() override;

private:
	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);

};


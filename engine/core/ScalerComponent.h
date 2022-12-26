#pragma once

#pragma once
#include <string>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Actor.h"

#include "Math.h"

using namespace DirectX::SimpleMath;


class ScalerComponent : public Component {
private:
	
	Vector3 m_scale = Vector3::One;
	bool m_needUpdate = false;

	DelegateHandle m_mouseMoveHandle;

public:

	//ScalerComponent(Actor* gameObject) : Component(gameObject) {}

	void OnInit() override {
		m_mouseMoveHandle = game()->input()->MouseMove.AddRaw(this, &ScalerComponent::m_OnMouseMove);
	}

	void OnDestroy() {
		game()->input()->MouseMove.Remove(m_mouseMoveHandle);
	}

	void OnUpdate() override {
		if (m_needUpdate) {
			m_needUpdate = false;
			localScale(m_scale);
		}
	}

	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args) {
		float value = 0;

		if (!game()->input()->IsKeyDown(Keys::R))
			return;

		if (args.WheelDelta > 0)
			value += 1;

		if (args.WheelDelta < 0)
			value -= 1;

		if (value != 0) {
			m_scale *= (1 + value * 0.01f);
			m_needUpdate = true;
		}
	}


};



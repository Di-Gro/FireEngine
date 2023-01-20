#pragma once

#include <unordered_map>

#include "InputDevice.h"
#include "SimpleMath.h"
#include "CSBridge.h"
#include "CS.h"

class Game;

class HotKeys {
private:
	struct MapItem {
		KeyState state = KeyState::None;
		unsigned int count = 0;
	};

	std::unordered_map<Keys, MapItem> m_states;
	Game* m_game = nullptr;
	DelegateHandle m_mouseMoveHandle;
	int m_mouseWheelDelta = 0;
	Vector2 m_mousePosDelta = Vector2::Zero;

public:
	void Init(Game* game);
	void Update(InputDevice* input);
	void LateUpdate();
	void Destroy();

	bool Is(Keys key, KeyState state);
	bool Is(Keys key);

	bool GetButtonDown(Keys key);
	bool GetButtonDown(Keys key, Keys modifier);
	bool GetButtonDown(Keys key, Keys modifier1, Keys modifier2);
	bool GetButtonUp(Keys key);
	bool GetButton(Keys key);

	bool GetButtonDownEd(Keys key);
	bool GetButtonDownEd(Keys key, Keys modifier);
	bool GetButtonDownEd(Keys key, Keys modifier1, Keys modifier2);
	bool GetButtonUpEd(Keys key);
	bool GetButtonEd(Keys key);

	inline DirectX::SimpleMath::Vector2 GetMousePosition();
	inline int wheelDelta() { return m_mouseWheelDelta; }
	inline Vector2 mouseDelta() { return m_mousePosDelta; }

	void RegisterHotkey(Keys key);
	void UnregisterHotkey(Keys key);

private:
	void m_InitMono();

	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);

};

FUNC(HotKeys, GetButtonDown, bool)(CppRef objRef, int keyCode);
FUNC(HotKeys, GetButtonUp, bool)(CppRef objRef, int keyCode);
FUNC(HotKeys, GetButton, bool)(CppRef objRef, int keyCode);

FUNC(HotKeys, RegisterHotkey, void)(CppRef objRef, int keyCode);
FUNC(HotKeys, UnregisterHotkey, void)(CppRef objRef, int keyCode);

struct float2 {
	float _1;
	float _2;
};

FUNC(HotKeys, MousePosition, CS::float2)(CppRef objRef);

PROP_GET(HotKeys, int, wheelDelta)
PROP_GET(HotKeys, CS::float2, mouseDelta)
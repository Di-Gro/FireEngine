#include "HotKeys.h"

#include "InputDevice.h"
#include "Game.h"
#include "CSBridge.h"

//using namespace CS;

void HotKeys::Init(Game* game) {
	m_game = game;
	m_InitMono();

	m_mouseMoveHandle = m_game->input()->MouseMove.AddRaw(this, &HotKeys::m_OnMouseMove);

	RegisterHotkey(Keys::Tilda);
}

void HotKeys::m_InitMono() {
	auto type = m_game->mono()->GetType("Engine", "Input");
	auto method = mono::make_method_invoker<void(CppRef)>(type, "cpp_OnInit");

	CppRef cppRef = RefCpp((CppRefs::Create(this).cppRef()));
	method(cppRef);
}

void HotKeys::Destroy() {
	auto type = m_game->mono()->GetType("Engine", "Input");
	auto method = mono::make_method_invoker<void(void)>(type, "cpp_OnDestroy");

	method();

	m_game->input()->MouseMove.Remove(m_mouseMoveHandle);
}

void HotKeys::Update(InputDevice* input) {
	for (auto it = m_states.begin(); it != m_states.end(); it++) {
		bool hasDown = input->IsKeyDown(it->first);
		auto& state = it->second.state;

		if (hasDown) {
			if (state == KeyState::None || state == KeyState::Release)
				state = KeyState::Press;

			else if (state == KeyState::Press)
				state = KeyState::Hold;
		}
		else {
			if (state == KeyState::Press || state == KeyState::Hold)
				state = KeyState::Release;

			else if (state == KeyState::Release)
				state = KeyState::None;
		}
	}
}

void HotKeys::LateUpdate() {
	m_mouseWheelDelta = 0;
}

void HotKeys::RegisterHotkey(Keys key) {
	if (m_states.count(key) == 0)
		m_states[key] = { KeyState::None, 1 };
	else
		m_states[key].count++;
}

void HotKeys::UnregisterHotkey(Keys key) {
	auto it = m_states.find(key);

	if (it != m_states.end()) {
		it->second.count--;
		if (it->second.count == 0)
			m_states.erase(key);
	}
}

bool HotKeys::Is(Keys key, KeyState state) {
	if (m_states.count(key) > 0)
		return m_states[key].state == state;

	return false;
}

bool HotKeys::Is(Keys key) {
	if (m_states.count(key) > 0)
		return m_states[key].state != KeyState::None;

	return false;
}

bool HotKeys::GetButtonDown(Keys key) {
	return Is(key, KeyState::Press);
}

bool HotKeys::GetButtonUp(Keys key) {
	return Is(key, KeyState::Release);
}

bool HotKeys::GetButton(Keys key) {
	return Is(key);
}

DirectX::SimpleMath::Vector2 HotKeys::GetMousePosition() {
	return m_game->input()->MousePosition;
}

void HotKeys::m_OnMouseMove(const InputDevice::MouseMoveArgs& args) {
	if (args.WheelDelta != 0)
		m_mouseWheelDelta += args.WheelDelta > 0 ? 1 : -1;
}


DEF_FUNC(HotKeys, GetButtonDown, bool)(CppRef objRef, int keyCode) {
	return CppRefs::ThrowPointer<HotKeys>(objRef)->GetButtonDown((Keys)keyCode);
}

DEF_FUNC(HotKeys, GetButtonUp, bool)(CppRef objRef, int keyCode) {
	return CppRefs::ThrowPointer<HotKeys>(objRef)->GetButtonUp((Keys)keyCode);
}

DEF_FUNC(HotKeys, GetButton, bool)(CppRef objRef, int keyCode) {
	return CppRefs::ThrowPointer<HotKeys>(objRef)->GetButton((Keys)keyCode);
}

DEF_FUNC(HotKeys, RegisterHotkey, void)(CppRef objRef, int keyCode) {
	CppRefs::ThrowPointer<HotKeys>(objRef)->RegisterHotkey((Keys)keyCode);
}

DEF_FUNC(HotKeys, UnregisterHotkey, void)(CppRef objRef, int keyCode) {
	CppRefs::ThrowPointer<HotKeys>(objRef)->UnregisterHotkey((Keys)keyCode);
}

DEF_FUNC(HotKeys, MousePosition, CS::float2)(CppRef objRef) {
	return CS::ToCS(CppRefs::ThrowPointer<HotKeys>(objRef)->GetMousePosition());
}

DEF_PROP_GET(HotKeys, int, wheelDelta)
#include "InputDevice.h"
#include <iostream>
#include "Game.h"


using namespace DirectX::SimpleMath;


InputDevice::InputDevice() {

}

void InputDevice::Init(Game* inGame) {
	m_game = inGame;
	m_game->window()->SetInputHandler(this);

	m_keys = new std::unordered_set<Keys>();

	RAWINPUTDEVICE Rid[2];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = 0;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = m_game->window()->GetHWindow();

	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = 0;   // adds HID keyboard and also ignores legacy keyboard messages
	Rid[1].hwndTarget = m_game->window()->GetHWindow();

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE) {
		auto errorCode = GetLastError();
		std::cout << "ERROR: " << errorCode << std::endl;
	}

	//m_InitMono();
}

InputDevice::~InputDevice() {
	delete m_keys;
}

//void InputDevice::m_InitMono() {
//	auto type = m_game->mono()->GetType("Engine", "Input");
//	auto method_SetRef = mono::make_method_invoker<void(CppRef)>(type, "cpp_SetInputDeviceRef");
//
//	CppRef cppRef = RefCpp((CppRefs::Create(this).cppRef()));
//	method_SetRef(cppRef);
//}

void InputDevice::OnInput(LPARAM lparam) {
	UINT dwSize = 0;
	GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
	LPBYTE lpb = new BYTE[dwSize];
	if (lpb == nullptr)
		return;

	if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

	RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);

	if (raw->header.dwType == RIM_TYPEKEYBOARD) {
		//std::cout << "MakeCode: " << raw->data.keyboard.MakeCode;
		//std::cout << " Flags: " << raw->data.keyboard.Flags;
		//std::cout << " VKey: " << raw->data.keyboard.VKey;
		//std::cout << " Message: " << raw->data.keyboard.Message << std::endl;

		OnKeyDown({
			raw->data.keyboard.MakeCode,
			raw->data.keyboard.Flags,
			raw->data.keyboard.VKey,
			raw->data.keyboard.Message
		});

	} else if (raw->header.dwType == RIM_TYPEMOUSE) {
		//std::cout << " usButtonFlags: " << raw->data.mouse.usButtonFlags << std::endl;

		OnMouseMove({
			raw->data.mouse.usFlags,
			raw->data.mouse.usButtonFlags,
			static_cast<int>(raw->data.mouse.ulExtraInformation),
			static_cast<int>(raw->data.mouse.ulRawButtons),
			static_cast<short>(raw->data.mouse.usButtonData),
			raw->data.mouse.lLastX,
			raw->data.mouse.lLastY
		});
	}

	delete[] lpb;
}

void InputDevice::OnKeyDown(KeyboardArgs args) {
	bool Break = args.Flags & 0x01;

	auto key = static_cast<Keys>(args.VKey);

	if (args.MakeCode == 42) key = Keys::LeftShift;
	if (args.MakeCode == 54) key = Keys::RightShift;

	if (Break) {
		if (m_keys->count(key))	RemovePressedKey(key);
	}
	else {
		if (!m_keys->count(key))	AddPressedKey(key);
	}
}

void InputDevice::OnMouseMove(RawMouseArgs args) {
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::LeftButtonDown))
		AddPressedKey(Keys::LeftButton);

	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::LeftButtonUp))
		RemovePressedKey(Keys::LeftButton);

	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::RightButtonDown))
		AddPressedKey(Keys::RightButton);

	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::RightButtonUp))
		RemovePressedKey(Keys::RightButton);

	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::MiddleButtonDown))
		AddPressedKey(Keys::MiddleButton);

	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::MiddleButtonUp))
		RemovePressedKey(Keys::MiddleButton);

	POINT p;
	GetCursorPos(&p);
	ScreenToClient(m_game->window()->GetHWindow(), &p);

	MousePosition = Vector2(p.x, p.y);
	MouseOffset = Vector2(args.X, args.Y);
	MouseWheelDelta = args.WheelDelta;

	const MouseMoveArgs moveArgs = { MousePosition, MouseOffset, MouseWheelDelta };

	MouseMove.Broadcast(moveArgs);
}

void InputDevice::AddPressedKey(Keys key) {
	m_keys->insert(key);
}

void InputDevice::RemovePressedKey(Keys key) {
	m_keys->erase(key);
}

bool InputDevice::IsKeyDown(Keys key) {
	return m_keys->count(key);
}

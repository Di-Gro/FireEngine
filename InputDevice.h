#pragma once

#include <unordered_set>
#include <d3d11.h>
#include <SimpleMath.h>

#include "Keys.h"
#include "IWin32InputHandler.h"
#include "Delegates.h"

class Game;


class InputDevice : public IWin32InputHandler {
	friend class Game;

	Game* m_game;

	std::unordered_set<Keys>* m_keys;

public:

	struct MouseMoveArgs {
		DirectX::SimpleMath::Vector2 Position;
		DirectX::SimpleMath::Vector2 Offset;
		int WheelDelta;
	};

	DirectX::SimpleMath::Vector2 MousePosition;
	DirectX::SimpleMath::Vector2 MouseOffset;
	int MouseWheelDelta = 0;

	MulticastDelegate<const MouseMoveArgs&> MouseMove; 

public:

	InputDevice();
	void Init(Game* inGame);
	~InputDevice();

	bool IsKeyDown(Keys key);	

protected:
	struct KeyboardArgs {
		USHORT MakeCode;
		USHORT Flags;
		USHORT VKey;
		UINT   Message;
	};

	enum class MouseButtonFlags {
		/// <unmanaged>RI_MOUSE_LEFT_BUTTON_DOWN</unmanaged>
		LeftButtonDown = 1,
		/// <unmanaged>RI_MOUSE_LEFT_BUTTON_UP</unmanaged>
		LeftButtonUp = 2,
		/// <unmanaged>RI_MOUSE_RIGHT_BUTTON_DOWN</unmanaged>
		RightButtonDown = 4,
		/// <unmanaged>RI_MOUSE_RIGHT_BUTTON_UP</unmanaged>
		RightButtonUp = 8,
		/// <unmanaged>RI_MOUSE_MIDDLE_BUTTON_DOWN</unmanaged>
		MiddleButtonDown = 16, // 0x00000010
		/// <unmanaged>RI_MOUSE_MIDDLE_BUTTON_UP</unmanaged>
		MiddleButtonUp = 32, // 0x00000020
		/// <unmanaged>RI_MOUSE_BUTTON_1_DOWN</unmanaged>
		Button1Down = LeftButtonDown, // 0x00000001
		/// <unmanaged>RI_MOUSE_BUTTON_1_UP</unmanaged>
		Button1Up = LeftButtonUp, // 0x00000002
		/// <unmanaged>RI_MOUSE_BUTTON_2_DOWN</unmanaged>
		Button2Down = RightButtonDown, // 0x00000004
		/// <unmanaged>RI_MOUSE_BUTTON_2_UP</unmanaged>
		Button2Up = RightButtonUp, // 0x00000008
		/// <unmanaged>RI_MOUSE_BUTTON_3_DOWN</unmanaged>
		Button3Down = MiddleButtonDown, // 0x00000010
		/// <unmanaged>RI_MOUSE_BUTTON_3_UP</unmanaged>
		Button3Up = MiddleButtonUp, // 0x00000020
		/// <unmanaged>RI_MOUSE_BUTTON_4_DOWN</unmanaged>
		Button4Down = 64, // 0x00000040
		/// <unmanaged>RI_MOUSE_BUTTON_4_UP</unmanaged>
		Button4Up = 128, // 0x00000080
		/// <unmanaged>RI_MOUSE_BUTTON_5_DOWN</unmanaged>
		Button5Down = 256, // 0x00000100
		/// <unmanaged>RI_MOUSE_BUTTON_5_UP</unmanaged>
		Button5Up = 512, // 0x00000200
		/// <unmanaged>RI_MOUSE_WHEEL</unmanaged>
		MouseWheel = 1024, // 0x00000400
		/// <unmanaged>RI_MOUSE_HWHEEL</unmanaged>
		Hwheel = 2048, // 0x00000800

		None = 0,
	};

	struct RawMouseArgs {
		int Mode;
		int ButtonFlags;
		int ExtraInformation;
		int Buttons;
		int WheelDelta;
		int X;
		int Y;
	};

	void OnInput(LPARAM lparam) override;

	void AddPressedKey(Keys key);
	void RemovePressedKey(Keys key);
	void OnKeyDown(KeyboardArgs args);
	void OnMouseMove(RawMouseArgs args);
};

#pragma once

#include <windows.h>
#include <WinUser.h>

namespace DiGro {
	namespace GameFramework {
		class IWindow {

        public:

            virtual int GetHeight() = 0;
            virtual int GetWidth() = 0;

            virtual HINSTANCE GetHInstance() = 0;
            virtual WNDCLASSEX GetWindowClass() = 0;
            virtual HWND GetHWindow() = 0;

            virtual void Create() = 0;
            virtual void Exit(int code) = 0;

            virtual LRESULT MassageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) = 0;

            virtual ~IWindow() {};
		};
	}
}


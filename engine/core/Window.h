#pragma once

#include <iostream>
#include <windows.h>
#include <WinUser.h>

#include "IWin32InputHandler.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class Game;

class Window {
public:
    static Window* s_window;

public:
    bool sizeChanged = false;

private:
    Game* m_game;
    int m_height = 800;
    int m_width = 800;
    HINSTANCE m_hInstance = 0;
    LPCWSTR m_name = L"Window";
    WNDCLASSEX m_wc = {};
    HWND m_hWnd = 0;
    IWin32InputHandler* m_inputHandler = nullptr;
    bool m_showCursor = true;

    RECT m_rcOldClip; // previous area for ClipCursor

    bool m_isCursorClipped = false;

public:

    Window() {};

    void Init(Game* game, LPCWSTR name, int width, int height);
    void Destroy();

    int GetHeight() { return m_height; }
    int GetWidth() { return m_width; }
    HINSTANCE GetHInstance() { return m_hInstance; }
    WNDCLASSEX GetWindowClass() { return m_wc; }
    HWND GetHWindow() { return m_hWnd; }

    void SetInputHandler(IWin32InputHandler* handler) { m_inputHandler = handler; };

    void Create();
    void Exit(int code);

    LRESULT MassageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

    void ClipCursor(float x, float y, float width, float height);
    void UnclipCursor();
    bool IsCursorClipped() { return m_isCursorClipped; }


    Vector2 ScreenToViewport(Vector2 screenPositon);
    Vector2 ScreenToOffset(Vector2 screenPositon);
    Vector2 ViewportToOffset(Vector2 viewportPosition);
    
};



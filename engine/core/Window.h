#pragma once
#include "IWindow.h"

#include <iostream>
#include <windows.h>
#include <WinUser.h>

#include "IWin32InputHandler.h"
#include <SimpleMath.h>

using namespace DiGro::GameFramework;
using namespace DirectX::SimpleMath;

class Game;

class Window : public IWindow {
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

    //RECT m_rcOldClip; // previous area for ClipCursor

public:

    Window() {};

    void Init(Game* game, LPCWSTR name, int width, int height);
    void Destroy();

    int GetHeight() override { return m_height; }
    int GetWidth() override { return m_width; }
    HINSTANCE GetHInstance() override { return m_hInstance; }
    WNDCLASSEX GetWindowClass() override { return m_wc; }
    HWND GetHWindow() override { return m_hWnd; }

    void SetInputHandler(IWin32InputHandler* handler) { m_inputHandler = handler; };

    void Create() override;
    void Exit(int code) override;

    LRESULT MassageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) override;


    Vector2 ScreenToViewport(Vector2 screenPositon);
    Vector2 ScreenToOffset(Vector2 screenPositon);
    Vector2 ViewportToOffset(Vector2 viewportPosition);
    
};



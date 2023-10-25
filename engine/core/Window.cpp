#include "Window.h"

#include "imgui\imgui_impl_win32.h"

#include "Game.h"

Window* Window::s_window = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    auto* window = Window::s_window;
    //Window* window = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if(window != nullptr)
        return window->MassageHandler(hwnd, umessage, wparam, lparam);

    return DefWindowProc(hwnd, umessage, wparam, lparam);
}

void Window::Init(Game* game, LPCWSTR name, int width, int height) {
    m_game = game;

    m_name = name;
    m_width = width;
    m_height = height;
    m_hInstance = GetModuleHandle(nullptr);

    m_wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    m_wc.lpfnWndProc = WndProc;
    m_wc.cbClsExtra = 0;
    m_wc.cbWndExtra = 0;
    m_wc.hInstance = m_hInstance;
    m_wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    m_wc.hIconSm = m_wc.hIcon;
    m_wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    m_wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    m_wc.lpszMenuName = nullptr;
    m_wc.lpszClassName = name;
    m_wc.cbSize = sizeof(WNDCLASSEX);

    RegisterClassEx(&m_wc);
}

void Window::Create() {
    s_window = this;

    RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    auto posX = (GetSystemMetrics(SM_CXSCREEN) - m_width) / 2;
    auto posY = (GetSystemMetrics(SM_CYSCREEN) - m_height) / 2;

    m_hWnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        m_name,
        m_name,
        dwStyle,
        posX, posY,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, m_hInstance, nullptr);

    //SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

    ShowWindow(m_hWnd, SW_MAXIMIZE);
    SetForegroundWindow(m_hWnd);
    ShowCursor(m_showCursor);

    // Record the area in which the cursor can move.
    GetClipCursor(&m_rcOldClip);
    //GetWindowRect(m_hWnd, &windowRect); // Get the dimensions of the application's window.
    //ClipCursor(&windowRect);  // Confine the cursor to the application's window.

}

void Window::Destroy() {
    UnclipCursor();
}

void Window::ClipCursor(float x, float y, float width, float height) {
    RECT rect = { (LONG)x, (LONG)y, (LONG)(x + width), (LONG)(y + height)};

    ::ClipCursor(&rect);

    m_isCursorClipped = true;
}

void Window::UnclipCursor() {
    ::ClipCursor(&m_rcOldClip);
}

void Window::Exit(int code) { PostQuitMessage(code); }

LRESULT Window::MassageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    LRESULT res = 0;
    
    res = ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam);

    m_game->WindowMassageHandler(hwnd, umessage, wparam, lparam);

    switch (umessage) {
    case WM_CLOSE: {
        m_game->Exit(0);
        return 0;
    }
    case WM_SIZE: {
        m_width = LOWORD(lparam);
        m_height = HIWORD(lparam);
        sizeChanged = true;
        return 0;
    }
    case WM_INPUT: {
        if (m_inputHandler != nullptr)
            m_inputHandler->OnInput(lparam);
        break;
    }
    }

    if (res != 0)
        return res;

    return DefWindowProc(hwnd, umessage, wparam, lparam);
}

Vector2 Window::ScreenToViewport(Vector2 screenPositon) {
    return screenPositon / Vector2(m_width, m_height);
}

Vector2 Window::ScreenToOffset(Vector2 screenPositon) {
    return ViewportToOffset(ScreenToViewport(screenPositon));
}

Vector2 Window::ViewportToOffset(Vector2 viewportPosition) {
    Vector2 d;
    d.x = (viewportPosition.x - 0.5f) * 2;
    d.y = (0.5f - viewportPosition.y) * 2;
    return  d;
}

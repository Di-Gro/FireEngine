#include "Window.h"


static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    using Iwindow = DiGro::GameFramework::IWindow;
    IWindow* window = (Iwindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if(window != nullptr)
        return window->MassageHandler(hwnd, umessage, wparam, lparam);

    return DefWindowProc(hwnd, umessage, wparam, lparam);
}

void Window::Init(LPCWSTR name, int width, int height) {
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
    RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

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

    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

    ShowWindow(m_hWnd, SW_SHOW);
    SetForegroundWindow(m_hWnd);
    ShowCursor(m_showCursor);
}

void Window::Exit(int code) { PostQuitMessage(code); }

LRESULT Window::MassageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    switch (umessage) {
    case WM_CLOSE: {

        return DefWindowProc(hwnd, umessage, wparam, lparam);
    }
    case WM_KEYDOWN: {
        //std::cout << "Key: " << static_cast<unsigned int>(wparam) << std::endl;
        return 0;
    }
    case WM_SIZE: {
        m_width = LOWORD(lparam);
        m_height = HIWORD(lparam);
        return 0;
    }
    case WM_INPUT: {
        if (m_inputHandler != nullptr)
            m_inputHandler->OnInput(lparam);
    }
    // Fall is OK
    default:
        return DefWindowProc(hwnd, umessage, wparam, lparam);
    }
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
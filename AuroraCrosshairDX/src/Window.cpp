#include "Window.h"

Window::Window()
    : hWnd(nullptr), width(1920), height(1080), className(L"AuroraCrosshairWindow")
{
}

Window::~Window()
{
    Destroy();
}

bool Window::Create(const std::wstring& title, unsigned int width, unsigned int height)
{
    this->width = width;
    this->height = height;

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = className.c_str();

    if (!RegisterClassEx(&wc))
        return false;

    hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT,
        className.c_str(),
        title.c_str(),
        WS_POPUP,
        0, 0, width, height,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        nullptr);

    if (!hWnd)
        return false;

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    return true;
}

void Window::Destroy()
{
    if (hWnd)
    {
        DestroyWindow(hWnd);
        hWnd = nullptr;
    }

    UnregisterClass(className.c_str(), GetModuleHandle(nullptr));
}

bool Window::ProcessMessages()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

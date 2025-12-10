#pragma once

#include <windows.h>
#include <string>

class Window
{
public:
    Window();
    ~Window();

    bool Create(const std::wstring& title, unsigned int width, unsigned int height);
    void Destroy();
    
    bool ProcessMessages();
    
    HWND GetHandle() const { return hWnd; }
    unsigned int GetWidth() const { return width; }
    unsigned int GetHeight() const { return height; }

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    HWND hWnd;
    unsigned int width;
    unsigned int height;
    std::wstring className;
};

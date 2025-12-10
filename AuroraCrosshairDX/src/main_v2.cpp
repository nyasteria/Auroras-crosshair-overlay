#include <windows.h>

HWND g_hWnd = nullptr;
int g_crosshairSize = 20;
int g_crosshairThickness = 3;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void RenderCrosshair()
{
    // Create a memory DC for off-screen rendering
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    
    // Create a 32-bit DIB (Device Independent Bitmap) for alpha blending
    BITMAPV5HEADER bi = {};
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = 1920;
    bi.bV5Height = 1080;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;
    
    void* pvBits = nullptr;
    HBITMAP hbmMem = CreateDIBSection(hdcMem, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    
    if (!hbmMem || !pvBits)
    {
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        return;
    }
    
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
    
    // Clear to transparent (0,0,0,0)
    DWORD* pixels = (DWORD*)pvBits;
    for (int i = 0; i < 1920 * 1080; i++)
        pixels[i] = 0;  // ARGB: 0 = fully transparent
    
    // Draw pink crosshair
    HBRUSH pinkBrush = CreateSolidBrush(RGB(255, 102, 204));
    
    int centerX = 1920 / 2;
    int centerY = 1080 / 2;
    
    // Horizontal bar
    RECT hBar = {
        centerX - g_crosshairSize,
        centerY - g_crosshairThickness / 2,
        centerX + g_crosshairSize,
        centerY + g_crosshairThickness / 2
    };
    FillRect(hdcMem, &hBar, pinkBrush);
    
    // Vertical bar
    RECT vBar = {
        centerX - g_crosshairThickness / 2,
        centerY - g_crosshairSize,
        centerX + g_crosshairThickness / 2,
        centerY + g_crosshairSize
    };
    FillRect(hdcMem, &vBar, pinkBrush);
    
    // Manually set alpha for pink pixels (GDI doesn't support alpha blending to DIB directly)
    pixels = (DWORD*)pvBits;
    DWORD pinkColor = RGB(255, 102, 204);  // 0x00CC66FF in BGRA
    
    for (int i = 0; i < 1920 * 1080; i++)
    {
        if ((pixels[i] & 0x00FFFFFF) == pinkColor)
            pixels[i] |= 0xFF000000;  // Set alpha to fully opaque
    }
    
    // Update layered window with the bitmap
    SIZE sizeWnd = { 1920, 1080 };
    POINT ptSrc = { 0, 0 };
    BLENDFUNCTION blendFunc = {};
    blendFunc.BlendOp = AC_SRC_OVER;
    blendFunc.BlendFlags = 0;
    blendFunc.AlphaFormat = AC_SRC_ALPHA;
    blendFunc.SourceConstantAlpha = 255;
    
    UpdateLayeredWindow(g_hWnd, hdcScreen, nullptr, &sizeWnd, hdcMem, &ptSrc, 0, &blendFunc, ULW_ALPHA);
    
    // Cleanup
    DeleteObject(pinkBrush);
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register window class
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = L"AuroraCrosshair";
    RegisterClassEx(&wcex);

    // Create layered window for transparency support
    g_hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        L"AuroraCrosshair",
        L"Aurora Crosshair",
        WS_POPUP,
        0, 0, 1920, 1080,
        nullptr, nullptr, hInstance, nullptr);

    if (!g_hWnd)
        return -1;

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);

    // Main message loop
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            RenderCrosshair();
            Sleep(16);  // ~60 FPS
        }
    }

    return (int)msg.wParam;
}

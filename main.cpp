#include "MainWindow.h"
#include <Windows.h>
#include <comdef.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Failed to initialize COM", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Initialize Media Foundation
    hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Failed to initialize Media Foundation", L"Error", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return -1;
    }

    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Enable HiDPI awareness
    SetProcessDPIAware();

    // Create and show main window
    MainWindow mainWindow;
    if (!mainWindow.Initialize(hInstance)) {
        MessageBox(NULL, L"Failed to initialize main window", L"Error", MB_OK | MB_ICONERROR);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        MFShutdown();
        CoUninitialize();
        return -1;
    }

    mainWindow.Show(nCmdShow);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    Gdiplus::GdiplusShutdown(gdiplusToken);
    MFShutdown();
    CoUninitialize();

    return (int)msg.wParam;
}

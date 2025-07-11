#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <memory>
#include "CameraView.h"
#include "AnimatedMenu.h"

#ifdef _DEBUG
#include "DebugOverlay.h"
#endif

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool Initialize(HINSTANCE hInstance);
    void Show(int nCmdShow);

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OnPaint();
    void OnSize(UINT width, UINT height);
    void OnKeyDown(WPARAM wParam);
    void OnTimer(WPARAM timerId);
    void OnClose();

    bool CreateDeviceResources();
    void DiscardDeviceResources();
    void CalculateLayout();

    // Window and rendering
    HWND m_hwnd;
    HINSTANCE m_hInstance;
    ID2D1Factory* m_pD2DFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    IDWriteFactory* m_pDWriteFactory;
    IDWriteTextFormat* m_pTextFormat;

    // Components
    std::unique_ptr<CameraView> m_pCameraView;
    std::unique_ptr<AnimatedMenu> m_pAnimatedMenu;

#ifdef _DEBUG
    std::unique_ptr<DebugOverlay> m_pDebugOverlay;
#endif

    // Layout
    D2D1_RECT_F m_cameraRect;
    UINT m_clientWidth;
    UINT m_clientHeight;

    // Constants
    static const UINT_PTR TIMER_FRAME = 1;
    static const UINT FRAME_RATE = 60;
    static const UINT TIMER_INTERVAL = 1000 / FRAME_RATE;
    static const wchar_t* WINDOW_CLASS_NAME;
    static const wchar_t* WINDOW_TITLE;
};

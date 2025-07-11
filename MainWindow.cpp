#include "MainWindow.h"
#include <windowsx.h>

const wchar_t* MainWindow::WINDOW_CLASS_NAME = L"ModernCameraApp";
const wchar_t* MainWindow::WINDOW_TITLE = L"Modern Camera Application";

MainWindow::MainWindow() :
    m_hwnd(nullptr),
    m_hInstance(nullptr),
    m_pD2DFactory(nullptr),
    m_pRenderTarget(nullptr),
    m_pDWriteFactory(nullptr),
    m_pTextFormat(nullptr),
    m_clientWidth(0),
    m_clientHeight(0)
{
    m_cameraRect = D2D1::RectF();
}

MainWindow::~MainWindow()
{
    DiscardDeviceResources();
    if (m_pD2DFactory) {
        m_pD2DFactory->Release();
        m_pD2DFactory = nullptr;
    }
    if (m_pDWriteFactory) {
        m_pDWriteFactory->Release();
        m_pDWriteFactory = nullptr;
    }
    if (m_pTextFormat) {
        m_pTextFormat->Release();
        m_pTextFormat = nullptr;
    }
}

bool MainWindow::Initialize(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    // Create Direct2D factory
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (FAILED(hr)) {
        return false;
    }

    // Create DirectWrite factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), 
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
    if (FAILED(hr)) {
        return false;
    }

    // Create text format
    hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"en-US",
        &m_pTextFormat
    );
    if (FAILED(hr)) {
        return false;
    }

    // Register window class
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WINDOW_CLASS_NAME;
    wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex)) {
        return false;
    }

    // Calculate window size for HiDPI
    const int baseWidth = 1280;
    const int baseHeight = 720;
    
    RECT windowRect = { 0, 0, baseWidth, baseHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create window
    m_hwnd = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!m_hwnd) {
        return false;
    }

    // Initialize components
    m_pCameraView = std::make_unique<CameraView>();
    m_pAnimatedMenu = std::make_unique<AnimatedMenu>();

#ifdef _DEBUG
    m_pDebugOverlay = std::make_unique<DebugOverlay>();
    if (!m_pDebugOverlay->Initialize(hInstance)) {
        // Debug overlay is optional, continue without it
    }
#endif

    // Start frame timer
    SetTimer(m_hwnd, TIMER_FRAME, TIMER_INTERVAL, nullptr);

    return true;
}

void MainWindow::Show(int nCmdShow)
{
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MainWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hwnd = hwnd;
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    } else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_SIZE:
        OnSize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYDOWN:
        OnKeyDown(wParam);
        return 0;

    case WM_TIMER:
        OnTimer(wParam);
        return 0;

    case WM_CLOSE:
        OnClose();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
}

void MainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);

    if (!CreateDeviceResources()) {
        EndPaint(m_hwnd, &ps);
        return;
    }

    m_pRenderTarget->BeginDraw();
    
    // Clear with dark background
    m_pRenderTarget->Clear(D2D1::ColorF(0.1f, 0.1f, 0.15f, 1.0f));

    // Draw camera view
    if (m_pCameraView) {
        m_pCameraView->Draw(m_pRenderTarget, m_cameraRect);
    }

    // Draw animated menu
    if (m_pAnimatedMenu) {
        m_pAnimatedMenu->Draw(m_pRenderTarget, D2D1::RectF(0, 0, (float)m_clientWidth, (float)m_clientHeight));
    }

#ifdef _DEBUG
    // Update debug overlay
    if (m_pDebugOverlay && m_pCameraView) {
        m_pDebugOverlay->UpdateCameraStats(
            m_pCameraView->GetResolution(),
            m_pCameraView->GetFPS(),
            m_pCameraView->GetLatency()
        );
    }
#endif

    HRESULT hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        DiscardDeviceResources();
    }

    EndPaint(m_hwnd, &ps);
}

void MainWindow::OnSize(UINT width, UINT height)
{
    m_clientWidth = width;
    m_clientHeight = height;

    if (m_pRenderTarget) {
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }

    CalculateLayout();
}

void MainWindow::OnKeyDown(WPARAM wParam)
{
    switch (wParam) {
    case 'G':
    case 'g':
        if (m_pAnimatedMenu) {
            m_pAnimatedMenu->Toggle();
        }
        break;

    case VK_ESCAPE:
        if (m_pAnimatedMenu && m_pAnimatedMenu->IsOpen()) {
            m_pAnimatedMenu->Close();
        }
        break;
    }
}

void MainWindow::OnTimer(WPARAM timerId)
{
    if (timerId == TIMER_FRAME) {
        // Update animations
        if (m_pAnimatedMenu) {
            m_pAnimatedMenu->Update();
        }

        // Update camera
        if (m_pCameraView) {
            m_pCameraView->Update();
        }

        // Force repaint
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void MainWindow::OnClose()
{
    KillTimer(m_hwnd, TIMER_FRAME);
    DestroyWindow(m_hwnd);
}

bool MainWindow::CreateDeviceResources()
{
    if (m_pRenderTarget) {
        return true;
    }

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    HRESULT hr = m_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size),
        &m_pRenderTarget
    );

    if (SUCCEEDED(hr)) {
        // Initialize camera view with render target
        if (m_pCameraView) {
            m_pCameraView->Initialize(m_pRenderTarget);
        }

        // Initialize animated menu with render target
        if (m_pAnimatedMenu) {
            m_pAnimatedMenu->Initialize(m_pRenderTarget, m_pDWriteFactory);
        }

        CalculateLayout();
    }

    return SUCCEEDED(hr);
}

void MainWindow::DiscardDeviceResources()
{
    if (m_pRenderTarget) {
        m_pRenderTarget->Release();
        m_pRenderTarget = nullptr;
    }
}

void MainWindow::CalculateLayout()
{
    if (m_clientWidth == 0 || m_clientHeight == 0) {
        return;
    }

    // Calculate camera rectangle (centered, maintaining aspect ratio)
    const float aspectRatio = 16.0f / 9.0f;
    const float margin = 20.0f;
    
    float availableWidth = (float)m_clientWidth - 2 * margin;
    float availableHeight = (float)m_clientHeight - 2 * margin;
    
    float cameraWidth, cameraHeight;
    
    if (availableWidth / availableHeight > aspectRatio) {
        // Height is the limiting factor
        cameraHeight = availableHeight;
        cameraWidth = cameraHeight * aspectRatio;
    } else {
        // Width is the limiting factor
        cameraWidth = availableWidth;
        cameraHeight = cameraWidth / aspectRatio;
    }
    
    float left = ((float)m_clientWidth - cameraWidth) / 2.0f;
    float top = ((float)m_clientHeight - cameraHeight) / 2.0f;
    
    m_cameraRect = D2D1::RectF(left, top, left + cameraWidth, top + cameraHeight);
}

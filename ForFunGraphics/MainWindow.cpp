#include "MainWindow.h"
#include "BaseWindow.h"

// Special safe release function for releasing COM interface pointers
template <class T> void SafeRelease(T** ppT)
{
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
    {
        // Initialize a timer for our graphics refresh rate
        SetTimer(m_hwnd, timerId, 10, (TIMERPROC)NULL);
        // On window creation, initialize the Direct2D factory
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))) {
            return -1;
        }
        // Also initialize a DirectWrite factory for writing text.
        if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pWriteFactory),
            reinterpret_cast<IUnknown**>(&pWriteFactory)))) {
            return -1;
        }
        // Use the WriteFactory to create a DirectWrite text format object
        if (FAILED(pWriteFactory->CreateTextFormat(
            L"Verdana",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            50,
            L"", //locale
            &pTextFormat))) {
            return -1;
        }
        // Now align the text
        pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
        pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        return 0;
    }

    case WM_DESTROY:
    {
        // Release our graphics resources
        KillTimer(m_hwnd, timerId);
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;
    }

    case WM_PAINT:
    {
        // WM_PAINT is sent when we need to paint a portion of the app window, and
        // also when UpdateWindow() and RedrawWindow() are called.
        OnPaint();
        return 0;
    }

    case WM_SIZE:
    {
        // WM_SIZE is invoked when the window changes size
        Resize();
        return 0;
    }

    case WM_TIMER:
    {
        if (wParam == timerId) {
            // timeElapsed++;
            // Uncomment this to use the timer to trigger repainting (it doesn't actually fire every 10ms)
            // OnPaint();
        }
        return 0;
    }

    case WM_KEYDOWN:
    {
        HandleKeypress(wParam);
        return 0;
    }

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}

HRESULT MainWindow::CreateGraphicsResources() {
    HRESULT hr = S_OK;

    // Only run if the render target isn't already created
    if (pRenderTarget == NULL) {
        // Get the rectangle (the struct is just coordinates) for the window's client area
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        // Now use the factory to create the render target, default target properties, and pass the window 
        // handle + size in
        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        // If that worked, use the render target to create the brush
        // We use the render target instead of the factory since render target handles device dependent resources.
        if (SUCCEEDED(hr)) {
            // Choose a color for the brush (red, green, blue, alpha) -> yellow
            // See: https://docs.microsoft.com/en-us/windows/win32/learnwin32/using-color-in-direct2d
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            // If both worked, we should update our layout (the shapes we're drawing)
            if (SUCCEEDED(hr)) {
                CalculateLayout();
            }
        }
    }

    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    // Use this special SafeRelease function
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint() {
    // The standard Direct2D render loop is: https://docs.microsoft.com/en-us/windows/win32/learnwin32/drawing-with-direct2d#the-direct2d-render-loop
    // Invoke CreateGraphicsResources() so that we can start painting
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr)) {
        // Start painting
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);

        // In Direct2D, drawing is done on the render target
        // Start Direct2D drawing
        pRenderTarget->BeginDraw();

        // Clear the render target
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        // User our brush to draw/fill the ellipse
        // Possible drawing functions: https://docs.microsoft.com/en-us/windows/win32/api/d2d1/nn-d2d1-id2d1rendertarget
        // Use the ellipse object that is part of MainWindow (and is adjusted accordingly in other parts of the code)
        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::AntiqueWhite));
        //pRenderTarget->FillEllipse(ellipse, pBrush);
        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        //pRenderTarget->DrawEllipse(ellipse, pBrush, 10);

        // Get the time of day
        SYSTEMTIME timeOfDay;
        GetLocalTime(&timeOfDay);

        // Compute FPS and display it
        D2D1_SIZE_F size = pRenderTarget->GetSize();
		TIME_MS_T currTime = (((TIME_MS_T)timeOfDay.wHour * 60 + (TIME_MS_T)timeOfDay.wMinute) * 60 + (TIME_MS_T)timeOfDay.wSecond) * 1000 + timeOfDay.wMilliseconds;
        if (startTime) {
            // Display FPS only after first frame
            TIME_MS_T delta = currTime - startTime - timeElapsed;
            fpsAverage.addData(delta);
            TIME_MS_T averageDelta = fpsAverage.getAverage();
			unsigned fps = round(1000.0 / ((double)(averageDelta)));

            // Draw the FPS resulting from the averaging
            std::wstring counterText = std::to_wstring(fps);
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
            pRenderTarget->DrawText(
                counterText.c_str(),
                counterText.length(),
                pTextFormat,
                D2D1::RectF(0, 0, size.width, size.height),
                pBrush);
            //std::cout << "OnPaint() invoked at: "
            //    << std::to_string(timeOfDay.wHour) << ":"
            //    << std::to_string(timeOfDay.wMinute) << ":"
            //    << std::to_string(timeOfDay.wSecond) << ":"
            //    << std::to_string(timeOfDay.wMilliseconds)
            //    << ". FPS is: " << fps
            //    << ". Time elapsed (ms) is: " << timeElapsed
            //    << std::endl;
        } else {
            // First frame, just record the starting time.
            startTime = currTime;
        }
        // Update the time elapsed, after the FPS calculation since we need the delta for FPS.
        timeElapsed = currTime - startTime;

        // Draw the Scene
        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
        MyScene.UpdateInTime(timeElapsed);
        MyScene.Render(pRenderTarget, pBrush);

        // End D2Direct drawing
        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
            // If there was an error (we lost the graphics device or something), we need to re-create the
            // render target and device-dependent resources
            DiscardGraphicsResources();
        }

        // Finish painting
        EndPaint(m_hwnd, &ps);

        // Trigger another repainting immediately
        // Comment this out to use the timer instead
        RedrawWindow(m_hwnd, NULL, NULL, RDW_INTERNALPAINT);
    }
}

void MainWindow::Resize() {
    // Only need to do anything if we have a valid render target
    if (pRenderTarget != NULL) {
        // Get the new size of the client area (in physical pixels)
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        // Now resize the render target to the new window size (physical pixels)
        pRenderTarget->Resize(size);

        // Also recalculate any shapes we've drawn as part of our stuff
        CalculateLayout();

        // Mark the entire region as part of the update region (forces a WM_PAINT)
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void MainWindow::CalculateLayout() {
    // Only need to do anything if we have a valid render target
    if (pRenderTarget != NULL) {
        // Get the size of the render target (in DIPs)
        D2D1_SIZE_F size = pRenderTarget->GetSize();

        // Compute the placement and radius of our ellipse (in middle, circle that fits)
        const float x = size.width / 2;
        const float y = size.height / 2;
        const float radius = min(x, y) - 5;

        // Create a new ellipse for the new size
        ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
    }
}


void MainWindow::HandleKeypress(WPARAM KeyCode) {
    switch (KeyCode) { 
    case VK_ESCAPE: {
        MyScene.SetPoVLocation({ 0., 0., 0., 1. });
        MyScene.SetPoVAngles({ 0., 0. , 0.});
        break;
    }
    case VK_UP: {
        tuple<float, float, float> Angles = MyScene.GetPoVAngles();
        get<0>(Angles) -= 5.;
        MyScene.SetPoVAngles(Angles);
        break;
    }
    case VK_DOWN: {
        tuple<float, float, float> Angles = MyScene.GetPoVAngles();
        get<0>(Angles) += 5.;
        MyScene.SetPoVAngles(Angles);
        break;
    }
    case VK_LEFT: {
        tuple<float, float, float> Angles = MyScene.GetPoVAngles();
        get<1>(Angles) -= 5;
        MyScene.SetPoVAngles(Angles);
        break;
    }
    case VK_RIGHT: {
        tuple<float, float, float> Angles = MyScene.GetPoVAngles();
        get<1>(Angles) += 5;
        MyScene.SetPoVAngles(Angles);
        break;
    }
    case VK_SPACE: {
        MyScene.StepPoV({0., 1., 0., 0.});
        break;
    }
    case VK_CONTROL: {
        MyScene.StepPoV({ 0., -1., 0., 0. });
        break;
    }
    case (VK_LETTER_BASE + 'W' - '@'): {
        MyScene.StepPoV({ 0., 0., -1., 0. });
        break;
    }
    case (VK_LETTER_BASE + 'S' - '@'): {
        MyScene.StepPoV({ 0., 0., 1., 0. });
        break;
    }
    case (VK_LETTER_BASE + 'A' - '@'): {
        MyScene.StepPoV({ -1., 0., 0., 0. });
        break;
    }
    case (VK_LETTER_BASE + 'D' - '@'): {
        MyScene.StepPoV({ 1., 0., 0., 0. });
        break;
    }
    default:
        std::cout << "Unknown keycode: " << std::hex << KeyCode << std::endl;
    }
};
#include <ds/spritewnd.h>
#include <ds/framework.h>

#include <ds/utils.h>
#include <spine/spine.h>


using namespace Gdiplus;

// [XY](3, 2) = [UV](3, 3) @ M(3, 2)
// | x1, y1 |   | u1, v1, 1 |   | m11, m12 |
// | x2, y2 | = | u2, v2, 1 | @ | m21, m22 |
// | x3, y3 |   | u3, v3, 1 |   | dx,  dy  |
void GetAffineMatrix(
    double x1, double y1,
    double x2, double y2,
    double x3, double y3,
    double u1, double v1,
    double u2, double v2,
    double u3, double v3,
    Matrix* m
)
{
    double x12 = x1 - x2;
    double y12 = y1 - y2;
    double x23 = x2 - x3;
    double y23 = y2 - y3;
    double u12 = u1 - u2;
    double v12 = v1 - v2;
    double u23 = u2 - u3;
    double v23 = v2 - v3;

    double invdet = 1.0000 / (u12 * v23 - v12 * u23);
    double m11    = invdet * (x12 * v23 - v12 * x23);
    double m12    = invdet * (y12 * v23 - v12 * y23);
    double m21    = invdet * (u12 * x23 - x12 * u23);
    double m22    = invdet * (u12 * y23 - y12 * u23);
    double dx     = x1 - m11 * u1 - m21 * v1;
    double dy     = y1 - m12 * u1 - m22 * v1;

    m->SetElements((REAL)m11, (REAL)m12, (REAL)m21, (REAL)m22, (REAL)dx, (REAL)dy);
}
///////////////////////////// Message Process ////////////////////////////////

LRESULT SpriteWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return this->OnCreate(wParam, lParam);
    case WM_DESTROY:
        return this->OnDestroy(wParam, lParam);
    case WM_CLOSE:
        return this->OnClose(wParam, lParam);
    case WM_PAINT:
        return this->OnPaint(wParam, lParam);
    case WM_CONTEXTMENU:
        return this->OnContextMenu(wParam, lParam);
    case WM_COMMAND:
        return this->OnCommand(wParam, lParam);
    case WM_TIMER:
        return this->OnTimer(wParam, lParam);
    case WM_INITMENUPOPUP:
        return this->OnInitMenuPopup(wParam, lParam);
    case WM_MOUSEMOVE:
        return this->OnMouseMove(wParam, lParam);
    case WM_LBUTTONDOWN:
        return this->OnLButtonDown(wParam, lParam);
    case WM_LBUTTONUP:
        return this->OnLButtonUp(wParam, lParam);
    default:
        return DefWindowProcW(this->hWnd, uMsg, wParam, lParam);
    }
}

LRESULT SpriteWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
    //SetLayeredWindowAttributes(pWndData->hWnd, RGB(0, 255, 0), 255, LWA_COLORKEY);
    return 0;
}

LRESULT SpriteWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT SpriteWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
    // 屏蔽普通退出, 只能通过菜单退出
    DestroyWindow(this->hWnd);
    return 0;
}

LRESULT SpriteWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
    // 开始绘图
    PAINTSTRUCT ps = { 0 };
    BeginPaint(this->hWnd, &ps);
    EndPaint(this->hWnd, &ps);
    auto p1 = L"D:\\ACGN\\C\\illusts\\AzurLane\\skin\\IllustriousTea%20Party.png";
    auto p2 = L"D:\\ACGN\\C\\illusts\\堤娅\\99633552_p0.png";
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(p1, false);
    //if (!bitmap) {
    //    std::cout << "Failed to load the image" << std::endl;
    //    return;
    //}

    int width = bitmap->GetWidth();
    int height = bitmap->GetHeight();

    //SetWindowPos(pWndData->hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemory = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemory, hBitmap);

    Gdiplus::Graphics graphics(hdcMemory);
    TextureBrush brush(bitmap, WrapModeClamp);
    Matrix m(0.45, 0, 0, 0.45, 0, 0);
    brush.SetTransform(&m);
    Gdiplus::Status status = graphics.FillRectangle(&brush, RectF(0, 0, width, height));
        //graphics.DrawImage(bitmap, 0, 0, width, height);

    //if (status != Gdiplus::Ok) {
    //    std::cout << "Failed to draw the image" << std::endl;
    //    return;
    //}

    BLENDFUNCTION blend = { 
        AC_SRC_OVER,
        0,
        255,
        AC_SRC_ALPHA
    };

    POINT ptSrc = { 0, 0 };
    POINT ptWnd = { 0, 0 };
    SIZE sizeWnd = { width, height };

    BOOL result = UpdateLayeredWindow(
        this->hWnd, 
        hdcScreen, 
        0, &sizeWnd, 
        hdcMemory, 
        &ptSrc, 
        RGB(0, 255, 0),
        &blend, 
        ULW_ALPHA | ULW_COLORKEY
    );
    //if (!result) {
    //    std::cout << "Failed to update the layered window" << std::endl;
    //}

    SelectObject(hdcMemory, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMemory);
    ReleaseDC(NULL, hdcScreen);
    delete bitmap;
    return 0;
}

LRESULT SpriteWindow::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT SpriteWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT SpriteWindow::OnTimer(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT SpriteWindow::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT SpriteWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (wParam & MK_LBUTTON)
    {
        POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        RECT rcWnd;
        GetWindowRect(this->hWnd, &rcWnd);
        SetWindowPos(
            this->hWnd, HWND_TOPMOST,
            rcWnd.left + (ptCursor.x - this->ptDragSrc.x),
            rcWnd.top + (ptCursor.y - this->ptDragSrc.y),
            0, 0,
            SWP_SHOWWINDOW | SWP_NOSIZE
        );
    }
    return 0;
}

LRESULT SpriteWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    SetCapture(this->hWnd); // 防止鼠标跟丢

    // 保存点击位置
    this->ptDragSrc.x = GET_X_LPARAM(lParam);
    this->ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
}

LRESULT SpriteWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    ReleaseCapture();

    // TODO
    return DefWindowProcW(this->hWnd, WM_LBUTTONUP, wParam, lParam);
}



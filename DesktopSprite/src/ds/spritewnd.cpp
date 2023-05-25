#include <ds/spritewnd.h>
#include <ds/framework.h>

#include <ds/utils.h>

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

    // 加载 spine
    this->spinechar = new SpineChar(this->hWnd);
    this->spinechar->CreateTargetResourcse();
    this->spinechar->LoadSpine(
        "D:\\ACGN\\AzurLane_Export\\spines\\guanghui_2\\guanghui_2.atlas",
        "D:\\ACGN\\AzurLane_Export\\spines\\guanghui_2\\guanghui_2.skel"
    );

    SetWindowPos(this->hWnd, HWND_TOPMOST, 1000, 600, 700, 700, SWP_SHOWWINDOW);

    this->spinechar->Start();
    return 0;
}

LRESULT SpriteWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    this->spinechar->Stop();
    this->spinechar->UnloadSpine();
    this->spinechar->ReleaseTargetResources();
    delete this->spinechar;
    this->spinechar = NULL;
    return 0;
}

LRESULT SpriteWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
    // 屏蔽普通退出, 只能通过菜单退出
    return 0;
}

LRESULT SpriteWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps = { 0 };
    BeginPaint(this->hWnd, &ps);
    EndPaint(this->hWnd, &ps);
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

    this->spinechar->Lock();
    this->spinechar->SendAction(SpineAction::TOUCH);
    this->spinechar->Unlock();

    return DefWindowProcW(this->hWnd, WM_LBUTTONUP, wParam, lParam);
}



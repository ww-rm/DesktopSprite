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
    case WM_LBUTTONDBLCLK:
        return this->OnLButtonDBClick(wParam, lParam);
    case WM_MOUSEWHEEL:
        return this->OnMouseWheel(wParam, lParam);
    default:
        return DefWindowProcW(this->hWnd, uMsg, wParam, lParam);
    }
}

LRESULT SpriteWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
    // ���� spine
    this->spinechar = new SpineChar(this->hWnd);
    this->spinechar->CreateTargetResourcse();
    this->spinechar->LoadSpine(
        "D:\\ACGN\\AzurLane_Export\\spines\\lafei_4\\lafei_4.atlas",
        "D:\\ACGN\\AzurLane_Export\\spines\\lafei_4\\lafei_4.skel"
    );

    SetWindowPos(this->hWnd, HWND_TOPMOST, 1000, 600, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

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
    // ������ͨ�˳�, ֻ��ͨ���˵��˳�
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
    if (this->isDragging && (wParam & MK_LBUTTON))
    {
        POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        RECT rcWnd = { 0 };
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
    SetCapture(this->hWnd); // ��ֹ������

    // ������λ��
    this->ptDragSrc.x = GET_X_LPARAM(lParam);
    this->ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
}

LRESULT SpriteWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    ReleaseCapture();

    if (this->isDragging)
    {
        this->isDragging = FALSE;
        this->spinechar->Lock();
        this->spinechar->SendAction(SpineAction::DRAGDOWN);
        this->spinechar->Unlock();
    }
    else
    {
        this->spinechar->Lock();
        this->spinechar->SendAction(SpineAction::TOUCH);
        this->spinechar->Unlock();
    }

    return 0;
}

LRESULT SpriteWindow::OnLButtonDBClick(WPARAM wParam, LPARAM lParam)
{
    SetCapture(this->hWnd); // ��ֹ������

    // ������λ��
    this->ptDragSrc.x = GET_X_LPARAM(lParam);
    this->ptDragSrc.y = GET_Y_LPARAM(lParam);

    if (!this->isDragging)
    {
        this->isDragging = TRUE;
        this->spinechar->Lock();
        this->spinechar->SendAction(SpineAction::DRAGUP);
        this->spinechar->Unlock();
    }

    return 0;
}

LRESULT SpriteWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
    SHORT zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

    if (zDelta >= 0)
    {
        this->spinechar->Lock();
        this->spinechar->SendAction(SpineAction::WINK);
        this->spinechar->Unlock();
    }
    else
    {
        this->spinechar->Lock();
        this->spinechar->SendAction(SpineAction::DANCE);
        this->spinechar->Unlock();
    }

    return 0;
}

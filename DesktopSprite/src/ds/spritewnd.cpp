#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/winapp.h>

#include <ds/spritewnd.h>

SpriteWindow::SpriteWindow() {}

PCWSTR SpriteWindow::GetClassName_() const
{
    return L"DesktopSpriteSpriteWndClass";
}

SpineChar* SpriteWindow::GetSpineChar()
{
    return this->spinechar;
}

BOOL SpriteWindow::LoadLastPosFromReg(FLOAT* x, FLOAT* y)
{
    // Ĭ�Ͼ���λ����ԭ��
    *x = 0;
    *y = 0;

    // ��ע�����
    HKEY hkApp = NULL;
    DWORD dwDisposition = 0;
    DWORD cbData = 0;

    WCHAR subkey[128] = { 0 };
    if (FAILED(StringCchPrintfW(subkey, 128, L"SOFTWARE\\%s", WinApp::GetName())))
    {
        return FALSE;
    }

    if (RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, &dwDisposition))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    cbData = sizeof(FLOAT);
    RegQueryAnyValue(hkApp, L"LastSpritePosX", (PBYTE)x, &cbData);
    RegCloseKey(hkApp);

    return TRUE;
}

BOOL SpriteWindow::SaveCurrentPosToReg()
{
    RECT currentWndRc = { 0 };
    if (!GetWindowRect(this->hWnd, &currentWndRc))
    {
        return FALSE;
    }

    // ��ע�����
    HKEY hkApp = NULL;
    DWORD dwDisposition = 0;
    DWORD cbData = 0;

    WCHAR subkey[128] = { 0 };
    StringCchPrintfW(subkey, 128, L"SOFTWARE\\%s", WinApp::GetName());

    if (RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, &dwDisposition))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    RegSetBinValue(hkApp, L"LastSpritePos", (PBYTE)&currentWndRc, sizeof(POINT));
    RegCloseKey(hkApp);
    return TRUE;
}

BOOL SpriteWindow::LoadFlipXFromReg(BOOL* flip)
{
    // Ĭ�Ϸ�ת, Ҳ���ǳ������
    *flip = TRUE;

    // ��ע�����
    HKEY hkApp = NULL;
    DWORD dwDisposition = 0;
    DWORD cbData = 0;

    WCHAR subkey[128] = { 0 };
    if (FAILED(StringCchPrintfW(subkey, 128, L"SOFTWARE\\%s", WinApp::GetName())))
    {
        return FALSE;
    }

    if (RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, &dwDisposition))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    cbData = sizeof(BOOL);
    RegQueryAnyValue(hkApp, L"LastSpriteFlipX", (PBYTE)&flip, &cbData);
    RegCloseKey(hkApp);

    return TRUE;
}

BOOL SpriteWindow::SaveFlipXFromReg(BOOL flip)
{
    // ��ע�����
    HKEY hkApp = NULL;
    DWORD dwDisposition = 0;
    DWORD cbData = 0;

    WCHAR subkey[128] = { 0 };
    StringCchPrintfW(subkey, 128, L"SOFTWARE\\%s", WinApp::GetName());

    if (RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, &dwDisposition))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    RegSetBinValue(hkApp, L"LastSpritePos", (PBYTE)&flip, sizeof(BOOL));
    RegCloseKey(hkApp);
    return TRUE;
}

BOOL SpriteWindow::ApplyConfig(const AppConfig::AppConfig* newConfig)
{
    const AppConfig::AppConfig* currentConfig = AppConfig::Get();
    if (!newConfig)
    {
        newConfig = currentConfig;
    }
    BOOL isNew = (newConfig != currentConfig);

    // ��������
    if (!isNew || newConfig->bShowSprite != currentConfig->bShowSprite)
    {
        ShowWindow(this->hWnd, newConfig->bShowSprite ? SW_SHOWNA : SW_HIDE);
    }
    // TODO here

    // spine ���, ÿ������Ҫ�������в���
    if (!isNew || StrCmpW(newConfig->szBalloonIconPath, currentConfig->szBalloonIconPath))
    {
        //DestroyIcon(this->balloonIcon);
        //Bitmap(newConfig->szBalloonIconPath).GetHICON(&this->balloonIcon);

        //if (isNew)
        //{
        //    this->pNotifyIcon->PopupIconInfo(L"ͼ���޸ĳɹ�", L"������Ч����~", this->balloonIcon, TRUE);
        //}
    }
    return TRUE;
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
    GetSysDragSize(&this->sysDragSize);

    // ��ʼ����С, λ��, Z ��, ���ǻ�����ʾ
    SIZE wndSize = { 1920, 1080 };
    GetScreenResolution(&wndSize);
    SetWindowPos(this->hWnd, HWND_TOPMOST, 0, 0, wndSize.cx, wndSize.cy, SWP_HIDEWINDOW);

    // Ӧ��ȫ������
    this->ApplyConfig();

    // ���� spine
    this->spinechar = new SpineChar();
    this->spinerenderer = new SpineRenderer(this->hWnd, this->spinechar);
    this->spinechar->LoadSpine(
        L"D:\\ACGN\\AzurLane_Export\\spines\\lafei_4\\lafei_4.atlas",
        L"D:\\ACGN\\AzurLane_Export\\spines\\lafei_4\\lafei_4.skel",
        100
    );

    FLOAT spX = 0;
    FLOAT spY = 0;
    this->LoadLastPosFromReg(&spX, &spY);
    //this->spinechar->SetPosition(spX, spY);

    this->spinerenderer->CreateTargetResourcse();
    this->spinerenderer->CreateSpineResources();
    this->spinerenderer->Start();
    return 0;
}

LRESULT SpriteWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    this->spinerenderer->Stop();
    this->spinerenderer->ReleaseSpineResources();
    this->spinerenderer->ReleaseTargetResources();
    this->spinechar->UnloadSpine();
    delete this->spinerenderer;
    this->spinerenderer = NULL;
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
    POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    INT deltaX = ptCursor.x - this->ptDragSrc.x;
    INT deltaY = ptCursor.y - this->ptDragSrc.y;
    RECT rcWnd = { 0 };

    if (wParam & MK_LBUTTON)
    {
        // �ж��Ƿ񳬹��϶���ֵ, �������϶�
        if (!this->isDragging && (abs(deltaX) >= this->sysDragSize.cx || abs(deltaY) >= this->sysDragSize.cy))
        {
            this->isDragging = TRUE;
            this->spinerenderer->Lock();
            this->spinechar->SendAction(SpineAction::DRAGUP);
            this->spinerenderer->Unlock();
        }

        if (this->isDragging)
        {
            GetWindowRect(this->hWnd, &rcWnd);
            SetWindowPos(this->hWnd, 0, rcWnd.left + deltaX, rcWnd.top + deltaY, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
        }
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
        this->spinerenderer->Lock();
        this->spinechar->SendAction(SpineAction::DRAGDOWN);
        this->spinerenderer->Unlock();

        // ����һ�����ڵĴ���λ��
        this->SaveCurrentPosToReg();
    }
    else
    {
        this->spinerenderer->Lock();
        this->spinechar->SendAction(SpineAction::TOUCH);
        this->spinerenderer->Unlock();
    }

    return 0;
}

LRESULT SpriteWindow::OnLButtonDBClick(WPARAM wParam, LPARAM lParam)
{
    SetCapture(this->hWnd); // ��ֹ������

    // ������λ��
    this->ptDragSrc.x = GET_X_LPARAM(lParam);
    this->ptDragSrc.y = GET_Y_LPARAM(lParam);

    // TODO: ת��

    return 0;
}

LRESULT SpriteWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
    SHORT zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

    if (zDelta >= 0)
    {
        this->spinerenderer->Lock();
        this->spinechar->SendAction(SpineAction::WINK);
        this->spinerenderer->Unlock();
    }
    else
    {
        this->spinerenderer->Lock();
        this->spinechar->SendAction(SpineAction::DANCE);
        this->spinerenderer->Unlock();
    }

    return 0;
}

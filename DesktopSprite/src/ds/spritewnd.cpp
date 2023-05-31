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
    // Ĭ�Ͼ���λ���Ǵ�������
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
    cbData = sizeof(FLOAT);
    RegQueryAnyValue(hkApp, L"LastSpritePosY", (PBYTE)y, &cbData);
    RegCloseKey(hkApp);

    return TRUE;
}

BOOL SpriteWindow::SaveCurrentPosToReg()
{
    FLOAT x = 0;
    FLOAT y = 0;
    this->spinechar->GetPosition(&x, &y);

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

    RegSetBinValue(hkApp, L"LastSpritePosX", (PBYTE)&x, sizeof(FLOAT));
    RegSetBinValue(hkApp, L"LastSpritePosY", (PBYTE)&y, sizeof(FLOAT));
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

BOOL SpriteWindow::SaveFlipXToReg()
{
    BOOL flip = TRUE;
    this->spinechar->GetFlipX(&flip);

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
    if (!isNew || newConfig->bSpriteMousePass != currentConfig->bSpriteMousePass)
    {
        LONG_PTR exStyle = GetWindowLongPtrW(this->hWnd, GWL_EXSTYLE);
        if (newConfig->bSpriteMousePass)
        {
            exStyle |= WS_EX_TRANSPARENT;
        }
        else
        {
            exStyle &= ~WS_EX_TRANSPARENT;
        }
        SetWindowLongPtrW(this->hWnd, GWL_EXSTYLE, exStyle);
    }

    // spine ���, ÿ������Ҫ�������в���
    if (!isNew || 
        StrCmpW(newConfig->szSpineAtlasPath, currentConfig->szSpineAtlasPath) || 
        StrCmpW(newConfig->szSpineSkelPath, currentConfig->szSpineSkelPath) ||
        newConfig->spScale != currentConfig->spScale)
    {
        if (!this->spinechar->LoadSpine(newConfig->szSpineAtlasPath, newConfig->szSpineSkelPath, newConfig->spScale) ||
            !this->spinerenderer->CreateSpineResources() ||
            !this->spinerenderer->Start())
        {
            this->spinerenderer->Stop();
            this->spinerenderer->ReleaseSpineResources();
            this->spinechar->UnloadSpine();
            MessageBoxW(this->hWnd, L"spine ��Դ����ʧ�ܣ�", L"���ô���", MB_ICONINFORMATION);
        }

    }
    if (!isNew || newConfig->maxFps != currentConfig->maxFps)
    {
        this->spinerenderer->Lock();
        this->spinerenderer->SetMaxFps(newConfig->maxFps);
        this->spinerenderer->Unlock();
    }
    if (!isNew || newConfig->spTransparencyPercent != currentConfig->spTransparencyPercent)
    {
        this->spinerenderer->Lock();
        this->spinerenderer->SetTransparency(newConfig->spTransparencyPercent);
        this->spinerenderer->Unlock();
    }
    if (!isNew ||
        StrCmpW(newConfig->spAnimeIdle, currentConfig->spAnimeIdle) ||
        StrCmpW(newConfig->spAnimeDrag, currentConfig->spAnimeDrag) ||
        StrCmpW(newConfig->spAnimeWork, currentConfig->spAnimeWork) ||
        StrCmpW(newConfig->spAnimeSleep, currentConfig->spAnimeSleep) ||
        StrCmpW(newConfig->spAnimeStand, currentConfig->spAnimeStand) ||
        StrCmpW(newConfig->spAnimeTouch, currentConfig->spAnimeTouch) ||
        StrCmpW(newConfig->spAnimeWink, currentConfig->spAnimeWink) ||
        StrCmpW(newConfig->spAnimeVictory, currentConfig->spAnimeVictory) ||
        StrCmpW(newConfig->spAnimeDance, currentConfig->spAnimeDance) ||
        StrCmpW(newConfig->spAnimeDizzy, currentConfig->spAnimeDizzy))
    {
        this->spinerenderer->Lock();
        this->spinechar->SetAnimeName(SpineAnime::IDLE, newConfig->spAnimeIdle);
        this->spinechar->SetAnimeName(SpineAnime::DRAG, newConfig->spAnimeDrag);
        this->spinechar->SetAnimeName(SpineAnime::WORK, newConfig->spAnimeWork);
        this->spinechar->SetAnimeName(SpineAnime::SLEEP, newConfig->spAnimeSleep);

        this->spinechar->SetAnimeName(SpineAnime::STAND, newConfig->spAnimeStand);
        this->spinechar->SetAnimeName(SpineAnime::TOUCH, newConfig->spAnimeTouch);
        this->spinechar->SetAnimeName(SpineAnime::WINK, newConfig->spAnimeWink);
        this->spinechar->SetAnimeName(SpineAnime::VICTORY, newConfig->spAnimeVictory);
        this->spinechar->SetAnimeName(SpineAnime::DANCE, newConfig->spAnimeDance);
        this->spinechar->SetAnimeName(SpineAnime::DIZZY, newConfig->spAnimeDizzy);
        this->spinechar->SendAction(SpineAction::REFRESH); // refresh to show the new anime
        this->spinerenderer->Unlock();
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

    // ���� spine
    this->spinechar = new SpineChar();
    this->spinerenderer = new SpineRenderer(this->hWnd, this->spinechar);

    // ��ͼ������Դ���봴���ɹ�
    if (!this->spinerenderer->CreateTargetResourcse())
    {
        return -1;
    }

    // Ӧ��ȫ������
    this->ApplyConfig();

    // ��ע����ʼ�� sprite λ�úͳ���
    FLOAT x = 0;
    FLOAT y = 0;
    BOOL flipX = TRUE;
    this->LoadLastPosFromReg(&x, &y);
    this->LoadFlipXFromReg(&flipX);

    this->spinerenderer->Lock();
    this->spinechar->SetPosition(x, y);
    this->spinechar->SetFlipX(flipX);
    this->spinerenderer->Unlock();

    return 0;
}

LRESULT SpriteWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    this->SaveCurrentPosToReg();
    this->SaveFlipXToReg();

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

    this->spinerenderer->Lock();
    BOOL flipX = TRUE;
    this->spinechar->GetFlipX(&flipX);
    flipX = (BOOL)!flipX;
    this->spinechar->SetFlipX(flipX);
    this->SaveFlipXToReg();
    this->spinerenderer->Unlock();

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

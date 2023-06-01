#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/winapp.h>
#include <resource.h>

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

BOOL SpriteWindow::LoadLastPosFromReg(POINT* pt)
{
    // Ĭ�Ͼ���λ���Ǵ�������
    pt->x = 0;
    pt->y = 0;

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

    cbData = sizeof(POINT);
    RegQueryAnyValue(hkApp, L"LastSpritePos", (PBYTE)pt, &cbData);
    RegCloseKey(hkApp);

    return TRUE;
}

BOOL SpriteWindow::SaveCurrentPosToReg()
{
    FLOAT x = 0;
    FLOAT y = 0;
    this->spinechar->GetPosition(&x, &y);
    POINT pt = { (LONG)x, (LONG)y };

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

    RegSetBinValue(hkApp, L"LastSpritePos", (PBYTE)&pt, sizeof(POINT));
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
    RegQueryAnyValue(hkApp, L"LastSpriteFlipX", (PBYTE)flip, &cbData);
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

    RegSetBinValue(hkApp, L"LastSpriteFlipX", (PBYTE)&flip, sizeof(BOOL));
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
        // ��ж������ spine ��Դ
        this->spinerenderer->Stop();
        this->spinerenderer->ReleaseSpineResources();
        this->spinechar->UnloadSpine();

        if (this->spinechar->LoadSpine(newConfig->szSpineAtlasPath, newConfig->szSpineSkelPath, newConfig->spScale) &&
            this->spinerenderer->CreateSpineResources())
        {
            // ��ע����ʼ�� sprite λ�úͳ���
            POINT pos = { 0 };
            BOOL flipX = TRUE;
            this->LoadLastPosFromReg(&pos);
            this->LoadFlipXFromReg(&flipX);
            this->spinechar->SetPosition((FLOAT)pos.x, (FLOAT)pos.y);
            this->spinechar->SetFlipX(flipX);
            this->spinerenderer->Start();
        }
        else
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

BOOL SpriteWindow::ShowContextMenu(INT x, INT y)
{
    // ����ContextMenu
    HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_SPCONTEXTMENU));
    HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

    // ����ڲ˵��ⵥ������˵�����ʧ������
    SetForegroundWindow(this->hWnd);

    // ��ʾ�˵�
    TrackPopupMenuEx(hContextMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, x, y, this->hWnd, NULL);

    DestroyMenu(hContextMenuBar);

    return TRUE;;
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
    this->ShowContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    return 0;
}

LRESULT SpriteWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
    AppConfig::AppConfig* pcfgdata = new AppConfig::AppConfig(*AppConfig::Get());
    switch (LOWORD(wParam))
    {
    case IDM_SHOWSPRITE:
        pcfgdata->bShowSprite = (BOOL)!pcfgdata->bShowSprite;
        break;
    case IDM_SPMOUSEPASS:
        pcfgdata->bSpriteMousePass = (BOOL)!pcfgdata->bSpriteMousePass;
        break;
    default:
        break;
    }
    this->ApplyConfig(pcfgdata);
    AppConfig::Set(pcfgdata);
    AppConfig::SaveToFile(WinApp::GetConfigPath());
    delete pcfgdata;
    return 0;
}

LRESULT SpriteWindow::OnTimer(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT SpriteWindow::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
    // ��ȡ�˵����
    HMENU hMenu = (HMENU)wParam;
    SetMenuItemState(hMenu, IDM_SHOWSPRITE, FALSE, AppConfig::Get()->bShowSprite ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_SPMOUSEPASS, FALSE, AppConfig::Get()->bSpriteMousePass ? MFS_CHECKED : MFS_UNCHECKED);
    return 0;
}

LRESULT SpriteWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    INT deltaX = ptCursor.x - this->ptDragSrc.x;
    INT deltaY = -(ptCursor.y - this->ptDragSrc.y); // ��Ϊ�� y ��Գ���

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
            // NOTE: �˴����� Lock/Unlock, ��Ϊʵ���е㿨
            this->spinechar->SetPosition((FLOAT)(this->ptSpriteDragSrc.x + deltaX), (FLOAT)(this->ptSpriteDragSrc.y + deltaY));
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

    // ���澫��λ��
    FLOAT x = 0;
    FLOAT y = 0;
    this->spinechar->GetPosition(&x, &y);
    this->ptSpriteDragSrc.x = (LONG)x;
    this->ptSpriteDragSrc.y = (LONG)y;
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

        // ����һ�����ڵľ���λ��
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

    // ���澫��λ��
    FLOAT x = 0;
    FLOAT y = 0;
    this->spinechar->GetPosition(&x, &y);
    this->ptSpriteDragSrc.x = (LONG)x;
    this->ptSpriteDragSrc.y = (LONG)y;

    // ת��
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

    if (zDelta <= 0)
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

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
    // 默认精灵位置是窗口中心
    *x = 0;
    *y = 0;

    // 打开注册表项
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

    // 打开注册表项
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
    // 默认翻转, 也就是朝向左侧
    *flip = TRUE;

    // 打开注册表项
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

    // 打开注册表项
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

    // 窗口设置
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

    // spine 相关, 每个都需要上锁进行操作
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
            MessageBoxW(this->hWnd, L"spine 资源加载失败！", L"设置错误", MB_ICONINFORMATION);
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

    // 初始化大小, 位置, Z 序, 但是还不显示
    SIZE wndSize = { 1920, 1080 };
    GetScreenResolution(&wndSize);
    SetWindowPos(this->hWnd, HWND_TOPMOST, 0, 0, wndSize.cx, wndSize.cy, SWP_HIDEWINDOW);

    // 加载 spine
    this->spinechar = new SpineChar();
    this->spinerenderer = new SpineRenderer(this->hWnd, this->spinechar);

    // 绘图基础资源必须创建成功
    if (!this->spinerenderer->CreateTargetResourcse())
    {
        return -1;
    }

    // 应用全局设置
    this->ApplyConfig();

    // 从注册表初始化 sprite 位置和朝向
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
    POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    INT deltaX = ptCursor.x - this->ptDragSrc.x;
    INT deltaY = ptCursor.y - this->ptDragSrc.y;
    RECT rcWnd = { 0 };

    if (wParam & MK_LBUTTON)
    {
        // 判定是否超过拖动阈值, 避免误拖动
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
    SetCapture(this->hWnd); // 防止鼠标跟丢

    // 保存点击位置
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

        // 保存一次现在的窗口位置
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
    SetCapture(this->hWnd); // 防止鼠标跟丢

    // 保存点击位置
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

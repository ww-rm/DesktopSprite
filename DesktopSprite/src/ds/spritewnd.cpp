#include <ds/framework.h>
#include <resource.h>
#include <ds/utils.h>
#include <ds/perfmonitor.h>
#include <ds/winapp.h>

#include <ds/spritewnd.h>

static const INT IDT_CHECKCPUHEALTH = 1;
static const UINT CHECKCPUHEALTH_INTERVAL = 1000;
static const FLOAT ANIME_STAND_L = 20;
static const FLOAT ANIME_STAND_R = 60;
static const FLOAT ANIME_STAND_K = 0.8f / (ANIME_STAND_R - ANIME_STAND_L);
static const FLOAT ANIME_STAND_B = 0.1f - ANIME_STAND_K * ANIME_STAND_L;
static const FLOAT ANIME_DIZZY_L = 10;
static const FLOAT ANIME_DIZZY_R = 60;
static const FLOAT ANIME_DIZZY_K = 0.8f / (ANIME_DIZZY_R - ANIME_DIZZY_L);
static const FLOAT ANIME_DIZZY_B = 0.1f - ANIME_DIZZY_K * ANIME_DIZZY_L;

static const INT IDT_CHECKLASTINPUT = 2;
static const INT MAX_LASTINPUTINTERVAL = 10 * 60 * 1000; // 10 minutes

SpriteWindow::SpriteWindow() 
{
    this->rndDev = new std::random_device();
    this->rndEng = new std::mt19937((*this->rndDev)());
    this->uniformRnd = new std::uniform_real_distribution<float>(0, 1);
}

SpriteWindow::~SpriteWindow() 
{
    delete this->uniformRnd;
    delete this->rndEng;
    delete this->rndDev;
}

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
    // 默认精灵位置是窗口中心
    pt->x = 0;
    pt->y = 0;

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

    RegSetBinValue(hkApp, L"LastSpritePos", (PBYTE)&pt, sizeof(POINT));
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
    RegQueryAnyValue(hkApp, L"LastSpriteFlipX", (PBYTE)flip, &cbData);
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
        // 先卸载所有 spine 资源
        this->spinerenderer->Stop();
        this->spinerenderer->ReleaseSpineResources();
        this->spinechar->UnloadSpine();

        if (this->spinechar->LoadSpine(newConfig->szSpineAtlasPath, newConfig->szSpineSkelPath, newConfig->spScale) &&
            this->spinerenderer->CreateSpineResources())
        {
            // 从注册表初始化 sprite 位置和朝向
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

BOOL SpriteWindow::ShowContextMenu(INT x, INT y)
{
    // 加载ContextMenu
    HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_SPCONTEXTMENU));
    HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

    // 解决在菜单外单击左键菜单不消失的问题
    SetForegroundWindow(this->hWnd);

    // 显示菜单
    TrackPopupMenuEx(hContextMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, x, y, this->hWnd, NULL);

    DestroyMenu(hContextMenuBar);

    return TRUE;;
}

BOOL SpriteWindow::SendFreeOrBusy()
{
    if (this->cpuHealthState >= (INT)ANIME_STAND_R)
    {
        this->spinerenderer->Lock();
        this->spinechar->SendAction(SpineAction::STAND);
        this->spinerenderer->Unlock();
        this->cpuHealthState = 0;
    }
    else if (this->cpuHealthState >= (INT)ANIME_STAND_L)
    {
        if ((*this->uniformRnd)(*this->rndEng) <= (ANIME_STAND_K * this->cpuHealthState + ANIME_STAND_B))
        {
            this->spinerenderer->Lock();
            this->spinechar->SendAction(SpineAction::STAND);
            this->spinerenderer->Unlock();
            this->cpuHealthState = 0;
        }
    }
    else if (this->cpuHealthState > -(INT)ANIME_DIZZY_L)
    {
        return TRUE;
    }
    else if (this->cpuHealthState > -(INT)ANIME_DIZZY_R)
    {
        if ((*this->uniformRnd)(*this->rndEng) <= (ANIME_DIZZY_K * (-this->cpuHealthState) + ANIME_DIZZY_B))
        {
            this->spinerenderer->Lock();
            this->spinechar->SendAction(SpineAction::DIZZY);
            this->spinerenderer->Unlock();
            this->cpuHealthState = 0;
        }
    }
    else
    {
        this->spinerenderer->Lock();
        this->spinechar->SendAction(SpineAction::DIZZY);
        this->spinerenderer->Unlock();
        this->cpuHealthState = 0;
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
    case WM_DISPLAYCHANGE:
        return this->OnDisplayChange(wParam, lParam);
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
    case WM_PERFDATAUPDATED:
        return this->OnPerfDataUpdated(wParam, lParam);
    default:
        return DefWindowProcW(this->hWnd, uMsg, wParam, lParam);
    }
}

LRESULT SpriteWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
    GetSysDragSize(&this->sysDragSize);

    // 注册监视器
    PerfMonitor::RegisterMessage(this->hWnd, WM_PERFDATAUPDATED);

    // 初始化大小, 位置, Z 序, 但是还不显示
    SIZE wndSize = { 1920, 1080 };
    GetScreenResolution(&wndSize);
    SetWindowPos(this->hWnd, HWND_TOPMOST, 1, 1, wndSize.cx - 1, wndSize.cy - 1, SWP_HIDEWINDOW); // 防止全屏检测, 边缘都少一个像素

    // 加载 spine
    this->spinechar = new SpineChar();
    this->spinerenderer = new SpineRenderer(this->hWnd, this->spinechar);

    // 绘图基础资源必须创建成功
    if (!this->spinerenderer->CreateTargetResources())
    {
        return -1;
    }

    // 应用全局设置
    this->ApplyConfig();

    SetTimer(this->hWnd, IDT_CHECKCPUHEALTH, CHECKCPUHEALTH_INTERVAL, NULL);
    SetTimer(this->hWnd, IDT_CHECKLASTINPUT, 5000, NULL);

    return 0;
}

LRESULT SpriteWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    KillTimer(this->hWnd, IDT_CHECKLASTINPUT);
    KillTimer(this->hWnd, IDT_CHECKCPUHEALTH);

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
    this->ShowContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    return 0;
}

LRESULT SpriteWindow::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
    SIZE newResolution = { LOWORD(lParam), HIWORD(lParam) };
    
    // 要先停掉渲染, 因为渲染的时候会调整窗口大小
    this->spinerenderer->Stop();
    this->spinerenderer->ReleaseSpineResources();
    this->spinechar->UnloadSpine();
    this->spinerenderer->ReleaseTargetResources();

    // 重设窗口大小
    SetWindowPos(this->hWnd, HWND_TOPMOST, 0, 0, newResolution.cx, newResolution.cy, 0);

    // 重新创建 spine 和渲染资源
    const AppConfig::AppConfig* config = AppConfig::Get();

    if (this->spinerenderer->CreateTargetResources() &&
        this->spinechar->LoadSpine(config->szSpineAtlasPath, config->szSpineSkelPath, config->spScale) &&
        this->spinerenderer->CreateSpineResources())
    {
        // 从注册表初始化 sprite 位置和朝向
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
        this->spinerenderer->ReleaseTargetResources();
        MessageBoxW(this->hWnd, L"渲染或 spine 资源加载失败！", L"发生错误", MB_ICONINFORMATION);
    }
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
    switch (wParam)
    {
    case IDT_CHECKCPUHEALTH:
        if (this->perfData.cpuPercent <= 50)
        {
            this->cpuHealthState += 1;
        }
        else
        {
            this->cpuHealthState -= 1;
        }
        this->SendFreeOrBusy();
        break;
    case IDT_CHECKLASTINPUT:
        if (GetLastInputInterval() >= MAX_LASTINPUTINTERVAL)
        {
            this->spinerenderer->Lock();
            this->spinechar->SendAction(SpineAction::SLEEP);
            this->spinerenderer->Unlock();
        }
        break;
    default:
        return DefWindowProcW(this->hWnd, WM_TIMER, wParam, lParam);
    }
    return 0;
}

LRESULT SpriteWindow::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
    // 获取菜单句柄
    HMENU hMenu = (HMENU)wParam;
    SetMenuItemState(hMenu, IDM_SHOWSPRITE, FALSE, AppConfig::Get()->bShowSprite ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_SPMOUSEPASS, FALSE, AppConfig::Get()->bSpriteMousePass ? MFS_CHECKED : MFS_UNCHECKED);
    return 0;
}

LRESULT SpriteWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    INT deltaX = ptCursor.x - this->ptDragSrc.x;
    INT deltaY = -(ptCursor.y - this->ptDragSrc.y); // 因为按 y 轴对称了

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
            // NOTE: 此处不用 Lock/Unlock, 因为实测有点卡
            this->spinechar->SetPosition((FLOAT)(this->ptSpriteDragSrc.x + deltaX), (FLOAT)(this->ptSpriteDragSrc.y + deltaY));
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

    // 保存精灵位置
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

        // 保存一次现在的精灵位置
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

    // 保存精灵位置
    FLOAT x = 0;
    FLOAT y = 0;
    this->spinechar->GetPosition(&x, &y);
    this->ptSpriteDragSrc.x = (LONG)x;
    this->ptSpriteDragSrc.y = (LONG)y;

    // 转向
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

LRESULT SpriteWindow::OnPerfDataUpdated(WPARAM wParam, LPARAM lParam)
{
    ((PerfMonitor::PerfMonitor*)lParam)->GetPerfData(&this->perfData);
    return 0;
}

#include "framework.h"
#include "util.h"
#include "config.h"
#include "notifyicon.h"
#include "perfdata.h"

#include "mainwnd.h"

static UINT             uMsgTaskbarCreated              = 0;                        // 任务栏重建消息

static BOOL             bFloatWnd                       = FALSE;                    // 是否桌面浮窗

static HFONT            hTextFont                       = NULL;                     // 显示文本的字体
static COLORREF         rgbText                         = RGB(0, 0, 0);             // 显示文本的颜色

//static WCHAR            szBalloonIconPath[MAX_PATH]     = { 0 };                    // 气球图标文件路径
static BOOL             bInfoSound                      = FALSE;                    // 气球提示是否有声音

// 运行时数据
static BOOL             bWndFixed                       = FALSE;                    // 窗口是否通过图标点击长期显示

// 过程函数声明
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnSettingChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnInitMenuPopup(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnMouseLeave(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnNotifyIcon(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnTimeAlarm(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnTaskBarCreated(HWND hWnd, WPARAM wParam, LPARAM lParam);


// 过程处理实现
static LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // 注册任务栏重建消息
    uMsgTaskbarCreated = RegisterWindowMessageW(SZMSG_TASKBARCREATED);

    // 应用配置项
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    LoadConfigFromReg(pCfgData);
    ApplyAppConfig(pCfgData, hWnd);
    DefFreeMem(pCfgData);

    // 添加图标
    AddNotifyIcon(
        hWnd, ID_NIDMAIN, WM_NOTIFYICON,
        LoadIconW(
            GetModuleHandleW(NULL),
            MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK)
        )
    );

    // 设置图标提示信息
    WCHAR szTip[MAX_NIDTIP] = { 0 };
    LoadStringW(GetModuleHandleW(NULL), IDS_APPNAME, szTip, MAX_NIDTIP);
    SetNotifyIconTip(hWnd, ID_NIDMAIN, szTip);

    // 设置 85% 透明度
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), (255 * 85) / 100, LWA_ALPHA);

    // 每 1s 刷新一次显示
    SetTimer(hWnd, IDT_REFRESHRECT, REFRESHINTERVAL, (TIMERPROC)NULL);
    return 0;
}

static LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (hTextFont != NULL)
    {
        DeleteObject(hTextFont);
        hTextFont = NULL;
    }
    DeleteNotifyIcon(hWnd, ID_NIDMAIN);
    PostQuitMessage(EXIT_SUCCESS);
    return 0;
}

static LRESULT OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // 不是浮动且失去激活
    if (!bFloatWnd && !wParam)
    {
        bWndFixed = FALSE;
        AnimateWindow(hWnd, 100, AW_HIDE | AW_BLEND);
    }
    return 0;
}

static LRESULT OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    AnimateWindow(hWnd, 100, AW_HIDE | AW_BLEND);
    return 0;
}

static LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PERFDATA perfData = { 0 };
    GetPerfData(&perfData);
    WCHAR szDataBuffer[4][16] = { 0 };
    StringCchPrintfW(szDataBuffer[0], 16, L"%.2f%%", perfData.cpuPercent);
    StringCchPrintfW(szDataBuffer[1], 16, L"%.2f%%", perfData.memPercent);
    ConvertSpeed(perfData.uploadSpeed, szDataBuffer[2], 16);
    ConvertSpeed(perfData.downloadSpeed, szDataBuffer[3], 16);

    WCHAR szTipFormat[MAX_LOADSTRING] = { 0 };
    WCHAR szTip[MAX_LOADSTRING] = { 0 };
    LoadStringW(GetModuleHandleW(NULL), IDS_TIP, szTipFormat, MAX_LOADSTRING);
    StringCchPrintfW(
        szTip, MAX_LOADSTRING, szTipFormat,
        szDataBuffer[0], szDataBuffer[1], szDataBuffer[2], szDataBuffer[3]
    );
    SIZE_T nTextLength = 0;
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);

    PAINTSTRUCT ps = { 0 };
    HDC hdc = BeginPaint(hWnd, &ps);
    HFONT hFontPre = SelectObject(hdc, hTextFont);
    SetTextColor(hdc, rgbText);
    SetBkMode(hdc, TRANSPARENT);

    RECT rcDraw = { 0 };
    GetClientRect(hWnd, &rcDraw);
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_LEFT);

    SelectObject(hdc, hFontPre);
    EndPaint(hWnd, &ps);
    return 0;
}

static LRESULT OnSettingChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // 自动调节图标颜色
    SetNotifyIcon(hWnd, ID_NIDMAIN,
        LoadIconW(
            GetModuleHandleW(NULL),
            MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK)
        )
    );
    return 0;
}

static LRESULT OnContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // 加载ContextMenu
    HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
    HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

    // 解决在菜单外单击左键菜单不消失的问题
    SetForegroundWindow(hWnd);

    // 显示菜单
    TrackPopupMenuEx(
        hContextMenu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON,
        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
        hWnd, NULL
    );

    DestroyMenu(hContextMenuBar);
    return 0;
}

static LRESULT OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) Menu: FALSE, Accelerator: TRUE
    // LOWORD(wParam) identifier
    // lParam: 0

    // 读取配置
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    LoadConfigFromReg(pCfgData);
    switch (LOWORD(wParam))
    {
    case IDM_FLOATWND:
        pCfgData->bFloatWnd = !pCfgData->bFloatWnd;
        break;
    case IDM_AUTORUN:
        pCfgData->bAutoRun = !pCfgData->bAutoRun;
        break;
    case IDM_TIMEALARM:
        pCfgData->bTimeAlarm = !pCfgData->bTimeAlarm;
        break;
    case IDM_INFOSOUND:
        pCfgData->bInfoSound = !pCfgData->bInfoSound;
        break;
    case IDM_EXIT:
        DestroyWindow(hWnd);
        break;
    default:
        DefFreeMem(pCfgData);
        return DefWindowProcW(hWnd, WM_COMMAND, wParam, lParam);
    }

    // 保存配置
    ApplyAppConfig(pCfgData, hWnd);
    SaveConfigToReg(pCfgData);
    DefFreeMem(pCfgData);
    return 0;
}

static LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case IDT_REFRESHRECT:
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case IDT_TIMEALARM:
        PostMessageW(hWnd, WM_TIMEALARM, 0, 0);
        SetTimer(hWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
        break;
    default:
        return DefWindowProcW(hWnd, WM_TIMER, wParam, lParam);
    }
    return 0;
}

static LRESULT OnInitMenuPopup(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    LoadConfigFromReg(pCfgData);

    HMENU hMenu = (HMENU)wParam;
    SetMenuItemState(hMenu, IDM_AUTORUN, pCfgData->bAutoRun ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_FLOATWND, pCfgData->bFloatWnd ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_TIMEALARM, pCfgData->bTimeAlarm ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_INFOSOUND, pCfgData->bInfoSound ? MFS_CHECKED : MFS_UNCHECKED);

    DefFreeMem(pCfgData);
    return 0;
}

static LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProcW(hWnd, WM_MOUSEMOVE, wParam, lParam);
}

static LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // TODO
    return DefWindowProcW(hWnd, WM_LBUTTONDOWN, wParam, lParam);
}

static LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // TODO
    return DefWindowProcW(hWnd, WM_LBUTTONUP, wParam, lParam);
}

static LRESULT OnMouseLeave(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // TODO
    return DefWindowProcW(hWnd, WM_MOUSELEAVE, wParam, lParam);
}

static LRESULT OnNotifyIcon(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(lParam))
    {
    case WM_CONTEXTMENU:
    {
        // 加载ContextMenu
        HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
        HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

        // 解决在菜单外单击左键菜单不消失的问题
        SetForegroundWindow(hWnd);
        
        // 显示菜单
        TrackPopupMenuEx(
            hContextMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON,
            GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam),
            hWnd, NULL
        );

        DestroyMenu(hContextMenuBar);
        break;
    }
    case NIN_SELECT:
    case NIN_KEYSELECT:
    {
        if (!bFloatWnd)
        {
            // 不是浮动的情况下固定住窗口显示
            bWndFixed = TRUE;
        }

        // 只有点击了图标才需要设置前景窗口
        SetForegroundWindow(hWnd);
    }
    case NIN_POPUPOPEN:
    {
        // 不是浮动的情况下显示显示弹窗
        if (!bFloatWnd)
        {
            //SetLayeredWindowAttributes;
            //UpdateLayeredWindow;
            //UpdateLayeredWindowIndirect;
            RECT rcNotifyIcon = { 0 };
            GetNotifyIconRect(hWnd, ID_NIDMAIN, &rcNotifyIcon);

            SetWindowPos(
                hWnd, HWND_TOPMOST,
                rcNotifyIcon.left - ((150 - (rcNotifyIcon.right - rcNotifyIcon.left)) / 2),
                rcNotifyIcon.top - 100,
                150, 100,
                SWP_NOACTIVATE
            );
            AnimateWindow(hWnd, 100, AW_BLEND);

            // 需要立即重绘窗口
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    }
    case NIN_POPUPCLOSE:
    {
        // 不是浮动且没有固定则隐藏弹窗
        if (!bFloatWnd && !bWndFixed)
        {
            AnimateWindow(hWnd, 100, AW_HIDE | AW_BLEND);
            break;
        }
    }
    default:
        return DefWindowProcW(hWnd, WM_NOTIFYICON, wParam, lParam);
    }
    return 0;
}

static LRESULT OnTimeAlarm(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);

    HINSTANCE hInstance = GetModuleHandleW(NULL);

    WCHAR szInfoTitle[MAX_NIDINFOTITLE] = { 0 };
    LoadStringW(hInstance, IDS_TIMEALARMTITLE, szInfoTitle, MAX_NIDINFOTITLE);

    WCHAR szInfo[MAX_NIDINFO] = { 0 };
    WCHAR szInfoFormat[MAX_NIDINFO] = { 0 };
    LoadStringW(hInstance, IDS_TIMEALARMINFO, szInfoFormat, MAX_NIDINFO);
    StringCchPrintfW(szInfo, MAX_NIDINFO, szInfoFormat, st.wHour, st.wMinute);

    //HICON hIcon = LoadIconFromFile(szBalloonIconPath);
    HICON hIcon = LoadIconRawSize(hInstance, MAKEINTRESOURCEW(IDI_TIMEALARM));
    SetNotifyIconInfo(hWnd, ID_NIDMAIN, szInfoTitle, szInfo, hIcon, bInfoSound);

    DestroyIcon(hIcon);
    return 0;
}

static LRESULT OnTaskBarCreated(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    AddNotifyIcon(
        hWnd, ID_NIDMAIN, WM_NOTIFYICON,
        LoadIconW(
            GetModuleHandleW(NULL), 
            MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK)
        )
    );
    return 0;
}


// 消息映射
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate(hWnd, wParam, lParam);
    case WM_DESTROY:
        return OnDestroy(hWnd, wParam, lParam);
    case WM_ACTIVATE:
        return OnActivate(hWnd, wParam, lParam);
    case WM_CLOSE:
        return OnClose(hWnd, wParam, lParam);
    case WM_PAINT:
        return OnPaint(hWnd, wParam, lParam);
    case WM_SETTINGCHANGE:
        return OnSettingChange(hWnd, wParam, lParam);
    case WM_CONTEXTMENU:
        return OnContextMenu(hWnd, wParam, lParam);
    case WM_COMMAND:
        return OnCommand(hWnd, wParam, lParam);
    case WM_TIMER:
        return OnTimer(hWnd, wParam, lParam);
    case WM_INITMENUPOPUP:
        return OnInitMenuPopup(hWnd, wParam, lParam);
    case WM_MOUSEMOVE:
        return OnMouseMove(hWnd, wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(hWnd, wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(hWnd, wParam, lParam);
    case WM_MOUSELEAVE:
        return OnMouseLeave(hWnd, wParam, lParam);
    case WM_NOTIFYICON:
        return OnNotifyIcon(hWnd, wParam, lParam);
    case WM_TIMEALARM:
        return OnTimeAlarm(hWnd, wParam, lParam);
    default:
        if (uMsg == uMsgTaskbarCreated)
            return OnTaskBarCreated(hWnd, wParam, lParam);
        else
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}


ATOM RegisterMainWnd(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.lpszClassName = MAINWNDCLASSNAME;
    wcex.lpfnWndProc = MainWndProc;
    wcex.hInstance = hInstance;
    wcex.style = CS_DBLCLKS | CS_DROPSHADOW | CS_HREDRAW | CS_VREDRAW;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.hbrBackground = GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

HWND CreateMainWnd(HINSTANCE hInstance)
{
    return CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        MAINWNDCLASSNAME, NULL, 
        WS_POPUP,
        CW_USEDEFAULT, 0, 400, 100,
        NULL, NULL, hInstance, NULL
    );
}

DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd)
{
    bFloatWnd = pCfgData->bFloatWnd;
    if (pCfgData->bFloatWnd)
    {
        AnimateWindow(hMainWnd, 100, AW_BLEND);
        InvalidateRect(hMainWnd, NULL, TRUE);
    }
    else
    {
        AnimateWindow(hMainWnd, 100, AW_HIDE | AW_BLEND);
    }

    if (pCfgData->bAutoRun)
    {
        SetAppAutoRun();
    }
    else
    {
        UnsetAppAutoRun();
    }

    if (pCfgData->bTimeAlarm)
    {
        SetTimer(hMainWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
    }
    else 
    {
        KillTimer(hMainWnd, IDT_TIMEALARM);
    }

    DeleteObject(hTextFont);
    hTextFont = CreateFontIndirectW(&pCfgData->lfText);
    rgbText = pCfgData->rgbTextColor;

    //StringCchCopyW(szBalloonIconPath, MAX_PATH, pCfgData->szBalloonIconPath);
    bInfoSound = pCfgData->bInfoSound;
    return 0;
}

UINT GetHourTimeDiff()
{
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    UINT uTimeDiff = (59 - st.wMinute) * 60 * 1000 + (60 - st.wSecond) * 1000 + 100;
    return uTimeDiff;
}

DWORD SetMenuItemState(HMENU hMenu, UINT uIdentifier, UINT uState)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STATE;
    mii.fState = uState;
    SetMenuItemInfoW(hMenu, uIdentifier, FALSE, &mii); // ERROR_MENU_ITEM_NOT_FOUND
    return 0;
}

UINT GetMenuItemState(HMENU hMenu, UINT uIdentifier)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STATE;
    GetMenuItemInfoW(hMenu, uIdentifier, FALSE, &mii);
    return mii.fState;
}

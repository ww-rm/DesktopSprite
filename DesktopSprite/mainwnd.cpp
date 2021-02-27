#include "framework.h"
#include "util.h"
#include "config.h"
#include "notifyicon.h"
#include "perfdata.h"
#include "mainwnd.h"

// 常量定义
static UINT     const   REFRESHINTERVAL                 = 1000;                     // 屏幕显示刷新间隔
static UINT     const   ANIMATIONTIME                   = 100;                      // 动画效果持续时间
static UINT     const   ID_NIDMAIN                      = 1;                        // 图标 ID
static INT      const   MAINWNDSIZE_CX                  = 100;                      // 窗口 cx
static INT      const   MAINWNDSIZE_CY                  = 400;                      // 窗口 cy

// 运行时设置项
static BOOL             bFloatWnd                       = FALSE;                    // 是否桌面浮窗
static HFONT            hTextFont                       = NULL;                     // 显示文本的字体
static COLORREF         rgbText                         = RGB(0, 0, 0);             // 显示文本的颜色
//static WCHAR            szBalloonIconPath[MAX_PATH]     = { 0 };                    // 气球图标文件路径
static BOOL             bInfoSound                      = FALSE;                    // 气球提示是否有声音

// 运行时数据
static UINT             uMsgTaskbarCreated              = 0;                        // 任务栏重建消息
static BOOL             bWndFixed                       = FALSE;                    // 窗口是否通过图标点击长期显示
static POINT            ptDragSrc                       = { 0 };                    // 拖动窗口时的源点


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

    // 按照读取的配置项初始化窗口数据
    bFloatWnd = pCfgData->bFloatWnd;

    // 是否浮动窗口
    if (pCfgData->bFloatWnd)
    {
        // 调整显示位置
        SetWindowPos(
            hWnd, HWND_TOPMOST,
            pCfgData->ptLastFloatPos.x, pCfgData->ptLastFloatPos.y,
            MAINWNDSIZE_CX, MAINWNDSIZE_CY,
            SWP_NOACTIVATE
        );

        // 显示窗口
        AnimateWindow(hWnd, ANIMATIONTIME, AW_BLEND);
        InvalidateRect(hWnd, NULL, TRUE);
    }

    // 是否整点报时
    if (pCfgData->bTimeAlarm)
    {
        SetTimer(hWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
    }

    // 字体, 颜色, 声音
    hTextFont = CreateFontIndirectW(&pCfgData->lfText);
    rgbText = pCfgData->rgbTextColor;
    bInfoSound = pCfgData->bInfoSound;
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
        AnimateWindow(hWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
    }
    return 0;
}

static LRESULT OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    AnimateWindow(hWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
    return 0;
}

static LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // 得到格式化的字符串数据 xx.xx% xx.xxKB/s
    PERFDATA perfData = { 0 };
    GetPerfData(&perfData);
    WCHAR szDataBuffer[4][16] = { 0 };  // CPU:MEM:UPLOAD:DOWNLOAD
    StringCchPrintfW(szDataBuffer[0], 16, L"%.2f%%", perfData.cpuPercent);
    StringCchPrintfW(szDataBuffer[1], 16, L"%.2f%%", perfData.memPercent);
    ConvertSpeed(perfData.uploadSpeed, szDataBuffer[2], 16);
    ConvertSpeed(perfData.downloadSpeed, szDataBuffer[3], 16);

    // 开始绘图
    PAINTSTRUCT ps = { 0 };
    HDC hdc = BeginPaint(hWnd, &ps);
    HFONT hFontPre = SelectFont(hdc, hTextFont);
    SetTextColor(hdc, rgbText);
    SetBkMode(hdc, TRANSPARENT);

    HINSTANCE hInstance = GetModuleHandleW(NULL);
    WCHAR szTip[MAX_LOADSTRING] = { 0 };
    RECT rcClient = { 0 };
    GetClientRect(hWnd, &rcClient);
    INT nClientHeight = rcClient.bottom - rcClient.top;
    RECT rcDraw = { 0 };
    CopyRect(&rcDraw, &rcClient);
    SIZE_T nTextLength = 0;

    // 绘制CPU
    LoadStringW(hInstance, IDS_TIP_CPU, szTip, MAX_LOADSTRING);
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);
    rcDraw.top = 0;
    rcDraw.bottom = rcDraw.top + nClientHeight / 8;
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_CENTER);

    StringCchLengthW(szDataBuffer[0], 16, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szDataBuffer[0], (INT)nTextLength, &rcDraw, DT_CENTER);

    // 绘制内存
    LoadStringW(hInstance, IDS_TIP_MEM, szTip, MAX_LOADSTRING);
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_CENTER);

    StringCchLengthW(szDataBuffer[1], 16, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szDataBuffer[1], (INT)nTextLength, &rcDraw, DT_CENTER);

    // 绘制上传
    LoadStringW(hInstance, IDS_TIP_UPLOAD, szTip, MAX_LOADSTRING);
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_CENTER);

    StringCchLengthW(szDataBuffer[2], 16, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szDataBuffer[2], (INT)nTextLength, &rcDraw, DT_CENTER);

    // 绘制下载
    LoadStringW(hInstance, IDS_TIP_DOWNLOAD, szTip, MAX_LOADSTRING);
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_CENTER);

    StringCchLengthW(szDataBuffer[3], 16, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szDataBuffer[3], (INT)nTextLength, &rcDraw, DT_CENTER);

    // 结束绘图
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
    {
        pCfgData->bFloatWnd = !pCfgData->bFloatWnd;
        bFloatWnd = pCfgData->bFloatWnd;
        if (pCfgData->bFloatWnd)
        {
            // 调整显示位置
            SetWindowPos(
                hWnd, HWND_TOPMOST,
                pCfgData->ptLastFloatPos.x, pCfgData->ptLastFloatPos.y,
                MAINWNDSIZE_CX, MAINWNDSIZE_CY,
                SWP_NOACTIVATE
            );

            // 显示窗口
            AnimateWindow(hWnd, ANIMATIONTIME, AW_BLEND);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else
        {
            // 如果不显示浮动窗口就保存一次现在的窗口位置
            RECT rcWnd = { 0 };
            GetWindowRect(hWnd, &rcWnd);
            pCfgData->ptLastFloatPos.x = rcWnd.left;
            pCfgData->ptLastFloatPos.y = rcWnd.top;
            AnimateWindow(hWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
        }
        break;
    }
    case IDM_AUTORUN:
    {
        pCfgData->bAutoRun = !pCfgData->bAutoRun;
        if (pCfgData->bAutoRun)
        {
            SetAppAutoRun();
        }
        else
        {
            UnsetAppAutoRun();
        }
        break;
    }
    case IDM_TIMEALARM:
    {
        pCfgData->bTimeAlarm = !pCfgData->bTimeAlarm;
        if (pCfgData->bTimeAlarm)
        {
            SetTimer(hWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
        }
        else
        {
            KillTimer(hWnd, IDT_TIMEALARM);
        }
        break;
    }
    case IDM_INFOSOUND:
    {
        pCfgData->bInfoSound = !pCfgData->bInfoSound;
        bInfoSound = pCfgData->bInfoSound;
        break;
    }
    case IDM_EXIT:
    {
        if (pCfgData->bFloatWnd)
        {
            // 如果当前显示浮动窗口退出时就保存一次现在的窗口位置
            RECT rcWnd = { 0 };
            GetWindowRect(hWnd, &rcWnd);
            pCfgData->ptLastFloatPos.x = rcWnd.left;
            pCfgData->ptLastFloatPos.y = rcWnd.top;
        }
        DestroyWindow(hWnd);
        break;
    }
    default:
    {
        DefFreeMem(pCfgData);
        return DefWindowProcW(hWnd, WM_COMMAND, wParam, lParam);
    }
    }

    // 保存修改之后的配置
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
    if (bFloatWnd)
    {
        if (wParam & MK_LBUTTON)
        {
            POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rcWnd;
            GetWindowRect(hWnd, &rcWnd);
            SetWindowPos(
                hWnd, HWND_TOPMOST,
                rcWnd.left + (ptCursor.x - ptDragSrc.x),
                rcWnd.top + (ptCursor.y- ptDragSrc.y), 
                MAINWNDSIZE_CX, MAINWNDSIZE_CY, 
                SWP_SHOWWINDOW | SWP_NOSIZE
            );
        }
    }
    return 0;
}

static LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // 保存点击位置
    ptDragSrc.x = GET_X_LPARAM(lParam);
    ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
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
                rcNotifyIcon.left - ((MAINWNDSIZE_CX - (rcNotifyIcon.right - rcNotifyIcon.left)) / 2),
                rcNotifyIcon.top - MAINWNDSIZE_CY,
                MAINWNDSIZE_CX, MAINWNDSIZE_CY,
                SWP_NOACTIVATE
            );
            AnimateWindow(hWnd, ANIMATIONTIME, AW_BLEND);

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
            AnimateWindow(hWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
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
    wcex.hbrBackground = GetStockBrush(BLACK_BRUSH);
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
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        NULL, NULL, hInstance, NULL
    );
}

//DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd)
//{
//    bFloatWnd = pCfgData->bFloatWnd;
//    if (pCfgData->bFloatWnd)
//    {
//        // 调整显示位置
//        SetWindowPos(
//            hMainWnd, HWND_TOPMOST,
//            pCfgData->ptLastFloatPos.x, pCfgData->ptLastFloatPos.y,
//            MAINWNDSIZE_CX, MAINWNDSIZE_CY,
//            SWP_NOACTIVATE
//        );
//
//        // 显示窗口
//        AnimateWindow(hMainWnd, ANIMATIONTIME, AW_BLEND);
//        InvalidateRect(hMainWnd, NULL, TRUE);
//    }
//    else
//    {
//        AnimateWindow(hMainWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
//    }
//
//    if (pCfgData->bAutoRun)
//    {
//        SetAppAutoRun();
//    }
//    else
//    {
//        UnsetAppAutoRun();
//    }
//
//    if (pCfgData->bTimeAlarm)
//    {
//        SetTimer(hMainWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
//    }
//    else 
//    {
//        KillTimer(hMainWnd, IDT_TIMEALARM);
//    }
//
//    DeleteObject(hTextFont);
//    hTextFont = CreateFontIndirectW(&pCfgData->lfText);
//    rgbText = pCfgData->rgbTextColor;
//
//    //StringCchCopyW(szBalloonIconPath, MAX_PATH, pCfgData->szBalloonIconPath);
//    bInfoSound = pCfgData->bInfoSound;
//    return 0;
//}

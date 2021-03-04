#include "framework.h"
#include "util.h"
#include "config.h"
#include "notifyicon.h"
#include "perfdata.h"

#include "mainwnd.h"
// TODO: 透明度设置对话框

using namespace Gdiplus;

// 常量定义
static UINT     const   REFRESHINTERVAL                 = 1000;                     // 屏幕显示刷新间隔
static UINT     const   ANIMATIONTIME                   = 100;                      // 动画效果持续时间
static UINT     const   ID_NIDMAIN                      = 1;                        // 图标 ID
static INT      const   MAINWNDSIZE_UNIT                = 25;                       // 窗口单元格大小

static UINT             uMsgTaskbarCreated              = 0;                        // 任务栏重建消息

// 过程函数声明
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT OnCreate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnDestroy(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnActivate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnClose(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnPaint(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnSettingChange(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnContextMenu(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnCommand(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnTimer(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnInitMenuPopup(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnMouseMove(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnLButtonDown(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnLButtonUp(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnMouseLeave(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnNotifyIcon(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnTimeAlarm(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnTaskBarCreated(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);


// 过程处理实现
static LRESULT OnCreate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // 应用配置项
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    LoadConfigFromReg(pCfgData);

    // 按照读取的配置项初始化窗口数据
    pWndData->bFloatWnd = pCfgData->bFloatWnd;

    // 声音
    pWndData->bInfoSound = pCfgData->bInfoSound;

    // 主题
    pWndData->bDarkTheme = pCfgData->bDarkTheme;

    // 设置透明度
    SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);

    // 根据显示内容决定窗口大小
    pWndData->byShowContent = pCfgData->byShowContent;
    SIZE sizeWnd = { 0 };
    GetWndSizeByShowContent(&sizeWnd, pCfgData->byShowContent);

    // 设置位置大小
    SetWindowPos(
        pWndData->hWnd, HWND_TOPMOST,
        pCfgData->ptLastFloatPos.x, pCfgData->ptLastFloatPos.y,
        sizeWnd.cx, sizeWnd.cy,
        SWP_NOACTIVATE
    );
    
    // 是否显示浮动窗口
    if (pCfgData->bFloatWnd)
    {
        ShowWindow(pWndData->hWnd, SW_SHOWNA);
        InvalidateRect(pWndData->hWnd, NULL, TRUE);
    }

    // 是否整点报时
    if (pCfgData->bTimeAlarm)
    {
        SetTimer(pWndData->hWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
    }

    // 字体与颜色
    LOGFONTW lfText = { 0 };
    GetSystemCapitalFont(&lfText);
    StringCchCopyW(lfText.lfFaceName, LF_FACESIZE, L"Agency FB");
    lfText.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    lfText.lfQuality = ANTIALIASED_QUALITY;
    pWndData->hFontText = CreateFontIndirectW(&lfText);
    DefFreeMem(pCfgData);

    // 初始化自身数据
    pWndData->bWndFixed = FALSE;

    // 添加图标
    AddNotifyIcon(
        pWndData->hWnd, ID_NIDMAIN, WM_NOTIFYICON,
        LoadIconW(
            GetModuleHandleW(NULL),
            MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK)
        )
    );

    // 设置图标提示信息
    WCHAR szTip[MAX_NIDTIP] = { 0 };
    LoadStringW(GetModuleHandleW(NULL), IDS_APPNAME, szTip, MAX_NIDTIP);
    SetNotifyIconTip(pWndData->hWnd, ID_NIDMAIN, szTip);

    // 每 1s 刷新一次显示
    SetTimer(pWndData->hWnd, IDT_REFRESHRECT, REFRESHINTERVAL, (TIMERPROC)NULL);
    return 0;
}

static LRESULT OnDestroy(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    DeleteFont(pWndData->hFontText);
    DeleteNotifyIcon(pWndData->hWnd, ID_NIDMAIN);
    PostQuitMessage(EXIT_SUCCESS);
    return 0;
}

static LRESULT OnActivate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // 不是浮动且失去激活
    if (!pWndData->bFloatWnd && !wParam)
    {
        pWndData->bWndFixed = FALSE;
        ShowWindow(pWndData->hWnd, SW_HIDE);
    }
    return 0;
}

static LRESULT OnClose(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // 屏蔽普通退出, 只能通过菜单退出
    return 0;
}

static LRESULT OnPaint(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // 得到性能数据
    PERFDATA perfData = { 0 };
    GetPerfData(&perfData);
    WCHAR szDataBuffer[16] = { 0 };     // 字符串缓冲区
    INT nLevel = 0;
    Color statusColor;

    // 开始绘图
    PAINTSTRUCT ps = { 0 };
    HDC hdc = BeginPaint(pWndData->hWnd, &ps);

    // 得到绘图窗体大小
    RECT rcClient;
    GetClientRect(pWndData->hWnd, &rcClient);
    SizeF sizeClient(REAL(rcClient.right - rcClient.left), REAL(rcClient.bottom - rcClient.top));

    // 使用缓冲区绘图
    Bitmap* pBmpMem = new Bitmap((INT)sizeClient.Width, (INT)sizeClient.Height);
    Graphics graphicsMem(pBmpMem);

    // 设置绘图模式
    graphicsMem.SetSmoothingMode(SmoothingModeAntiAlias);                       // 图形渲染抗锯齿
    graphicsMem.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);        // 文字渲染抗锯齿

    // 绘图属性对象
    SizeF drawSize(MAINWNDSIZE_UNIT * 3, MAINWNDSIZE_UNIT * 3);                 // 绘制矩形
    Pen pen(Color::Green, 5);                                                   // 图形颜色
    SolidBrush textbrush(pWndData->bDarkTheme ? Color::White : Color::Black);   // 文本颜色
    SolidBrush bgbrush(pWndData->bDarkTheme ? Color::Black : Color::White);     // 背景颜色
    Font font(hdc, pWndData->hFontText);                                        // 文字字体
    StringFormat strformat(StringFormatFlagsNoClip);                            // 文字居于矩形中心
    strformat.SetAlignment(StringAlignmentCenter);
    strformat.SetLineAlignment(StringAlignmentCenter);

    // 刷黑背景
    graphicsMem.FillRectangle(&bgbrush, 0., 0., sizeClient.Width, sizeClient.Height);

    // 绘制CPU与MEM
    if (pWndData->byShowContent & SHOWCONTENT_CPUMEM)
    {
        drawSize.Width = MAINWNDSIZE_UNIT * 3;
        drawSize.Height = MAINWNDSIZE_UNIT * 3;
        graphicsMem.TranslateTransform(0, 0);

        // CPU
        StringCchPrintfW(szDataBuffer, 16, L"C:%.0f%%", perfData.cpuPercent);
        if (perfData.cpuPercent < 50)
        {
            pen.SetColor(STATUSCOLOR_LOW);
        }
        else if (perfData.cpuPercent < 75)
        {
            pen.SetColor(STATUSCOLOR_MIDDLE);
        }
        else
        {
            pen.SetColor(STATUSCOLOR_HIGH);
        }
        graphicsMem.DrawString(
            szDataBuffer, -1, &font,
            RectF(PointF(0, 0), drawSize),
            &strformat, &textbrush
        );

        DrawCircle(
            graphicsMem, pen,
            PointF(drawSize.Width / 2, drawSize.Height / 2),
            drawSize.Height / 2 - 8,
            REAL(perfData.cpuPercent / 100)
        );

        // 绘制内存
        StringCchPrintfW(szDataBuffer, 16, L"M:%.0f%%", perfData.memPercent);
        if (perfData.memPercent < 75)
        {
            pen.SetColor(STATUSCOLOR_LOW);
        }
        else if (perfData.memPercent < 90)
        {
            pen.SetColor(STATUSCOLOR_MIDDLE);
        }
        else
        {
            pen.SetColor(STATUSCOLOR_HIGH);
        }
        graphicsMem.DrawString(
            szDataBuffer, -1, &font,
            RectF(PointF(MAINWNDSIZE_UNIT * 3, 0), drawSize),
            &strformat, &textbrush
        );
        DrawCircle(
            graphicsMem, pen,
            PointF(drawSize.Width / 2 + MAINWNDSIZE_UNIT * 3, drawSize.Height / 2),
            drawSize.Height / 2 - 8,
            REAL(perfData.memPercent / 100)
        );
    }

    // 绘制网速
    if (pWndData->byShowContent & SHOWCONTENT_NETSPEED)
    {
        drawSize.Width = MAINWNDSIZE_UNIT * 3;
        drawSize.Height = MAINWNDSIZE_UNIT * 1;
        graphicsMem.TranslateTransform(0, 0);

        if (pWndData->byShowContent & SHOWCONTENT_CPUMEM)
        {
            graphicsMem.TranslateTransform(0, MAINWNDSIZE_UNIT * 3);
        }

        // 绘制上传
        nLevel = ConvertSpeed(perfData.uploadSpeed, szDataBuffer, 16);
        if (nLevel < 3)
        {
            statusColor = STATUSCOLOR_HIGH;
        }
        else if (nLevel < 5)
        {
            statusColor = STATUSCOLOR_MIDDLE;
        }
        else
        {
            statusColor = STATUSCOLOR_LOW;
        }
        graphicsMem.DrawString(
            szDataBuffer, -1, &font,
            RectF(PointF(0, MAINWNDSIZE_UNIT), drawSize),
            &strformat, &textbrush
        );
        DrawSpeedStair(
            graphicsMem, statusColor,
            RectF(10, 2, drawSize.Width - 20, drawSize.Height - 4),
            TRUE, nLevel
        );

        // 绘制下载
        nLevel = ConvertSpeed(perfData.downloadSpeed, szDataBuffer, 16);
        if (nLevel < 3)
        {
            statusColor = STATUSCOLOR_HIGH;
        }
        else if (nLevel < 5)
        {
            statusColor = STATUSCOLOR_MIDDLE;
        }
        else
        {
            statusColor = STATUSCOLOR_LOW;
        }
        graphicsMem.DrawString(
            szDataBuffer, -1, &font,
            RectF(PointF(MAINWNDSIZE_UNIT * 3, MAINWNDSIZE_UNIT), drawSize),
            &strformat, &textbrush
        );
        DrawSpeedStair(
            graphicsMem, statusColor,
            RectF(MAINWNDSIZE_UNIT * 3 + 10, 2, drawSize.Width - 20, drawSize.Height - 4),
            FALSE, nLevel
        );
    }

    // 拷贝缓存图, 结束绘图
    Graphics graphics(hdc);
    CachedBitmap* pCachedBmp = new CachedBitmap(pBmpMem, &graphics);
    graphics.DrawCachedBitmap(pCachedBmp, 0, 0);
    EndPaint(pWndData->hWnd, &ps);

    delete pCachedBmp;
    delete pBmpMem;
    return 0;
}

static LRESULT OnSettingChange(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // 自动调节图标颜色
    SetNotifyIcon(pWndData->hWnd, ID_NIDMAIN,
        LoadIconW(
            GetModuleHandleW(NULL),
            MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK)
        )
    );
    return 0;
}

static LRESULT OnContextMenu(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // 加载ContextMenu
    HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
    HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

    // 解决在菜单外单击左键菜单不消失的问题
    SetForegroundWindow(pWndData->hWnd);

    // 显示菜单
    TrackPopupMenuEx(
        hContextMenu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON,
        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
        pWndData->hWnd, NULL
    );

    DestroyMenu(hContextMenuBar);
    return 0;
}

static LRESULT OnCommand(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) Menu: FALSE, Accelerator: TRUE
    // LOWORD(wParam) identifier
    // lParam: 0

    // 读取配置
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    LoadConfigFromReg(pCfgData);

    switch (LOWORD(wParam))
    {
        // 浮动窗口
    case IDM_FLOATWND:
    {
        pCfgData->bFloatWnd = !pCfgData->bFloatWnd;
        pWndData->bFloatWnd = pCfgData->bFloatWnd;
        if (pCfgData->bFloatWnd)
        {
            // 调整显示位置
            SetWindowPos(
                pWndData->hWnd, HWND_TOPMOST,
                pCfgData->ptLastFloatPos.x, pCfgData->ptLastFloatPos.y,
                0, 0,
                SWP_NOACTIVATE | SWP_NOSIZE
            );

            // 显示窗口
            ShowWindow(pWndData->hWnd, SW_SHOWNA);
            InvalidateRect(pWndData->hWnd, NULL, TRUE);
        }
        else
        {
            // 如果不显示浮动窗口就保存一次现在的窗口位置
            RECT rcWnd = { 0 };
            GetWindowRect(pWndData->hWnd, &rcWnd);
            pCfgData->ptLastFloatPos.x = rcWnd.left;
            pCfgData->ptLastFloatPos.y = rcWnd.top;
            ShowWindow(pWndData->hWnd, SW_HIDE);
        }
        break;
    }
    // 显示子菜单
    case IDM_SHOWCPUMEM:
    {
        if (pCfgData->byShowContent == SHOWCONTENT_CPUMEM)
        {
            MessageBoxW(NULL, L"至少保留一项显示内容！", L"提示信息", MB_OK);
        }
        else
        {
            pCfgData->byShowContent ^= SHOWCONTENT_CPUMEM;
            pWndData->byShowContent = pCfgData->byShowContent;
            SIZE sizeWnd = { 0 };
            GetWndSizeByShowContent(&sizeWnd, pCfgData->byShowContent);
            SetWindowPos(
                pWndData->hWnd, HWND_TOPMOST,
                0, 0,
                sizeWnd.cx, sizeWnd.cy,
                SWP_NOACTIVATE | SWP_NOMOVE
            );
        }
        break;
    }
    case IDM_SHOWNETSPEED:
    {
        if (pCfgData->byShowContent == SHOWCONTENT_NETSPEED)
        {
            MessageBoxW(NULL, L"至少保留一项显示内容！", L"提示信息", MB_OK);
        }
        else
        {
            pCfgData->byShowContent ^= SHOWCONTENT_NETSPEED;
            pWndData->byShowContent = pCfgData->byShowContent;
            SIZE sizeWnd = { 0 };
            GetWndSizeByShowContent(&sizeWnd, pCfgData->byShowContent);
            SetWindowPos(
                pWndData->hWnd, HWND_TOPMOST,
                0, 0,
                sizeWnd.cx, sizeWnd.cy,
                SWP_NOACTIVATE | SWP_NOMOVE
            );
        }
        break;
    }
    case IDM_DARKTHEME:
    {
        pCfgData->bDarkTheme = !pCfgData->bDarkTheme;
        pWndData->bDarkTheme = pCfgData->bDarkTheme;
        InvalidateRect(pWndData->hWnd, NULL, TRUE);
        break;
    }
    case IDM_TRAN_100:
    {
        pCfgData->byTransparency = 100 * 255 / 100;
        pWndData->byTransparency = pCfgData->byTransparency;
        SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
        break;
    }
    case IDM_TRAN_75:
    {
        pCfgData->byTransparency = 75 * 255 / 100;
        pWndData->byTransparency = pCfgData->byTransparency;
        SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
        break;
    }
    case IDM_TRAN_50:
    {
        pCfgData->byTransparency = 50 * 255 / 100;
        pWndData->byTransparency = pCfgData->byTransparency;
        SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
        break;
    }
    case IDM_TRAN_25:
    {
        pCfgData->byTransparency = 25 * 255 / 100;
        pWndData->byTransparency = pCfgData->byTransparency;
        SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
        break;
    }
    case IDM_TRANSPARENCY:
    {
        // TODO: 弹出一个填透明度的对话框
        SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
        break;
    }
    // 设置子菜单
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
            SetTimer(pWndData->hWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
        }
        else
        {
            KillTimer(pWndData->hWnd, IDT_TIMEALARM);
        }
        break;
    }
    case IDM_INFOSOUND:
    {
        pCfgData->bInfoSound = !pCfgData->bInfoSound;
        pWndData->bInfoSound = pCfgData->bInfoSound;
        break;
    }
    // 退出
    case IDM_EXIT:
    {
        if (pCfgData->bFloatWnd)
        {
            // 如果当前显示浮动窗口退出时就保存一次现在的窗口位置
            RECT rcWnd = { 0 };
            GetWindowRect(pWndData->hWnd, &rcWnd);
            pCfgData->ptLastFloatPos.x = rcWnd.left;
            pCfgData->ptLastFloatPos.y = rcWnd.top;
        }
        DestroyWindow(pWndData->hWnd);
        break;
    }
    default:
    {
        DefFreeMem(pCfgData);
        return DefWindowProcW(pWndData->hWnd, WM_COMMAND, wParam, lParam);
    }
    }

    // 保存修改之后的配置
    SaveConfigToReg(pCfgData);
    DefFreeMem(pCfgData);
    return 0;
}

static LRESULT OnTimer(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case IDT_REFRESHRECT:
        InvalidateRect(pWndData->hWnd, NULL, TRUE);
        break;
    case IDT_TIMEALARM:
        PostMessageW(pWndData->hWnd, WM_TIMEALARM, 0, 0);
        SetTimer(pWndData->hWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
        break;
    default:
        return DefWindowProcW(pWndData->hWnd, WM_TIMER, wParam, lParam);
    }
    return 0;
}

static LRESULT OnInitMenuPopup(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    LoadConfigFromReg(pCfgData);

    // 获取菜单句柄
    HMENU hMenu = (HMENU)wParam;

    // 设置菜单状态
    SetMenuItemState(hMenu, IDM_FLOATWND, FALSE, pCfgData->bFloatWnd ? MFS_CHECKED : MFS_UNCHECKED);

    // 子菜单
    SetMenuItemState(hMenu, IDM_AUTORUN, FALSE, pCfgData->bAutoRun ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_TIMEALARM, FALSE, pCfgData->bTimeAlarm ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_INFOSOUND, FALSE, pCfgData->bInfoSound ? MFS_CHECKED : MFS_UNCHECKED);

    // 子菜单
    SetMenuItemState(hMenu, IDM_SHOWCPUMEM, FALSE, (pCfgData->byShowContent & SHOWCONTENT_CPUMEM) ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_SHOWNETSPEED, FALSE, (pCfgData->byShowContent & SHOWCONTENT_NETSPEED) ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_DARKTHEME, FALSE, pCfgData->bDarkTheme ? MFS_CHECKED : MFS_UNCHECKED);
    switch (pCfgData->byTransparency)
    {
    case 100 * 255 / 100:
        SetMenuItemState(hMenu, IDM_TRAN_100, FALSE, MFS_CHECKED);
        break;
    case 75 * 255 / 100:
        SetMenuItemState(hMenu, IDM_TRAN_75, FALSE, MFS_CHECKED);
        break;
    case 50 * 255 / 100:
        SetMenuItemState(hMenu, IDM_TRAN_50, FALSE, MFS_CHECKED);
        break;
    case 25 * 255 / 100:
        SetMenuItemState(hMenu, IDM_TRAN_25, FALSE, MFS_CHECKED);
        break;
    default:
        SetMenuItemState(hMenu, IDM_TRANSPARENCY, FALSE, MFS_CHECKED);
    }

    DefFreeMem(pCfgData);
    return 0;
}

static LRESULT OnMouseMove(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    if (pWndData->bFloatWnd)
    {
        if (wParam & MK_LBUTTON)
        {
            POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rcWnd;
            GetWindowRect(pWndData->hWnd, &rcWnd);
            SetWindowPos(
                pWndData->hWnd, HWND_TOPMOST,
                rcWnd.left + (ptCursor.x - pWndData->ptDragSrc.x),
                rcWnd.top + (ptCursor.y- pWndData->ptDragSrc.y),
                0, 0, 
                SWP_SHOWWINDOW | SWP_NOSIZE
            );
        }
    }
    return 0;
}

static LRESULT OnLButtonDown(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // 保存点击位置
    pWndData->ptDragSrc.x = GET_X_LPARAM(lParam);
    pWndData->ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
}

static LRESULT OnLButtonUp(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // TODO
    return DefWindowProcW(pWndData->hWnd, WM_LBUTTONUP, wParam, lParam);
}

static LRESULT OnMouseLeave(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // TODO
    return DefWindowProcW(pWndData->hWnd, WM_MOUSELEAVE, wParam, lParam);
}

static LRESULT OnNotifyIcon(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(lParam))
    {
    case WM_CONTEXTMENU:
    {
        // 加载ContextMenu
        HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
        HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

        // 解决在菜单外单击左键菜单不消失的问题
        SetForegroundWindow(pWndData->hWnd);
        
        // 显示菜单
        TrackPopupMenuEx(
            hContextMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON,
            GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam),
            pWndData->hWnd, NULL
        );

        DestroyMenu(hContextMenuBar);
        break;
    }
    case NIN_SELECT:
    case NIN_KEYSELECT:
    {
        if (!pWndData->bFloatWnd)
        {
            // 不是浮动的情况下固定住窗口显示
            pWndData->bWndFixed = TRUE;
        }

        // 只有点击了图标才需要设置前景窗口
        SetForegroundWindow(pWndData->hWnd);
    }
    case NIN_POPUPOPEN:
    {
        // 不是浮动的情况下显示显示弹窗
        if (!pWndData->bFloatWnd)
        {
            RECT rcNotifyIcon = { 0 };
            GetNotifyIconRect(pWndData->hWnd, ID_NIDMAIN, &rcNotifyIcon);
            RECT rcWnd = { 0 };
            GetWindowRect(pWndData->hWnd, &rcWnd);
            SIZE sizeWnd = { rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top };

            SetWindowPos(
                pWndData->hWnd, HWND_TOPMOST,
                rcNotifyIcon.left - ((sizeWnd.cx - (rcNotifyIcon.right - rcNotifyIcon.left)) / 2),
                rcNotifyIcon.top - sizeWnd.cy,
                0, 0,
                SWP_NOACTIVATE | SWP_NOSIZE
            );
            ShowWindow(pWndData->hWnd, SW_SHOWNA);

            // 需要立即重绘窗口
            InvalidateRect(pWndData->hWnd, NULL, TRUE);
        }
        break;
    }
    case NIN_POPUPCLOSE:
    {
        // 不是浮动且没有固定则隐藏弹窗
        if (!pWndData->bFloatWnd && !pWndData->bWndFixed)
        {
            ShowWindow(pWndData->hWnd, SW_HIDE);
            break;
        }
    }
    default:
        return DefWindowProcW(pWndData->hWnd, WM_NOTIFYICON, wParam, lParam);
    }
    return 0;
}

static LRESULT OnTimeAlarm(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
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
    SetNotifyIconInfo(pWndData->hWnd, ID_NIDMAIN, szInfoTitle, szInfo, hIcon, pWndData->bInfoSound);

    DestroyIcon(hIcon);
    return 0;
}

static LRESULT OnTaskBarCreated(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    AddNotifyIcon(
        pWndData->hWnd, ID_NIDMAIN, WM_NOTIFYICON,
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
    PMAINWNDDATA pWndData = (PMAINWNDDATA)GetWndData(hWnd);
    switch (uMsg)
    {
    case WM_NCCREATE:
        pWndData = (PMAINWNDDATA)DefAllocMem(sizeof(MAINWNDDATA));
        if (pWndData == NULL)
            return FALSE;
        pWndData->hWnd = hWnd;
        SetWndData(hWnd, pWndData);
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    case WM_NCDESTROY:
        DefFreeMem(pWndData);
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    case WM_CREATE:
        return OnCreate(pWndData, wParam, lParam);
    case WM_DESTROY:
        return OnDestroy(pWndData, wParam, lParam);
    case WM_ACTIVATE:
        return OnActivate(pWndData, wParam, lParam);
    case WM_CLOSE:
        return OnClose(pWndData, wParam, lParam);
    case WM_PAINT:
        return OnPaint(pWndData, wParam, lParam);
    case WM_SETTINGCHANGE:
        return OnSettingChange(pWndData, wParam, lParam);
    case WM_CONTEXTMENU:
        return OnContextMenu(pWndData, wParam, lParam);
    case WM_COMMAND:
        return OnCommand(pWndData, wParam, lParam);
    case WM_TIMER:
        return OnTimer(pWndData, wParam, lParam);
    case WM_INITMENUPOPUP:
        return OnInitMenuPopup(pWndData, wParam, lParam);
    case WM_MOUSEMOVE:
        return OnMouseMove(pWndData, wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(pWndData, wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(pWndData, wParam, lParam);
    case WM_MOUSELEAVE:
        return OnMouseLeave(pWndData, wParam, lParam);
    case WM_NOTIFYICON:
        return OnNotifyIcon(pWndData, wParam, lParam);
    case WM_TIMEALARM:
        return OnTimeAlarm(pWndData, wParam, lParam);
    //case WM_MEASUREITEM:
    //case WM_DRAWITEM:
    //    return 0;
    default:
        if (uMsg == uMsgTaskbarCreated)
            return OnTaskBarCreated(pWndData, wParam, lParam);
        else
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}


ATOM RegisterMainWnd(HINSTANCE hInstance)
{
    // 注册任务栏重建消息
    uMsgTaskbarCreated = RegisterWindowMessageW(SZMSG_TASKBARCREATED);

    // 注册窗体
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
    wcex.hbrBackground = NULL;
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

BOOL DrawCircle(
    Graphics& graphics, 
    Pen& pen, 
    const PointF& ptCenter, 
    const REAL& nOuterRadius, 
    const REAL& sweepPercent
)
{
    REAL nr = nOuterRadius - pen.GetWidth() / 2;
    graphics.DrawArc(&pen, ptCenter.X - nr, ptCenter.Y - nr, 2 * nr, 2 * nr, 270, -360 * sweepPercent);
    return TRUE;
}

BOOL DrawSpeedStair(
    Graphics& graphics, 
    const Color& color, 
    const RectF& rect, 
    const BOOL& bUp, 
    const INT& nLevel, 
    const INT& nMaxLevel
)
{
    REAL whiteGap = 1;
    REAL height = rect.Height / nMaxLevel;
    REAL width = (rect.Width - whiteGap * nMaxLevel) / (nMaxLevel + 1);
    Pen pen(color, 2);

    // 绘制箭头
    PointF pt1(rect.GetLeft() + width / 2, rect.GetTop());
    PointF pt2(rect.GetLeft(), rect.GetTop() + rect.Height / 3);
    PointF pt3(rect.GetLeft() + width, rect.GetTop() + rect.Height / 3);
    PointF pt4(rect.GetLeft() + width / 2, rect.GetBottom());
    if (!bUp)
    {
        REAL ymirror = rect.GetTop() + rect.Height / 2;
        pt1.Y += 2 * (ymirror - pt1.Y);
        pt2.Y += 2 * (ymirror - pt2.Y);
        pt3.Y += 2 * (ymirror - pt3.Y);
        pt4.Y += 2 * (ymirror - pt4.Y);
    }
    graphics.DrawLine(&pen, pt1, pt2);
    graphics.DrawLine(&pen, pt1, pt3);
    graphics.DrawLine(&pen, pt1, pt4);

    // 绘制梯图
    pen.SetWidth(width);
    PointF pt5(rect.GetLeft() + 1.5f * width + whiteGap, rect.GetBottom());
    PointF pt6(rect.GetLeft() + 1.5f * width + whiteGap, rect.GetBottom() - height);
    for (INT i = 1; i <= (nLevel < nMaxLevel ? nLevel : nMaxLevel); i++)
    {
        graphics.DrawLine(&pen, pt5, pt6);
        pt5.X += (width + whiteGap);
        pt6.X += (width + whiteGap);
        pt6.Y -= height;
    }
    return TRUE;
}

DWORD GetWndSizeByShowContent(PSIZE psizeWnd, BYTE byShowContent)
{
    psizeWnd->cx = MAINWNDSIZE_UNIT * 6;
    psizeWnd->cy = 0;
    if (byShowContent & SHOWCONTENT_CPUMEM)
    {
        psizeWnd->cy += MAINWNDSIZE_UNIT * 3;
    }
    if (byShowContent & SHOWCONTENT_NETSPEED)
    {
        psizeWnd->cy += MAINWNDSIZE_UNIT * 2;
    }
    return 0;
}

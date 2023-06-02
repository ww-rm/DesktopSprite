#include <ds/framework.h>
#include <resource.h>
#include <ds/utils.h>
#include <ds/perfmonitor.h>
#include <ds/notifyicon.h>
#include <ds/config.h>
#include <ds/winapp.h>
#include <ds/aboutdlg.h>
#include <ds/configdlg.h>

#include <ds/mainwnd.h>

static const INT IDT_TIMEALARM = 1;

// 状态颜色
static Gdiplus::Color const STATUSCOLOR_GOOD = 0xff00ff00;
static Gdiplus::Color const STATUSCOLOR_NORMAL = 0xffff8000;
static Gdiplus::Color const STATUSCOLOR_BAD = 0xffff0000;

// 常量定义
static UINT     const   ID_NIDMAIN = 1;                        // 图标 ID
static UINT     const   BASE_WNDSIZE_PIXELS = 20;              // 主窗口的基本单元格像素大小


static BOOL DrawCircle(
    Gdiplus::Graphics& graphics, 
    Gdiplus::Pen& pen, 
    Gdiplus::PointF& ptCenter, 
    Gdiplus::REAL nOuterRadius, 
    Gdiplus::REAL sweepPercent
)
{
    Gdiplus::REAL nr = nOuterRadius - pen.GetWidth() / 2;
    graphics.DrawArc(&pen, ptCenter.X - nr, ptCenter.Y - nr, 2 * nr, 2 * nr, 270, -360 * sweepPercent);
    return TRUE;
}

static BOOL DrawSpeedStair(
    Gdiplus::Graphics& graphics,
    Gdiplus::Color& color,
    Gdiplus::RectF& rect,
    BOOL bUp, 
    INT nLevel, 
    INT nMaxLevel = 6
)
{
    Gdiplus::REAL whiteGap = 1;
    Gdiplus::REAL height = rect.Height / nMaxLevel;
    Gdiplus::REAL width = (rect.Width - whiteGap * nMaxLevel) / (nMaxLevel + 1);
    Gdiplus::Pen pen(color, 2);

    // 绘制箭头
    Gdiplus::PointF pt1(rect.GetLeft() + width / 2, rect.GetTop());
    Gdiplus::PointF pt2(rect.GetLeft(), rect.GetTop() + rect.Height / 3);
    Gdiplus::PointF pt3(rect.GetLeft() + width, rect.GetTop() + rect.Height / 3);
    Gdiplus::PointF pt4(rect.GetLeft() + width / 2, rect.GetBottom());
    if (!bUp)
    {
        Gdiplus::REAL ymirror = rect.GetTop() + rect.Height / 2;
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
    Gdiplus::PointF pt5(rect.GetLeft() + 1.5f * width + whiteGap, rect.GetBottom());
    Gdiplus::PointF pt6(rect.GetLeft() + 1.5f * width + whiteGap, rect.GetBottom() - height);
    for (INT i = 1; i <= (nLevel < nMaxLevel ? nLevel : nMaxLevel); i++)
    {
        graphics.DrawLine(&pen, pt5, pt6);
        pt5.X += (width + whiteGap);
        pt6.X += (width + whiteGap);
        pt6.Y -= height;
    }
    return TRUE;
}


///////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
{
    PathCchCombine(this->fontPath, MAX_PATH, WinApp::GetDir(), L"res\\font\\AGENCYR.TTF");
}

PCWSTR MainWindow::GetClassName_() const
{
    return L"DesktopSpriteMainWndClass";
}

PCWSTR MainWindow::GetFontPath() const
{
    return this->fontPath;
}

SpriteWindow* MainWindow::GetSpriteWnd()
{
    return this->spritewnd;
}

BOOL MainWindow::ShowContextMenu(INT x, INT y)
{
    // 加载ContextMenu
    HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
    HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

    // 解决在菜单外单击左键菜单不消失的问题
    SetForegroundWindow(this->hWnd);

    // 显示菜单
    TrackPopupMenuEx(hContextMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, x, y, this->hWnd, NULL);

    DestroyMenu(hContextMenuBar);

    return TRUE;;
}

BOOL MainWindow::GetDefaultWindowPos(POINT* pt)
{
    // 默认主窗口位置是屏幕的 1/6 处
    SIZE screenSize = { 0 };
    if (GetScreenResolution(&screenSize))
    {
        pt->x = screenSize.cx * 5 / 6;
        pt->y = screenSize.cy * 1 / 6;
        return TRUE;
    }
    pt->x = 0;
    pt->y = 0;
    return FALSE;
}

BOOL MainWindow::GetPopupWindowPos(POINT* pt)
{
    UINT uDirection = GetShellTrayDirection();

    // 通知区域图标位置大小
    RECT rcNotifyIcon = { 0 };
    this->pNotifyIcon->GetRect(&rcNotifyIcon);
    SIZE sizeNotifyIcon = { rcNotifyIcon.right - rcNotifyIcon.left, rcNotifyIcon.bottom - rcNotifyIcon.top };

    // 窗体位置大小
    RECT rcWnd = { 0 };
    GetWindowRect(this->hWnd, &rcWnd);
    SIZE sizeWnd = { rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top };

    // 根据任务栏位置计算窗口的位置
    LONG delta_cx = (sizeWnd.cx - sizeNotifyIcon.cx) / 2;
    LONG delta_cy = (sizeWnd.cy - sizeNotifyIcon.cy) / 2;
    switch (uDirection)
    {
    case ABE_LEFT:
        pt->x = rcNotifyIcon.right;
        pt->y = rcNotifyIcon.top - delta_cy;
        break;
    case ABE_TOP:
        pt->x = rcNotifyIcon.left - delta_cx;
        pt->y = rcNotifyIcon.bottom;
        break;
    case ABE_RIGHT:
        pt->x = rcNotifyIcon.left - sizeWnd.cx;
        pt->y = rcNotifyIcon.top - delta_cy;
        break;
    default:
        pt->x = rcNotifyIcon.left - delta_cx;
        pt->y = rcNotifyIcon.top - sizeWnd.cy;
        break;
    }

    return TRUE;
}

BOOL MainWindow::PopupOpen()
{
    // 保存原本的位置
    RECT wndRc = { 0 };
    POINT ptWnd = { 0 };

    GetWindowRect(this->hWnd, &wndRc);
    CopyPoint((POINT*)&wndRc, &this->currentFloatPos);

    if (this->GetPopupWindowPos(&ptWnd) &&
        SetWindowPos(this->hWnd, HWND_TOPMOST, ptWnd.x, ptWnd.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW) &&
        InvalidateRect(this->hWnd, NULL, TRUE))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL MainWindow::PopupClose()
{
    // 恢复原本的位置并隐藏
    return SetWindowPos(this->hWnd, HWND_TOPMOST, this->currentFloatPos.x, this->currentFloatPos.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_HIDEWINDOW);
}

BOOL MainWindow::IsIntersectDesktop()
{
    RECT desktop = { 0 };
    GetWindowRect(GetDesktopWindow(), &desktop);
    RECT wnd = { 0 };
    GetWindowRect(this->hWnd, &wnd);
    return CheckRectContainment(&wnd, &desktop) == 0;
}

BOOL MainWindow::LoadLastPosFromReg(POINT* pt)
{
    // 默认主窗口位置是屏幕的 1/6 处
    if (!this->GetDefaultWindowPos(pt))
    {
        return FALSE;
    }


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
    RegQueryAnyValue(hkApp, L"LastFloatPos", (PBYTE)pt, &cbData);
    RegCloseKey(hkApp);

    return TRUE;
}

BOOL MainWindow::SaveCurrentPosToReg()
{
    RECT currentWndRc = { 0 };
    if (!GetWindowRect(this->hWnd, &currentWndRc))
    {
        return FALSE;
    }

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

    RegSetBinValue(hkApp, L"LastFloatPos", (PBYTE)&currentWndRc, sizeof(POINT));
    RegCloseKey(hkApp);
    return TRUE;
}

HICON MainWindow::LoadNotifyIconBySysTheme()
{
    return LoadIconW(
        GetModuleHandleW(NULL), 
        MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK)
    );
}

BOOL MainWindow::ApplyConfig(const AppConfig::AppConfig* newConfig)
{
    const AppConfig::AppConfig* currentConfig = AppConfig::Get();
    if (!newConfig)
    {
        newConfig = currentConfig;
    }
    BOOL isNew = (newConfig != currentConfig);

    // 重设气泡图标
    if (!isNew || StrCmpW(newConfig->szBalloonIconPath, currentConfig->szBalloonIconPath))
    {
        DestroyIcon(this->balloonIcon);
        Gdiplus::Bitmap(newConfig->szBalloonIconPath).GetHICON(&this->balloonIcon);

        if (isNew)
        {
            this->pNotifyIcon->PopupIconInfo(L"图标修改成功", L"来看看效果吧~", this->balloonIcon, TRUE);
        }
    }

    // 设置浮动窗口
    if (!isNew || newConfig->bFloatWnd != currentConfig->bFloatWnd)
    {
        ShowWindow(this->hWnd, newConfig->bFloatWnd ? SW_SHOWNA : SW_HIDE);
    }

    // 调整显示内容
    if (!isNew || newConfig->byShowContent != currentConfig->byShowContent)
    {
        SIZE sizeWnd = { 0 };
        GetWndSizeByShowContent(&sizeWnd, newConfig->byShowContent);
        SetWindowPos(this->hWnd, HWND_TOPMOST, 0, 0, sizeWnd.cx, sizeWnd.cy, SWP_NOACTIVATE | SWP_NOMOVE);
    }

    // 设置透明度
    if (!isNew || newConfig->transparencyPercent != currentConfig->transparencyPercent)
    {
        SetLayeredWindowAttributes(this->hWnd, 0, PercentToAlpha(newConfig->transparencyPercent), LWA_ALPHA);
    }

    // 设置开机自启
    if (!isNew || newConfig->bAutoRun != currentConfig->bAutoRun)
    {
        if (newConfig->bAutoRun)
        {
            SetAppAutoRun(WinApp::GetName());
        }
        else
        {
            UnsetAppAutoRun(WinApp::GetName());
        }
    }

    // 整点报时
    if (!isNew || newConfig->bTimeAlarm != currentConfig->bTimeAlarm)
    {
        if (newConfig->bTimeAlarm)
        {
            SetTimer(this->hWnd, IDT_TIMEALARM, 500, (TIMERPROC)NULL);
        }
        else
        {
            KillTimer(this->hWnd, IDT_TIMEALARM);
        }
    }

    // 重绘一次
    InvalidateRect(this->hWnd, NULL, TRUE);

    return TRUE;
}

void MainWindow::GetWndSizeByShowContent(PSIZE psizeWnd, BYTE byShowContent)
{
    psizeWnd->cx = this->wndSizeUnit * 6;
    psizeWnd->cy = 0;
    if (byShowContent & SHOWCONTENT_CPUMEM)
    {
        psizeWnd->cy += this->wndSizeUnit * 3;
    }
    if (byShowContent & SHOWCONTENT_NETSPEED)
    {
        psizeWnd->cy += this->wndSizeUnit * 2;
    }
}

BOOL MainWindow::TimeAlarm()
{
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);

    WCHAR szInfo[MAX_NIDINFO] = { 0 };
    StringCchPrintfW(szInfo, MAX_NIDINFO, L"北京时间 %02d: %02d", st.wHour, st.wMinute);

    return this->pNotifyIcon->PopupIconInfo(L"Take a break~", szInfo, this->balloonIcon, AppConfig::Get()->bInfoSound);
}

BOOL MainWindow::CheckAndTimeAlarm()
{
    // 是否整点内
    if (IsOnTheHour())
    {
        // 是否已报过时
        if (!this->bClocked)
        {
            this->bClocked = TRUE;
            this->TimeAlarm();
        }
    }
    else
    {
        // 非整点清空报时记录
        this->bClocked = FALSE;
    }
    return TRUE;
}

INT MainWindow::ShowNoConentWarningMsg()
{
    return MessageBoxW(this->hWnd, L"至少保留一项显示内容！", L"提示信息", MB_ICONINFORMATION);
}

///////////////////////////// Message Process ////////////////////////////////

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return this->OnCreate(wParam, lParam);
    case WM_DESTROY:
        return this->OnDestroy(wParam, lParam);
    case WM_ACTIVATE:
        return this->OnActivate(wParam, lParam);
    case WM_CLOSE:
        return this->OnClose(wParam, lParam);
    case WM_PAINT:
        return this->OnPaint(wParam, lParam);
    case WM_SETTINGCHANGE:
        return this->OnSettingChange(wParam, lParam);
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
    case WM_DPICHANGED:
        return this->OnDpiChanged(wParam, lParam);
    case WM_NOTIFYICON:
        return this->OnNotifyIcon(wParam, lParam);
    case WM_PERFDATAUPDATED:
        return this->OnPerfDataUpdated(wParam, lParam);
    default:
        if (uMsg == this->uMsgTaskbarCreated)
            return this->OnTaskbarCreated(wParam, lParam);
        else
            return DefWindowProcW(this->hWnd, uMsg, wParam, lParam);
    }
}

LRESULT MainWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
    GetSysDragSize(&this->sysDragSize);

    // 设置窗口类样式, 增加阴影边框
    SetClassLongPtrW(this->hWnd, GCL_STYLE, GetClassLongPtrW(this->hWnd, GCL_STYLE) | CS_DROPSHADOW);

    // 注册监视器
    PerfMonitor::RegisterMessage(this->hWnd, WM_PERFDATAUPDATED);

    // DPI 相关
    this->wndSizeUnit = BASE_WNDSIZE_PIXELS * GetDpiForWindow(this->hWnd) / 96;

    // 初始化字体
    this->fontColl.AddFontFile(this->GetFontPath());

    // 添加图标
    this->pNotifyIcon = new NotifyIcon(this->hWnd, ID_NIDMAIN);
    this->pNotifyIcon->Add(WM_NOTIFYICON, this->LoadNotifyIconBySysTheme(), WinApp::GetName());

    // 初始化窗口位置
    POINT wndPos = { 0 };
    this->LoadLastPosFromReg(&wndPos);
    SetWindowPos(this->hWnd, HWND_TOPMOST, wndPos.x, wndPos.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);

    // 应用配置项
    this->ApplyConfig();

    // 如果由于某些未知因素导致保存的位置出了桌面, 则重置位置
    if (this->IsIntersectDesktop())
    {
        this->GetDefaultWindowPos(&wndPos);
        SetWindowPos(this->hWnd, HWND_TOPMOST, wndPos.x, wndPos.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
        this->SaveCurrentPosToReg();
    }

    // 创建 sprite 窗口
    this->spritewnd = new SpriteWindow;
    this->spritewnd->CreateWindow_();

    return 0;
}

LRESULT MainWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    DestroyWindow(this->spritewnd->GetWindowHandle());
    this->spritewnd = NULL;

    this->SaveCurrentPosToReg();
    AppConfig::SaveToFile(WinApp::GetConfigPath());

    this->pNotifyIcon->Delete();
    delete this->pNotifyIcon;
    this->pNotifyIcon = NULL;

    DestroyIcon(this->balloonIcon);

    PerfMonitor::UnregisterMessage(this->hWnd);

    PostQuitMessage(EXIT_SUCCESS);
    return 0;
}

LRESULT MainWindow::OnActivate(WPARAM wParam, LPARAM lParam)
{
    // 不是浮动且处于固定并失去激活
    if (!AppConfig::Get()->bFloatWnd && this->bWndFixed && !wParam)
    {
        this->bWndFixed = FALSE;
        // ShowWindow(this->hWnd, SW_HIDE); // 不能在 WM_ACTIVATE 里用这个, 有 bug, 会丢失鼠标按下的消息, 见 http://www.cnblogs.com/cswuyg/archive/2012/08/20/2647445.html

        this->PopupClose();
    }
    return 0;
}

LRESULT MainWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
    // 屏蔽普通退出, 只能通过菜单退出
    return 0;
}

LRESULT MainWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
    // 得到性能数据
    const INT dataBufferLen = 16;
    WCHAR szDataBuffer[dataBufferLen] = { 0 };     // 字符串缓冲区
    INT nLevel = 0;
    Gdiplus::Color statusColor;

    // 开始绘图
    PAINTSTRUCT ps = { 0 };
    HDC hdc = BeginPaint(this->hWnd, &ps);

    Gdiplus::REAL sizeUnit = (FLOAT)this->wndSizeUnit;

    // 得到绘图窗体大小
    RECT rcClient;
    GetClientRect(this->hWnd, &rcClient);
    Gdiplus::SizeF sizeClient((FLOAT)(rcClient.right - rcClient.left), (FLOAT)(rcClient.bottom - rcClient.top));

    // 使用缓冲区绘图
    Gdiplus::Bitmap* pBmpMem = new Gdiplus::Bitmap((INT)sizeClient.Width, (INT)sizeClient.Height);
    Gdiplus::Graphics graphicsMem(pBmpMem);

    // 设置绘图模式
    graphicsMem.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);                       // 图形渲染抗锯齿
    graphicsMem.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);        // 文字渲染抗锯齿

    // 绘图属性对象
    Gdiplus::SizeF drawSize(sizeUnit * 3, sizeUnit * 3);                 // 绘制矩形
    Gdiplus::Pen pen(Gdiplus::Color::Green, sizeUnit / 6);                                                   // 图形颜色
    Gdiplus::SolidBrush textbrush(AppConfig::Get()->bDarkTheme ? Gdiplus::Color::White : Gdiplus::Color::Black);   // 文本颜色
    Gdiplus::SolidBrush bgbrush(AppConfig::Get()->bDarkTheme ? Gdiplus::Color::Black : Gdiplus::Color::White);     // 背景颜色

    // 从容器中创建要使用的字体
    Gdiplus::FontFamily fontFamily;
    INT found = 0;
    this->fontColl.GetFamilies(1, &fontFamily, &found);
    Gdiplus::Font textFont(&fontFamily, sizeUnit * 2 / 3, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);             // 字体大小用像素值衡量

    Gdiplus::StringFormat strformat(Gdiplus::StringFormatFlagsNoClip);                            // 文字居于矩形中心
    strformat.SetAlignment(Gdiplus::StringAlignmentCenter);
    strformat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    // 刷黑背景
    graphicsMem.FillRectangle(&bgbrush, 0., 0., sizeClient.Width, sizeClient.Height);

    // 绘制CPU与MEM
    Gdiplus::PointF circleCenter;
    if (AppConfig::Get()->byShowContent & SHOWCONTENT_CPUMEM)
    {
        drawSize.Width = sizeUnit * 3;
        drawSize.Height = sizeUnit * 3;
        graphicsMem.TranslateTransform(0, 0);

        // CPU
        StringCchPrintfW(szDataBuffer, dataBufferLen, L"C:%.0f%%", this->perfData.cpuPercent);
        if (this->perfData.cpuPercent < 50)
        {
            pen.SetColor(STATUSCOLOR_GOOD);
        }
        else if (this->perfData.cpuPercent < 75)
        {
            pen.SetColor(STATUSCOLOR_NORMAL);
        }
        else
        {
            pen.SetColor(STATUSCOLOR_BAD);
        }
        graphicsMem.DrawString(
            szDataBuffer, -1, &textFont,
            Gdiplus::RectF(Gdiplus::PointF(0, 0), drawSize),
            &strformat, &textbrush
        );

        circleCenter = { drawSize.Width / 2, drawSize.Height / 2 };
        DrawCircle(
            graphicsMem, pen,
            circleCenter,
            drawSize.Height / 2 - sizeUnit * 0.3f,
            (FLOAT)(this->perfData.cpuPercent / 100)
        );

        // 绘制内存
        StringCchPrintfW(szDataBuffer, dataBufferLen, L"M:%.0f%%", this->perfData.memPercent);
        if (this->perfData.memPercent < 75)
        {
            pen.SetColor(STATUSCOLOR_GOOD);
        }
        else if (this->perfData.memPercent < 90)
        {
            pen.SetColor(STATUSCOLOR_NORMAL);
        }
        else
        {
            pen.SetColor(STATUSCOLOR_BAD);
        }
        graphicsMem.DrawString(
            szDataBuffer, -1, &textFont,
            Gdiplus::RectF(Gdiplus::PointF(sizeUnit * 3, 0), drawSize),
            &strformat, &textbrush
        );

        circleCenter = { drawSize.Width / 2 + sizeUnit * 3, drawSize.Height / 2 };
        DrawCircle(
            graphicsMem, pen,
            circleCenter,
            drawSize.Height / 2 - sizeUnit * 0.3f,
            (FLOAT)(this->perfData.memPercent / 100)
        );
    }

    // 绘制网速
    Gdiplus::RectF rectSpeed;
    if (AppConfig::Get()->byShowContent & SHOWCONTENT_NETSPEED)
    {
        drawSize.Width = sizeUnit * 3;
        drawSize.Height = sizeUnit * 1;
        graphicsMem.TranslateTransform(0, 0);

        if (AppConfig::Get()->byShowContent & SHOWCONTENT_CPUMEM)
        {
            graphicsMem.TranslateTransform(0, sizeUnit * 3);
        }

        // 绘制上传
        nLevel = ConvertSpeed(this->perfData.uploadSpeed, szDataBuffer, dataBufferLen);
        if (nLevel < 3)
        {
            statusColor = STATUSCOLOR_BAD;
        }
        else if (nLevel < 5)
        {
            statusColor = STATUSCOLOR_NORMAL;
        }
        else
        {
            statusColor = STATUSCOLOR_GOOD;
        }
        graphicsMem.DrawString(
            szDataBuffer, -1, &textFont,
            Gdiplus::RectF(Gdiplus::PointF(0, sizeUnit), drawSize),
            &strformat, &textbrush
        );

        rectSpeed = { sizeUnit * 0.4f, sizeUnit * 0.08f, drawSize.Width - sizeUnit * 0.8f, drawSize.Height - sizeUnit * 0.16f };
        DrawSpeedStair(graphicsMem, statusColor, rectSpeed, TRUE, nLevel);

        // 绘制下载
        nLevel = ConvertSpeed(this->perfData.downloadSpeed, szDataBuffer, dataBufferLen);
        if (nLevel < 3)
        {
            statusColor = STATUSCOLOR_BAD;
        }
        else if (nLevel < 5)
        {
            statusColor = STATUSCOLOR_NORMAL;
        }
        else
        {
            statusColor = STATUSCOLOR_GOOD;
        }
        graphicsMem.DrawString(szDataBuffer, -1, &textFont, Gdiplus::RectF(Gdiplus::PointF(sizeUnit * 3, sizeUnit), drawSize), &strformat, &textbrush);

        rectSpeed = { sizeUnit * 3.4f, sizeUnit * 0.08f, drawSize.Width - sizeUnit * 0.8f, drawSize.Height - sizeUnit * 0.16f };
        DrawSpeedStair(graphicsMem, statusColor, rectSpeed, FALSE, nLevel);
    }

    // 拷贝缓存图, 结束绘图
    Gdiplus::Graphics graphics(hdc);
    Gdiplus::CachedBitmap* pCachedBmp = new Gdiplus::CachedBitmap(pBmpMem, &graphics);
    graphics.DrawCachedBitmap(pCachedBmp, 0, 0);
    delete pCachedBmp;
    delete pBmpMem;

    EndPaint(this->hWnd, &ps);
    return 0;
}

LRESULT MainWindow::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
    // 自动调节图标颜色
    this->pNotifyIcon->ModifyIcon(this->LoadNotifyIconBySysTheme());
    return 0;
}

LRESULT MainWindow::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    // 只在显示浮窗的时候才弹出右键菜单
    if (AppConfig::Get()->bFloatWnd)
    {
        this->ShowContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    return 0;
}

LRESULT MainWindow::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
    //SIZE newResolution = { 0 };
    //newResolution.cx = LOWORD(lParam);
    //newResolution.cy = HIWORD(lParam);

    // 如果分辨率变了之后出桌面就重置到默认位置
    if (this->IsIntersectDesktop())
    {
        POINT wndPos = { 0 };
        this->GetDefaultWindowPos(&wndPos);
        SetWindowPos(this->hWnd, HWND_TOPMOST, wndPos.x, wndPos.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
        this->SaveCurrentPosToReg();
    }

    return 0;
}

LRESULT MainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) Menu: FALSE, Accelerator: TRUE
    // LOWORD(wParam) identifier
    // lParam: 0
    BOOL configChanged = FALSE;

    AppConfig::AppConfig* pcfgdata = new AppConfig::AppConfig(*AppConfig::Get());

    switch (LOWORD(wParam))
    {
    // 浮动窗口
    case IDM_FLOATWND:
        pcfgdata->bFloatWnd = (BOOL)!pcfgdata->bFloatWnd;
        configChanged = TRUE;
        break;
    // 显示子菜单
    case IDM_SHOWCPUMEM:
        if (pcfgdata->byShowContent == SHOWCONTENT_CPUMEM)
        {
            this->ShowNoConentWarningMsg();
        }
        else
        {
            pcfgdata->byShowContent ^= SHOWCONTENT_CPUMEM;
        }
        configChanged = TRUE;
        break;
    case IDM_SHOWNETSPEED:
        if (pcfgdata->byShowContent == SHOWCONTENT_NETSPEED)
        {
            this->ShowNoConentWarningMsg();
        }
        else
        {
            pcfgdata->byShowContent ^= SHOWCONTENT_NETSPEED;
        }
        configChanged = TRUE;
        break;
    case IDM_TIMEALARM:
        pcfgdata->bTimeAlarm = (BOOL)!pcfgdata->bTimeAlarm;
        configChanged = TRUE;
        break;
    case IDM_INFOSOUND:
        pcfgdata->bInfoSound = (BOOL)!pcfgdata->bInfoSound;
        configChanged = TRUE;
        break;
    case IDM_SHOWSPRITE:
        pcfgdata->bShowSprite = (BOOL)!pcfgdata->bShowSprite;
        configChanged = TRUE;
        break;
    case IDM_SPMOUSEPASS:
        pcfgdata->bSpriteMousePass = (BOOL)!pcfgdata->bSpriteMousePass;
        configChanged = TRUE;
        break;
    case IDM_RESETSPRITEPOS:
        this->spritewnd->GetSpineChar()->SetPosition(0, 0);
        this->spritewnd->SaveCurrentPosToReg();
        break;
    case IDM_CONFIG:
    {
        if (!this->configDlg)
        {
            this->configDlg = new ConfigDlg(this);
            this->configDlg->SetFormData(AppConfig::Get());
            if (this->configDlg->ShowDialogBox(GetModuleHandleW(NULL), NULL))
            {
                this->configDlg->GetFormData(pcfgdata);
                configChanged = TRUE;
            }
            delete this->configDlg;
            this->configDlg = NULL;
        }
        else
        {
            this->configDlg->FlashAndBeep();
        }
        break;
    }
    case IDM_ABOUT:
    {
        if (!this->aboutDlg)
        {
            this->aboutDlg = new AboutDlg();
            this->aboutDlg->ShowDialogBox(GetModuleHandleW(NULL), NULL);
            delete this->aboutDlg;
            this->aboutDlg = NULL;
        }
        else
        {
            this->aboutDlg->FlashAndBeep();
        }
        break;
    }
    case IDM_EXIT:
        DestroyWindow(this->hWnd);
        break;
    default:
        delete pcfgdata;
        return DefWindowProcW(this->hWnd, WM_COMMAND, wParam, lParam);
    }

    // 应用更改
    if (configChanged)
    {
        this->ApplyConfig(pcfgdata);
        this->spritewnd->ApplyConfig(pcfgdata);
        AppConfig::Set(pcfgdata);
        AppConfig::SaveToFile(WinApp::GetConfigPath());
    }

    delete pcfgdata;
    return 0;
}

LRESULT MainWindow::OnTimer(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case IDT_TIMEALARM:
        this->CheckAndTimeAlarm();
        break;
    default:
        return DefWindowProcW(this->hWnd, WM_TIMER, wParam, lParam);
    }
    return 0;
}

LRESULT MainWindow::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
    // 获取菜单句柄
    HMENU hMenu = (HMENU)wParam;

    // 设置菜单状态
    SetMenuItemState(hMenu, IDM_FLOATWND, FALSE, (AppConfig::Get()->bFloatWnd ? MFS_CHECKED : MFS_UNCHECKED) | (this->bWndFixed ? MFS_DISABLED : MFS_ENABLED));
    SetMenuItemState(hMenu, IDM_SHOWCPUMEM, FALSE, ((AppConfig::Get()->byShowContent & SHOWCONTENT_CPUMEM) ? MFS_CHECKED : MFS_UNCHECKED) | (this->bWndFixed ? MFS_DISABLED : MFS_ENABLED));
    SetMenuItemState(hMenu, IDM_SHOWNETSPEED, FALSE, ((AppConfig::Get()->byShowContent & SHOWCONTENT_NETSPEED) ? MFS_CHECKED : MFS_UNCHECKED) | (this->bWndFixed ? MFS_DISABLED : MFS_ENABLED));

    SetMenuItemState(hMenu, IDM_TIMEALARM, FALSE, AppConfig::Get()->bTimeAlarm ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_INFOSOUND, FALSE, AppConfig::Get()->bInfoSound ? MFS_CHECKED : MFS_UNCHECKED);

    SetMenuItemState(hMenu, IDM_SHOWSPRITE, FALSE, AppConfig::Get()->bShowSprite ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_SPMOUSEPASS, FALSE, AppConfig::Get()->bSpriteMousePass ? MFS_CHECKED : MFS_UNCHECKED);

    return 0;
}

LRESULT MainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    INT deltaX = ptCursor.x - this->ptDragSrc.x;
    INT deltaY = ptCursor.y - this->ptDragSrc.y;
    if (wParam & MK_LBUTTON)
    {
        if (AppConfig::Get()->bFloatWnd)
        {
            if (!this->isDragging && (abs(deltaX) >= this->sysDragSize.cx || abs(deltaY) >= this->sysDragSize.cy))
            {
                this->isDragging = TRUE;
            }
            if (this->isDragging)
            {
                RECT rcWnd = { 0 };
                GetWindowRect(this->hWnd, &rcWnd);
                SetWindowPos(this->hWnd, HWND_TOPMOST, rcWnd.left + deltaX, rcWnd.top + deltaY, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
            }
        }
    }
    return 0;
}

LRESULT MainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    SetCapture(this->hWnd); // 防止鼠标跟丢

    // 保存点击位置
    this->ptDragSrc.x = GET_X_LPARAM(lParam);
    this->ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
}

LRESULT MainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    ReleaseCapture();

    if (AppConfig::Get()->bFloatWnd)
    {
        this->isDragging = FALSE;

        // 保存一次现在的窗口位置
        this->SaveCurrentPosToReg();
    }
    return 0;
}

LRESULT MainWindow::OnLButtonDBClick(WPARAM wParam, LPARAM lParam)
{
    SetCapture(this->hWnd); // 防止鼠标跟丢

    // 保存点击位置
    this->ptDragSrc.x = GET_X_LPARAM(lParam);
    this->ptDragSrc.y = GET_Y_LPARAM(lParam);

    // TODO: 转向

    return 0;
}

LRESULT MainWindow::OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
    // 清单文件里需要将程序的 DPI Awareness 设置为 Per Monitor High DPI Aware 才能接收此消息
    // 重新计算窗体大小
    this->wndSizeUnit = BASE_WNDSIZE_PIXELS * GetDpiForWindow(this->hWnd) / 96;
    SIZE sizeWnd = { 0 };
    this->GetWndSizeByShowContent(&sizeWnd, AppConfig::Get()->byShowContent);

    // 设置新的窗体大小
    SetWindowPos(this->hWnd, HWND_TOPMOST, 0, 0, sizeWnd.cx, sizeWnd.cy, SWP_NOACTIVATE | SWP_NOMOVE);

    // 重绘窗体内容
    InvalidateRect(this->hWnd, NULL, TRUE);

    return 0;
}

LRESULT MainWindow::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(lParam))
    {
    case WM_LBUTTONDBLCLK:
#ifdef _DEBUG
    {
        ShowWindow(this->spritewnd->GetWindowHandle(), SW_SHOW);
        InvalidateRect(this->spritewnd->GetWindowHandle(), NULL, TRUE);
        OutputDebugStringW(L"Double Click on NotifyIcon!\n");
    }
#endif // !_DEBUG
        break;
    case WM_CONTEXTMENU:
        this->ShowContextMenu(GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam)); // 右键通知区域图标始终弹出右键菜单
        break;
    case NIN_SELECT:
    case NIN_KEYSELECT:
        if (!AppConfig::Get()->bFloatWnd && !this->bWndFixed)
        {
            this->bWndFixed = TRUE; // 固定住窗口显示
            this->PopupOpen();
            SetForegroundWindow(this->hWnd); // 只有点击了图标才需要设置前景窗口
        }
        break;
    case NIN_POPUPOPEN:
        // 不是浮动且没有固定则显示弹窗
        if (!AppConfig::Get()->bFloatWnd && !this->bWndFixed)
        {
            this->PopupOpen();
        }
        break;
    case NIN_POPUPCLOSE:
        // 不是浮动且没有固定则隐藏弹窗
        if (!AppConfig::Get()->bFloatWnd && !this->bWndFixed)
        {
            this->PopupClose();
        }
        break;
    default:
        return DefWindowProcW(this->hWnd, WM_NOTIFYICON, wParam, lParam);
    }
    return 0;
}

LRESULT MainWindow::OnPerfDataUpdated(WPARAM wParam, LPARAM lParam)
{
    ((PerfMonitor::PerfMonitor*)lParam)->GetPerfData(&this->perfData);
    InvalidateRect(this->hWnd, NULL, TRUE);
    return 0;
}

LRESULT MainWindow::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
    // 添加图标
    this->pNotifyIcon->Add(WM_NOTIFYICON, LoadNotifyIconBySysTheme(), WinApp::GetName());
    return 0;
}

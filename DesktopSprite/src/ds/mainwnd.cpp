#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/config.h>
#include <ds/notifyicon.h>
#include <ds/perfmonitor.h>
#include <ds/transdlg.h>
#include <ds/winapp.h>

#include <ds/mainwnd.h>

// 状态颜色
Gdiplus::Color const STATUSCOLOR_LOW = 0xff00ff00;
Gdiplus::Color const STATUSCOLOR_MIDDLE = 0xffff8000;
Gdiplus::Color const STATUSCOLOR_HIGH = 0xffff0000;

// 注册表键值
PCWSTR const REGVAL_LASTFLOATPOS = L"LastFloatPos";
PCWSTR const REGVAL_LASTRUNTIMERESOLUTION = L"LastRunTimeResolution";

using namespace Gdiplus;

// 常量定义
static UINT     const   REFRESHINTERVAL = 1000;                     // 屏幕显示刷新间隔
static UINT     const   ID_NIDMAIN = 1;                        // 图标 ID
static UINT     const   BASE_WNDSIZE_PIXELS = 20;                       // 主窗口的基本单元格像素大小

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

Font* CreateFontFromFile(PCWSTR szFontFilePath, REAL emSize, INT style, Unit unit)
{
    PrivateFontCollection prvtFontColl;
    prvtFontColl.AddFontFile(szFontFilePath);
    FontFamily fontFamily;
    INT found = 0;
    prvtFontColl.GetFamilies(1, &fontFamily, &found);
    Font* pFont = new Font(&fontFamily, emSize, style, unit);
    return pFont;
}

///////////////////////////////////////////////////////////////////


PCWSTR MainWindow::GetClassName_() const
{
    return L"DesktopSpriteMainWndClass";
}

PCWSTR MainWindow::GetConfigPath() const
{
    return L"config.json";
}

MainWindow::MainWindow(const WinApp* app)
{
    this->app = app;

    // 注册任务栏重建消息
    this->uMsgTaskbarCreated = RegisterWindowMessageW(SZMSG_TASKBARCREATED);

    // 注册窗体
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.lpszClassName = this->GetClassName_();
    wcex.lpfnWndProc = MainWindow::WindowProc;
    wcex.hInstance = GetModuleHandleW(NULL);
    wcex.style = CS_DBLCLKS | CS_DROPSHADOW | CS_HREDRAW | CS_VREDRAW;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.hIconSm = NULL;
    ATOM tmp = RegisterClassExW(&wcex);

    // 创建窗口
    HINSTANCE hInst = GetModuleHandleW(NULL);
    this->hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED, 
        this->GetClassName_(), 
        NULL, WS_POPUP, 
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
        NULL, NULL, 
        hInst, (LPVOID)this
    );

    if (!this->hWnd)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }
}

DWORD MainWindow::LoadFloatPosDataFromReg()
{
    // 默认主窗口位置是屏幕的 1/6 处
    GetScreenResolution(&this->lastResolution);
    this->lastFloatPos.x = this->lastResolution.cx * 5 / 6;
    this->lastFloatPos.y = this->lastResolution.cy * 1 / 6;

    DWORD dwErrorCode = ERROR_SUCCESS;

    // 打开注册表项
    HKEY hkApp = NULL;
    DWORD dwDisposition = 0;
    DWORD cbData = 0;

    WCHAR subkey[128] = { 0 };
    StringCchPrintfW(subkey, 128, L"SOFTWARE\\%s", this->app->GetAppName());

    dwErrorCode = RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, &dwDisposition);
    if (dwErrorCode == ERROR_SUCCESS && dwDisposition == REG_OPENED_EXISTING_KEY)
    {
        cbData = sizeof(POINT);
        RegQueryAnyValue(hkApp, REGVAL_LASTFLOATPOS, (PBYTE)&this->lastFloatPos, &cbData);
        cbData = sizeof(SIZE);
        RegQueryAnyValue(hkApp, REGVAL_LASTRUNTIMERESOLUTION, (PBYTE)&this->lastResolution, &cbData);
    }
    else
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
    }

    // 关闭注册表
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    return dwErrorCode;
}

DWORD MainWindow::UpdateFloatPosDataToReg(PPOINT newPos, PSIZE newResolution)
{
    CopyPoint(newPos, &this->lastFloatPos);
    CopySize(newResolution, &this->lastResolution);

    DWORD dwErrorCode = ERROR_SUCCESS;

    // 打开注册表项
    HKEY hkApp = NULL;
    DWORD dwDisposition = 0;
    DWORD cbData = 0;

    WCHAR subkey[128] = { 0 };
    StringCchPrintfW(subkey, 128, L"SOFTWARE\\%s", this->app->GetAppName());

    dwErrorCode = RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, &dwDisposition);
    if (dwErrorCode == ERROR_SUCCESS && dwDisposition == REG_OPENED_EXISTING_KEY)
    {
        RegSetBinValue(hkApp, REGVAL_LASTFLOATPOS, (PBYTE)&this->lastFloatPos, sizeof(POINT));
        RegSetBinValue(hkApp, REGVAL_LASTRUNTIMERESOLUTION, (PBYTE)&this->lastResolution, sizeof(SIZE));
    }
    else
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
    }

    // 关闭注册表
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    return dwErrorCode;

}

DWORD MainWindow::ApplyConfig()
{
    // 是否显示浮动窗口
    if (this->config.bFloatWnd)
    {
        SetWindowPos(this->hWnd, HWND_TOPMOST, this->lastFloatPos.x, this->lastFloatPos.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
        ShowWindow(this->hWnd, SW_SHOWNA);
    }
    else
    {
        ShowWindow(this->hWnd, SW_HIDE);
    }

    // 设置透明度
    SetLayeredWindowAttributes(this->hWnd, 0, PercentToAlpha(this->config.transparencyPercent), LWA_ALPHA);

    // 开机启动
    if (this->config.bAutoRun)
    {
        SetAppAutoRun(this->app->GetAppName());
    }
    else
    {
        UnsetAppAutoRun(this->app->GetAppName());
    }

    // 是否整点报时
    if (this->config.bTimeAlarm)
    {
        SetTimer(this->hWnd, IDT_TIMEALARM, 100, (TIMERPROC)NULL);
    }
    else
    {
        KillTimer(this->hWnd, IDT_TIMEALARM);
    }

    // 重绘一次
    InvalidateRect(this->hWnd, NULL, TRUE);

    this->config.SaveToReg(this->app->GetAppName());

    return 0;
}

DWORD MainWindow::ApplyConfig(PCFGDATA pcfgdata)
{
    // 设置浮动窗口
    if (pcfgdata->bFloatWnd != this->config.bFloatWnd)
    {
        if (pcfgdata->bFloatWnd)
        {
            SetWindowPos(this->hWnd, HWND_TOPMOST, this->lastFloatPos.x, this->lastFloatPos.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
            ShowWindow(this->hWnd, SW_SHOWNA);
        }
        else
        {
            ShowWindow(this->hWnd, SW_HIDE);
        }
    }

    // 调整显示内容
    if (pcfgdata->byShowContent != this->config.byShowContent)
    {
        SIZE sizeWnd = { 0 };
        GetWndSizeByShowContent(&sizeWnd, pcfgdata->byShowContent);
        SetWindowPos(this->hWnd, HWND_TOPMOST, 0, 0, sizeWnd.cx, sizeWnd.cy, SWP_NOACTIVATE | SWP_NOMOVE);
    }

    // 设置透明度
    if (pcfgdata->transparencyPercent != this->config.transparencyPercent)
    {
        SetLayeredWindowAttributes(this->hWnd, 0, PercentToAlpha(pcfgdata->transparencyPercent), LWA_ALPHA);
    }

    // 设置开机自启
    if (pcfgdata->bAutoRun != this->config.bAutoRun)
    {
        if (pcfgdata->bAutoRun)
        {
            SetAppAutoRun(this->app->GetAppName());
        }
        else
        {
            UnsetAppAutoRun(this->app->GetAppName());
        }
    }

    // 整点报时
    if (pcfgdata->bTimeAlarm != this->config.bTimeAlarm)
    {
        if (pcfgdata->bTimeAlarm)
        {
            SetTimer(this->hWnd, IDT_TIMEALARM, 100, (TIMERPROC)NULL);
        }
        else
        {
            KillTimer(this->hWnd, IDT_TIMEALARM);
        }
    }

    // 重绘一次
    InvalidateRect(this->hWnd, NULL, TRUE);

    this->config.Set(pcfgdata);
    this->config.SaveToReg(this->app->GetAppName());

    return 0;
}

DWORD MainWindow::GetWndSizeByShowContent(PSIZE psizeWnd, BYTE byShowContent)
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
    return 0;
}

DWORD MainWindow::TimeAlarm()
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
    this->pNotifyIcon->PopIconInfo(szInfoTitle, szInfo, hIcon, this->config.bInfoSound);

    DestroyIcon(hIcon);

    return 0;
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
        //case WM_POWERBROADCAST: // XXX: 收不到这个消息
        //    return this->OnPowerBroadcast(wParam, lParam);
    case WM_DPICHANGED:
        return this->OnDpiChanged(wParam, lParam);
    case WM_NOTIFYICON:
        return this->OnNotifyIcon(wParam, lParam);
    case WM_TIMEALARM:
        return this->OnTimeAlarm(wParam, lParam);
        //case WM_MEASUREITEM:
        //case WM_DRAWITEM:
        //    return 0;
    default:
        if (uMsg == this->uMsgTaskbarCreated)
            return this->OnTaskbarCreated(wParam, lParam);
        else
            return DefWindowProcW(this->hWnd, uMsg, wParam, lParam);
    }
}

LRESULT MainWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
    // 初始化配置参数相关
    this->config.LoadFromReg(this->app->GetAppName());
    //if (PathFileExistsW(this->GetConfigPath()))
    //{
    //    this->config.LoadFromFile(this->GetConfigPath());
    //}

    // DPI 相关
    this->wndSizeUnit = BASE_WNDSIZE_PIXELS * GetDpiForWindow(this->hWnd) / 96;

    // 设置窗口位置与大小, 获取当前屏幕分辨率, 并折算上一次运行时保存的窗口坐标到新分辨率
    this->LoadFloatPosDataFromReg();

    SIZE sizeWnd = { 0 };
    POINT point = { 0 };
    SIZE resolution = { 0 };
    this->GetWndSizeByShowContent(&sizeWnd, this->config.byShowContent);
    GetScreenResolution(&resolution);
    ConvertPointForResolution(&this->lastFloatPos, &this->lastResolution, &resolution, &point);
    SetWindowPos(this->hWnd, HWND_TOPMOST, point.x, point.y, sizeWnd.cx, sizeWnd.cy, SWP_NOACTIVATE);

    // 记录新的位置和分辨率
    this->UpdateFloatPosDataToReg(&point, &resolution);

    // 向字体容器添加私有字体
    DWORD cbData = 0;
    PBYTE pFontData = GetResPointer(IDR_TTF1, L"TTF", &cbData);
    this->fontColl.AddMemoryFont((PVOID)pFontData, (INT)cbData);

    // 添加图标
    this->pNotifyIcon = new NotifyIcon(
        this->hWnd, ID_NIDMAIN, WM_NOTIFYICON,
        LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK))
    );

    // 设置图标提示信息
    WCHAR szTip[MAX_NIDTIP] = { 0 };
    LoadStringW(GetModuleHandleW(NULL), IDS_APPNAME, szTip, MAX_NIDTIP);
    this->pNotifyIcon->SetTip(szTip);

    // 应用配置项
    this->ApplyConfig();

    // 每 1s 刷新一次显示
    SetTimer(this->hWnd, IDT_REFRESHRECT, REFRESHINTERVAL, (TIMERPROC)NULL);

    return 0;
}

LRESULT MainWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    this->config.SaveToReg(this->app->GetAppName());
    delete this->pNotifyIcon;
    PostQuitMessage(EXIT_SUCCESS);
    return 0;
}

LRESULT MainWindow::OnActivate(WPARAM wParam, LPARAM lParam)
{
    // 不是浮动且失去激活
    if (!this->config.bFloatWnd && !wParam)
    {
        this->bWndFixed = FALSE;
        ShowWindow(this->hWnd, SW_HIDE);
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
    PERFDATA perfData = { 0 };
    this->perfMonitor.GetPerfData(&perfData);
    WCHAR szDataBuffer[16] = { 0 };     // 字符串缓冲区
    INT nLevel = 0;
    Color statusColor;

    // 开始绘图
    PAINTSTRUCT ps = { 0 };
    HDC hdc = BeginPaint(this->hWnd, &ps);

    REAL sizeUnit = (REAL)this->wndSizeUnit;

    // 得到绘图窗体大小
    RECT rcClient;
    GetClientRect(this->hWnd, &rcClient);
    SizeF sizeClient(REAL(rcClient.right - rcClient.left), REAL(rcClient.bottom - rcClient.top));

    // 使用缓冲区绘图
    Bitmap* pBmpMem = new Bitmap((INT)sizeClient.Width, (INT)sizeClient.Height);
    Graphics graphicsMem(pBmpMem);

    // 设置绘图模式
    graphicsMem.SetSmoothingMode(SmoothingModeAntiAlias);                       // 图形渲染抗锯齿
    graphicsMem.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);        // 文字渲染抗锯齿

    // 绘图属性对象
    SizeF drawSize(sizeUnit * 3, sizeUnit * 3);                 // 绘制矩形
    Pen pen(Color::Green, sizeUnit / 6);                                                   // 图形颜色
    SolidBrush textbrush(this->config.bDarkTheme ? Color::White : Color::Black);   // 文本颜色
    SolidBrush bgbrush(this->config.bDarkTheme ? Color::Black : Color::White);     // 背景颜色

    // 从容器中创建要使用的字体
    FontFamily fontFamily;
    INT found = 0;
    this->fontColl.GetFamilies(1, &fontFamily, &found);
    Font textFont(&fontFamily, sizeUnit * 2 / 3, FontStyleRegular, UnitPixel);             // 字体大小用像素值衡量


    StringFormat strformat(StringFormatFlagsNoClip);                            // 文字居于矩形中心
    strformat.SetAlignment(StringAlignmentCenter);
    strformat.SetLineAlignment(StringAlignmentCenter);

    // 刷黑背景
    graphicsMem.FillRectangle(&bgbrush, 0., 0., sizeClient.Width, sizeClient.Height);

    // 绘制CPU与MEM
    if (this->config.byShowContent & SHOWCONTENT_CPUMEM)
    {
        drawSize.Width = sizeUnit * 3;
        drawSize.Height = sizeUnit * 3;
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
            szDataBuffer, -1, &textFont,
            RectF(PointF(0, 0), drawSize),
            &strformat, &textbrush
        );

        DrawCircle(
            graphicsMem, pen,
            PointF(drawSize.Width / 2, drawSize.Height / 2),
            drawSize.Height / 2 - sizeUnit * 0.3f,
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
            szDataBuffer, -1, &textFont,
            RectF(PointF(sizeUnit * 3, 0), drawSize),
            &strformat, &textbrush
        );
        DrawCircle(
            graphicsMem, pen,
            PointF(drawSize.Width / 2 + sizeUnit * 3, drawSize.Height / 2),
            drawSize.Height / 2 - sizeUnit * 0.3f,
            REAL(perfData.memPercent / 100)
        );
    }

    // 绘制网速
    if (this->config.byShowContent & SHOWCONTENT_NETSPEED)
    {
        drawSize.Width = sizeUnit * 3;
        drawSize.Height = sizeUnit * 1;
        graphicsMem.TranslateTransform(0, 0);

        if (this->config.byShowContent & SHOWCONTENT_CPUMEM)
        {
            graphicsMem.TranslateTransform(0, sizeUnit * 3);
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
            szDataBuffer, -1, &textFont,
            RectF(PointF(0, sizeUnit), drawSize),
            &strformat, &textbrush
        );
        DrawSpeedStair(
            graphicsMem, statusColor,
            RectF(sizeUnit * 0.4f, sizeUnit * 0.08f, drawSize.Width - sizeUnit * 0.8f, drawSize.Height - sizeUnit * 0.16f),
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
            szDataBuffer, -1, &textFont,
            RectF(PointF(sizeUnit * 3, sizeUnit), drawSize),
            &strformat, &textbrush
        );
        DrawSpeedStair(
            graphicsMem, statusColor,
            RectF(sizeUnit * 3.4f, sizeUnit * 0.08f, drawSize.Width - sizeUnit * 0.8f, drawSize.Height - sizeUnit * 0.16f),
            FALSE, nLevel
        );
    }

    // 拷贝缓存图, 结束绘图
    Graphics graphics(hdc);
    CachedBitmap* pCachedBmp = new CachedBitmap(pBmpMem, &graphics);
    graphics.DrawCachedBitmap(pCachedBmp, 0, 0);
    EndPaint(this->hWnd, &ps);

    delete pCachedBmp;
    delete pBmpMem;
    return 0;
}

LRESULT MainWindow::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
    // 自动调节图标颜色
    HICON hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK));
    this->pNotifyIcon->SetIcon(hIcon);
    return 0;
}

LRESULT MainWindow::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    // 加载ContextMenu
    HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
    HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

    // 解决在菜单外单击左键菜单不消失的问题
    SetForegroundWindow(this->hWnd);

    // 显示菜单
    TrackPopupMenuEx(hContextMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), this->hWnd, NULL);

    DestroyMenu(hContextMenuBar);
    return 0;
}

LRESULT MainWindow::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
    SIZE newResolution = { 0 };
    newResolution.cx = LOWORD(lParam);
    newResolution.cy = HIWORD(lParam);

    POINT newPos = { 0 };
    ConvertPointForResolution(&this->lastFloatPos, &this->lastResolution, &newResolution, &newPos);

    // 更新并保存现在的位置和分辨率
    this->UpdateFloatPosDataToReg(&newPos, &newResolution);

    // 如果当前浮动显示则修正位置
    if (this->config.bFloatWnd)
    {
        SetWindowPos(this->hWnd, HWND_TOPMOST, this->lastFloatPos.x, this->lastFloatPos.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
    }

    return 0;
}

LRESULT MainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) Menu: FALSE, Accelerator: TRUE
    // LOWORD(wParam) identifier
    // lParam: 0
    PCFGDATA pcfgdata = new CFGDATA;
    this->config.Get(pcfgdata);

    switch (LOWORD(wParam))
    {
        // 浮动窗口
    case IDM_FLOATWND:
        pcfgdata->bFloatWnd = (BOOL)!pcfgdata->bFloatWnd;
        break;
        // 显示子菜单
    case IDM_SHOWCPUMEM:
        if (pcfgdata->byShowContent == SHOWCONTENT_CPUMEM)
        {
            WCHAR szTitle[MAX_LOADSTRING];
            WCHAR szMsg[MAX_LOADSTRING];
            LoadStringW(GetModuleHandleW(NULL), IDS_SHOWWARNINGTITLE, szTitle, MAX_LOADSTRING);
            LoadStringW(GetModuleHandleW(NULL), IDS_SHOWWARNINGMSG, szMsg, MAX_LOADSTRING);
            MessageBoxW(NULL, szMsg, szTitle, MB_OK);
        }
        else
        {
            pcfgdata->byShowContent ^= SHOWCONTENT_CPUMEM;
        }
        break;
    case IDM_SHOWNETSPEED:
        if (pcfgdata->byShowContent == SHOWCONTENT_NETSPEED)
        {
            WCHAR szTitle[MAX_LOADSTRING];
            WCHAR szMsg[MAX_LOADSTRING];
            LoadStringW(GetModuleHandleW(NULL), IDS_SHOWWARNINGTITLE, szTitle, MAX_LOADSTRING);
            LoadStringW(GetModuleHandleW(NULL), IDS_SHOWWARNINGMSG, szMsg, MAX_LOADSTRING);
            MessageBoxW(NULL, szMsg, szTitle, MB_OK);
        }
        else
        {
            pcfgdata->byShowContent ^= SHOWCONTENT_NETSPEED;
        }
        break;
    case IDM_DARKTHEME:
        pcfgdata->bDarkTheme = (BOOL)!pcfgdata->bDarkTheme;
        break;
    case IDM_TRANS_100:
        pcfgdata->transparencyPercent = 100.0;
        break;
    case IDM_TRANS_75:
        pcfgdata->transparencyPercent = 75.0;
        break;
    case IDM_TRANS_50:
        pcfgdata->transparencyPercent = 50.0;
        break;
    case IDM_TRANS_25:
        pcfgdata->transparencyPercent = 25.0;
        break;
    case IDM_TRANSPARENCY:
        if (IsWindowEnabled(this->hWnd))
        {
            PTRANSDLGFORM pInitForm = new TRANSDLGFORM;
            // 填充表单初始值
            pInitForm->transparencyPercent = pcfgdata->transparencyPercent;

            // 显示对话框
            DialogBoxTrans(GetModuleHandleW(NULL), this->hWnd, pInitForm);

            // 获取返回表单值, 修改设置项
            pcfgdata->transparencyPercent = pInitForm->transparencyPercent;
            delete pInitForm;
        }
        break;
    case IDM_AUTORUN:
        pcfgdata->bAutoRun = (BOOL)!pcfgdata->bAutoRun;
        break;
    case IDM_TIMEALARM:
        pcfgdata->bTimeAlarm = (BOOL)!pcfgdata->bTimeAlarm;
        break;
    case IDM_INFOSOUND:
        pcfgdata->bInfoSound = (BOOL)!pcfgdata->bInfoSound;
        break;
    case IDM_EXIT:
        DestroyWindow(this->hWnd);
        break;
    default:
        delete pcfgdata;
        return DefWindowProcW(this->hWnd, WM_COMMAND, wParam, lParam);
    }

    // 应用更改
    this->ApplyConfig(pcfgdata);
    delete pcfgdata;
    return 0;
}

LRESULT MainWindow::OnTimer(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case IDT_REFRESHRECT:
        InvalidateRect(this->hWnd, NULL, TRUE);
        break;
    case IDT_TIMEALARM:
        PostMessageW(this->hWnd, WM_TIMEALARM, 0, 0);
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
    SetMenuItemState(hMenu, IDM_FLOATWND, FALSE, this->config.bFloatWnd ? MFS_CHECKED : MFS_UNCHECKED);

    // 子菜单
    SetMenuItemState(hMenu, IDM_AUTORUN, FALSE, this->config.bAutoRun ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_TIMEALARM, FALSE, this->config.bTimeAlarm ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_INFOSOUND, FALSE, this->config.bInfoSound ? MFS_CHECKED : MFS_UNCHECKED);

    // 子菜单
    SetMenuItemState(hMenu, IDM_SHOWCPUMEM, FALSE, (this->config.byShowContent & SHOWCONTENT_CPUMEM) ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_SHOWNETSPEED, FALSE, (this->config.byShowContent & SHOWCONTENT_NETSPEED) ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_DARKTHEME, FALSE, this->config.bDarkTheme ? MFS_CHECKED : MFS_UNCHECKED);
    
    switch ((UINT)this->config.transparencyPercent)
    {
    case 100:
        SetMenuItemState(hMenu, IDM_TRANS_100, FALSE, MFS_CHECKED);
        break;
    case 75:
        SetMenuItemState(hMenu, IDM_TRANS_75, FALSE, MFS_CHECKED);
        break;
    case 50:
        SetMenuItemState(hMenu, IDM_TRANS_50, FALSE, MFS_CHECKED);
        break;
    case 25:
        SetMenuItemState(hMenu, IDM_TRANS_25, FALSE, MFS_CHECKED);
        break;
    default:
        SetMenuItemState(hMenu, IDM_TRANSPARENCY, FALSE, MFS_CHECKED);
        break;
    }

    return 0;
}

LRESULT MainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (this->config.bFloatWnd)
    {
        if (wParam & MK_LBUTTON)
        {
            POINT ptCursor = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rcWnd;
            GetWindowRect(this->hWnd, &rcWnd);
            SetWindowPos(
                this->hWnd, HWND_TOPMOST,
                rcWnd.left + (ptCursor.x - this->ptDragSrc.x),
                rcWnd.top + (ptCursor.y - this->ptDragSrc.y),
                0, 0,
                SWP_SHOWWINDOW | SWP_NOSIZE
            );
        }
    }
    return 0;
}

LRESULT MainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    // 保存点击位置
    this->ptDragSrc.x = GET_X_LPARAM(lParam);
    this->ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
}

LRESULT MainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    // 保存一次现在的窗口位置和对应的分辨率
    if (this->config.bFloatWnd)
    {
        // 获得当前坐标和分辨率
        RECT point = { 0 };
        SIZE resolution = { 0 };
        GetWindowRect(this->hWnd, &point);
        GetScreenResolution(&resolution);
        this->UpdateFloatPosDataToReg((PPOINT)&point, &resolution);
    }
    return 0;
}

LRESULT MainWindow::OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
    // 清单文件里需要将程序的 DPI Awareness 设置为 Per Monitor High DPI Aware 才能接收此消息
    // 重新计算窗体大小
    this->wndSizeUnit = BASE_WNDSIZE_PIXELS * GetDpiForWindow(this->hWnd) / 96;
    SIZE sizeWnd = { 0 };
    this->GetWndSizeByShowContent(&sizeWnd, this->config.byShowContent);

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
    {
        // DEBUG here
        //MessageBoxW(this->hWnd, L"Double Click on NotifyIcon!\n", L"Double Click on NotifyIcon!\n", MB_OK);
        //TimeAlarm(this);
        OutputDebugStringW(L"Double Click on NotifyIcon!\n");
        break;
    }
    case WM_CONTEXTMENU:
    {
        // 加载ContextMenu
        HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
        HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

        // 解决在菜单外单击左键菜单不消失的问题
        SetForegroundWindow(this->hWnd);

        // 显示菜单
        TrackPopupMenuEx(hContextMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam), this->hWnd, NULL);

        DestroyMenu(hContextMenuBar);
        break;
    }
    case NIN_SELECT:
        //{
        //    // DEBUG
        //    WCHAR debugstr[128] = { 0 };
        //    int x = GET_X_LPARAM(wParam);
        //    int y = GET_Y_LPARAM(wParam);
        //    int dpi = GetDpiForWindow(this->hWnd);
        //    int dpi2 = GetDpiForSystem();
        //    swprintf_s(debugstr, 128, L"X:%d Y:%d DPI:%d SDPI:%d\n", x, y, dpi, dpi2);
        //    OutputDebugStringW(debugstr);
        //    RECT rcScreen = { 0 };
        //    GetWindowRect(GetDesktopWindow(), &rcScreen);
        //    swprintf_s(debugstr, 128, L"X:%d Y:%d\n", rcScreen.right, rcScreen.bottom);
        //    OutputDebugStringW(debugstr);
        //    break;
        //}
    case NIN_KEYSELECT:
    {
        if (!this->config.bFloatWnd)
        {
            // 不是浮动的情况下固定住窗口显示
            this->bWndFixed = TRUE;
        }

        // 只有点击了图标才需要设置前景窗口
        SetForegroundWindow(this->hWnd);
    }
    case NIN_POPUPOPEN:
    {
        // 不是浮动的情况下显示显示弹窗
        if (!this->config.bFloatWnd)
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
            POINT ptWnd = { 0 };
            LONG delta_cx = (sizeWnd.cx - sizeNotifyIcon.cx) / 2;
            LONG delta_cy = (sizeWnd.cy - sizeNotifyIcon.cy) / 2;
            switch (uDirection)
            {
            case ABE_LEFT:
                ptWnd.x = rcNotifyIcon.right;
                ptWnd.y = rcNotifyIcon.top - delta_cy;
                break;
            case ABE_TOP:
                ptWnd.x = rcNotifyIcon.left - delta_cx;
                ptWnd.y = rcNotifyIcon.bottom;
                break;
            case ABE_RIGHT:
                ptWnd.x = rcNotifyIcon.left - sizeWnd.cx;
                ptWnd.y = rcNotifyIcon.top - delta_cy;
                break;
            default:
                ptWnd.x = rcNotifyIcon.left - delta_cx;
                ptWnd.y = rcNotifyIcon.top - sizeWnd.cy;
                break;
            }
            SetWindowPos(this->hWnd, HWND_TOPMOST, ptWnd.x, ptWnd.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
            ShowWindow(this->hWnd, SW_SHOWNA);

            // 需要立即重绘窗口
            InvalidateRect(this->hWnd, NULL, TRUE);
        }
        break;
    }
    case NIN_POPUPCLOSE:
    {
        // 不是浮动且没有固定则隐藏弹窗
        if (!this->config.bFloatWnd && !this->bWndFixed)
        {
            ShowWindow(this->hWnd, SW_HIDE);
            break;
        }
    }
    default:
        return DefWindowProcW(this->hWnd, WM_NOTIFYICON, wParam, lParam);
    }
    return 0;
}

LRESULT MainWindow::OnTimeAlarm(WPARAM wParam, LPARAM lParam)
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
    return 0;
}

LRESULT MainWindow::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
    delete this->pNotifyIcon;
    // 添加图标
    this->pNotifyIcon = new NotifyIcon(
        this->hWnd, ID_NIDMAIN, WM_NOTIFYICON,
        LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK))
    );

    // 设置图标提示信息
    WCHAR szTip[MAX_NIDTIP] = { 0 };
    LoadStringW(GetModuleHandleW(NULL), IDS_APPNAME, szTip, MAX_NIDTIP);
    this->pNotifyIcon->SetTip(szTip);
    return 0;
}

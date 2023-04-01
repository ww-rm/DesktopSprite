#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/config.h>
#include <ds/notifyicon.h>
#include <ds/perfmonitor.h>
#include <ds/transdlg.h>
#include <ds/winapp.h>

#include <ds/mainwnd.h>

// ״̬��ɫ
Gdiplus::Color const STATUSCOLOR_LOW = 0xff00ff00;
Gdiplus::Color const STATUSCOLOR_MIDDLE = 0xffff8000;
Gdiplus::Color const STATUSCOLOR_HIGH = 0xffff0000;

// ע����ֵ
PCWSTR const REGVAL_LASTFLOATPOS = L"LastFloatPos";
PCWSTR const REGVAL_LASTRUNTIMERESOLUTION = L"LastRunTimeResolution";

using namespace Gdiplus;

// ��������
static UINT     const   REFRESHINTERVAL = 1000;                     // ��Ļ��ʾˢ�¼��
static UINT     const   ID_NIDMAIN = 1;                        // ͼ�� ID
static UINT     const   BASE_WNDSIZE_PIXELS = 20;                       // �����ڵĻ�����Ԫ�����ش�С

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

    // ���Ƽ�ͷ
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

    // ������ͼ
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

    // ע���������ؽ���Ϣ
    this->uMsgTaskbarCreated = RegisterWindowMessageW(SZMSG_TASKBARCREATED);

    // ע�ᴰ��
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

    // ��������
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
    // Ĭ��������λ������Ļ�� 1/6 ��
    GetScreenResolution(&this->lastResolution);
    this->lastFloatPos.x = this->lastResolution.cx * 5 / 6;
    this->lastFloatPos.y = this->lastResolution.cy * 1 / 6;

    DWORD dwErrorCode = ERROR_SUCCESS;

    // ��ע�����
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

    // �ر�ע���
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

    // ��ע�����
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

    // �ر�ע���
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    return dwErrorCode;

}

DWORD MainWindow::ApplyConfig()
{
    // �Ƿ���ʾ��������
    if (this->config.bFloatWnd)
    {
        SetWindowPos(this->hWnd, HWND_TOPMOST, this->lastFloatPos.x, this->lastFloatPos.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
        ShowWindow(this->hWnd, SW_SHOWNA);
    }
    else
    {
        ShowWindow(this->hWnd, SW_HIDE);
    }

    // ����͸����
    SetLayeredWindowAttributes(this->hWnd, 0, PercentToAlpha(this->config.transparencyPercent), LWA_ALPHA);

    // ��������
    if (this->config.bAutoRun)
    {
        SetAppAutoRun(this->app->GetAppName());
    }
    else
    {
        UnsetAppAutoRun(this->app->GetAppName());
    }

    // �Ƿ����㱨ʱ
    if (this->config.bTimeAlarm)
    {
        SetTimer(this->hWnd, IDT_TIMEALARM, 100, (TIMERPROC)NULL);
    }
    else
    {
        KillTimer(this->hWnd, IDT_TIMEALARM);
    }

    // �ػ�һ��
    InvalidateRect(this->hWnd, NULL, TRUE);

    this->config.SaveToReg(this->app->GetAppName());

    return 0;
}

DWORD MainWindow::ApplyConfig(PCFGDATA pcfgdata)
{
    // ���ø�������
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

    // ������ʾ����
    if (pcfgdata->byShowContent != this->config.byShowContent)
    {
        SIZE sizeWnd = { 0 };
        GetWndSizeByShowContent(&sizeWnd, pcfgdata->byShowContent);
        SetWindowPos(this->hWnd, HWND_TOPMOST, 0, 0, sizeWnd.cx, sizeWnd.cy, SWP_NOACTIVATE | SWP_NOMOVE);
    }

    // ����͸����
    if (pcfgdata->transparencyPercent != this->config.transparencyPercent)
    {
        SetLayeredWindowAttributes(this->hWnd, 0, PercentToAlpha(pcfgdata->transparencyPercent), LWA_ALPHA);
    }

    // ���ÿ�������
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

    // ���㱨ʱ
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

    // �ػ�һ��
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
        //case WM_POWERBROADCAST: // XXX: �ղ��������Ϣ
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
    // ��ʼ�����ò������
    this->config.LoadFromReg(this->app->GetAppName());
    //if (PathFileExistsW(this->GetConfigPath()))
    //{
    //    this->config.LoadFromFile(this->GetConfigPath());
    //}

    // DPI ���
    this->wndSizeUnit = BASE_WNDSIZE_PIXELS * GetDpiForWindow(this->hWnd) / 96;

    // ���ô���λ�����С, ��ȡ��ǰ��Ļ�ֱ���, ��������һ������ʱ����Ĵ������굽�·ֱ���
    this->LoadFloatPosDataFromReg();

    SIZE sizeWnd = { 0 };
    POINT point = { 0 };
    SIZE resolution = { 0 };
    this->GetWndSizeByShowContent(&sizeWnd, this->config.byShowContent);
    GetScreenResolution(&resolution);
    ConvertPointForResolution(&this->lastFloatPos, &this->lastResolution, &resolution, &point);
    SetWindowPos(this->hWnd, HWND_TOPMOST, point.x, point.y, sizeWnd.cx, sizeWnd.cy, SWP_NOACTIVATE);

    // ��¼�µ�λ�úͷֱ���
    this->UpdateFloatPosDataToReg(&point, &resolution);

    // �������������˽������
    DWORD cbData = 0;
    PBYTE pFontData = GetResPointer(IDR_TTF1, L"TTF", &cbData);
    this->fontColl.AddMemoryFont((PVOID)pFontData, (INT)cbData);

    // ���ͼ��
    this->pNotifyIcon = new NotifyIcon(
        this->hWnd, ID_NIDMAIN, WM_NOTIFYICON,
        LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK))
    );

    // ����ͼ����ʾ��Ϣ
    WCHAR szTip[MAX_NIDTIP] = { 0 };
    LoadStringW(GetModuleHandleW(NULL), IDS_APPNAME, szTip, MAX_NIDTIP);
    this->pNotifyIcon->SetTip(szTip);

    // Ӧ��������
    this->ApplyConfig();

    // ÿ 1s ˢ��һ����ʾ
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
    // ���Ǹ�����ʧȥ����
    if (!this->config.bFloatWnd && !wParam)
    {
        this->bWndFixed = FALSE;
        ShowWindow(this->hWnd, SW_HIDE);
    }
    return 0;
}

LRESULT MainWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
    // ������ͨ�˳�, ֻ��ͨ���˵��˳�
    return 0;
}

LRESULT MainWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
    // �õ���������
    PERFDATA perfData = { 0 };
    this->perfMonitor.GetPerfData(&perfData);
    WCHAR szDataBuffer[16] = { 0 };     // �ַ���������
    INT nLevel = 0;
    Color statusColor;

    // ��ʼ��ͼ
    PAINTSTRUCT ps = { 0 };
    HDC hdc = BeginPaint(this->hWnd, &ps);

    REAL sizeUnit = (REAL)this->wndSizeUnit;

    // �õ���ͼ�����С
    RECT rcClient;
    GetClientRect(this->hWnd, &rcClient);
    SizeF sizeClient(REAL(rcClient.right - rcClient.left), REAL(rcClient.bottom - rcClient.top));

    // ʹ�û�������ͼ
    Bitmap* pBmpMem = new Bitmap((INT)sizeClient.Width, (INT)sizeClient.Height);
    Graphics graphicsMem(pBmpMem);

    // ���û�ͼģʽ
    graphicsMem.SetSmoothingMode(SmoothingModeAntiAlias);                       // ͼ����Ⱦ�����
    graphicsMem.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);        // ������Ⱦ�����

    // ��ͼ���Զ���
    SizeF drawSize(sizeUnit * 3, sizeUnit * 3);                 // ���ƾ���
    Pen pen(Color::Green, sizeUnit / 6);                                                   // ͼ����ɫ
    SolidBrush textbrush(this->config.bDarkTheme ? Color::White : Color::Black);   // �ı���ɫ
    SolidBrush bgbrush(this->config.bDarkTheme ? Color::Black : Color::White);     // ������ɫ

    // �������д���Ҫʹ�õ�����
    FontFamily fontFamily;
    INT found = 0;
    this->fontColl.GetFamilies(1, &fontFamily, &found);
    Font textFont(&fontFamily, sizeUnit * 2 / 3, FontStyleRegular, UnitPixel);             // �����С������ֵ����


    StringFormat strformat(StringFormatFlagsNoClip);                            // ���־��ھ�������
    strformat.SetAlignment(StringAlignmentCenter);
    strformat.SetLineAlignment(StringAlignmentCenter);

    // ˢ�ڱ���
    graphicsMem.FillRectangle(&bgbrush, 0., 0., sizeClient.Width, sizeClient.Height);

    // ����CPU��MEM
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

        // �����ڴ�
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

    // ��������
    if (this->config.byShowContent & SHOWCONTENT_NETSPEED)
    {
        drawSize.Width = sizeUnit * 3;
        drawSize.Height = sizeUnit * 1;
        graphicsMem.TranslateTransform(0, 0);

        if (this->config.byShowContent & SHOWCONTENT_CPUMEM)
        {
            graphicsMem.TranslateTransform(0, sizeUnit * 3);
        }

        // �����ϴ�
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

        // ��������
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

    // ��������ͼ, ������ͼ
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
    // �Զ�����ͼ����ɫ
    HICON hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK));
    this->pNotifyIcon->SetIcon(hIcon);
    return 0;
}

LRESULT MainWindow::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    // ����ContextMenu
    HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
    HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

    // ����ڲ˵��ⵥ������˵�����ʧ������
    SetForegroundWindow(this->hWnd);

    // ��ʾ�˵�
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

    // ���²��������ڵ�λ�úͷֱ���
    this->UpdateFloatPosDataToReg(&newPos, &newResolution);

    // �����ǰ������ʾ������λ��
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
        // ��������
    case IDM_FLOATWND:
        pcfgdata->bFloatWnd = (BOOL)!pcfgdata->bFloatWnd;
        break;
        // ��ʾ�Ӳ˵�
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
            // ������ʼֵ
            pInitForm->transparencyPercent = pcfgdata->transparencyPercent;

            // ��ʾ�Ի���
            DialogBoxTrans(GetModuleHandleW(NULL), this->hWnd, pInitForm);

            // ��ȡ���ر�ֵ, �޸�������
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

    // Ӧ�ø���
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
    // ��ȡ�˵����
    HMENU hMenu = (HMENU)wParam;

    // ���ò˵�״̬
    SetMenuItemState(hMenu, IDM_FLOATWND, FALSE, this->config.bFloatWnd ? MFS_CHECKED : MFS_UNCHECKED);

    // �Ӳ˵�
    SetMenuItemState(hMenu, IDM_AUTORUN, FALSE, this->config.bAutoRun ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_TIMEALARM, FALSE, this->config.bTimeAlarm ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_INFOSOUND, FALSE, this->config.bInfoSound ? MFS_CHECKED : MFS_UNCHECKED);

    // �Ӳ˵�
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
    // ������λ��
    this->ptDragSrc.x = GET_X_LPARAM(lParam);
    this->ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
}

LRESULT MainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    // ����һ�����ڵĴ���λ�úͶ�Ӧ�ķֱ���
    if (this->config.bFloatWnd)
    {
        // ��õ�ǰ����ͷֱ���
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
    // �嵥�ļ�����Ҫ������� DPI Awareness ����Ϊ Per Monitor High DPI Aware ���ܽ��մ���Ϣ
    // ���¼��㴰���С
    this->wndSizeUnit = BASE_WNDSIZE_PIXELS * GetDpiForWindow(this->hWnd) / 96;
    SIZE sizeWnd = { 0 };
    this->GetWndSizeByShowContent(&sizeWnd, this->config.byShowContent);

    // �����µĴ����С
    SetWindowPos(this->hWnd, HWND_TOPMOST, 0, 0, sizeWnd.cx, sizeWnd.cy, SWP_NOACTIVATE | SWP_NOMOVE);

    // �ػ洰������
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
        // ����ContextMenu
        HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
        HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);

        // ����ڲ˵��ⵥ������˵�����ʧ������
        SetForegroundWindow(this->hWnd);

        // ��ʾ�˵�
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
            // ���Ǹ���������¹̶�ס������ʾ
            this->bWndFixed = TRUE;
        }

        // ֻ�е����ͼ�����Ҫ����ǰ������
        SetForegroundWindow(this->hWnd);
    }
    case NIN_POPUPOPEN:
    {
        // ���Ǹ������������ʾ��ʾ����
        if (!this->config.bFloatWnd)
        {
            UINT uDirection = GetShellTrayDirection();

            // ֪ͨ����ͼ��λ�ô�С
            RECT rcNotifyIcon = { 0 };
            this->pNotifyIcon->GetRect(&rcNotifyIcon);
            SIZE sizeNotifyIcon = { rcNotifyIcon.right - rcNotifyIcon.left, rcNotifyIcon.bottom - rcNotifyIcon.top };

            // ����λ�ô�С
            RECT rcWnd = { 0 };
            GetWindowRect(this->hWnd, &rcWnd);
            SIZE sizeWnd = { rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top };

            // ����������λ�ü��㴰�ڵ�λ��
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

            // ��Ҫ�����ػ洰��
            InvalidateRect(this->hWnd, NULL, TRUE);
        }
        break;
    }
    case NIN_POPUPCLOSE:
    {
        // ���Ǹ�����û�й̶������ص���
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
    // �Ƿ�������
    if (IsOnTheHour())
    {
        // �Ƿ��ѱ���ʱ
        if (!this->bClocked)
        {
            this->bClocked = TRUE;
            this->TimeAlarm();
        }
    }
    else
    {
        // ��������ձ�ʱ��¼
        this->bClocked = FALSE;
    }
    return 0;
}

LRESULT MainWindow::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
    delete this->pNotifyIcon;
    // ���ͼ��
    this->pNotifyIcon = new NotifyIcon(
        this->hWnd, ID_NIDMAIN, WM_NOTIFYICON,
        LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK))
    );

    // ����ͼ����ʾ��Ϣ
    WCHAR szTip[MAX_NIDTIP] = { 0 };
    LoadStringW(GetModuleHandleW(NULL), IDS_APPNAME, szTip, MAX_NIDTIP);
    this->pNotifyIcon->SetTip(szTip);
    return 0;
}

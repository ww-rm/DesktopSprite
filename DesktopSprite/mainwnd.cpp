#include "framework.h"
#include "util.h"
#include "config.h"
#include "notifyicon.h"
#include "perfdata.h"
#include "transdlg.h"

#include "mainwnd.h"

using namespace Gdiplus;

// ��������
static UINT     const   REFRESHINTERVAL                 = 1000;                     // ��Ļ��ʾˢ�¼��
static UINT     const   ID_NIDMAIN                      = 1;                        // ͼ�� ID
static UINT     const   BASE_WNDSIZE_PIXELS             = 20;                       // �����ڵĻ�����Ԫ�����ش�С

static UINT             uMsgTaskbarCreated              = 0;                        // �������ؽ���Ϣ

// ���̺�������
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT OnCreate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnDestroy(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnActivate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnClose(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnPaint(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnSettingChange(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnContextMenu(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnDisplayChange(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnCommand(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnTimer(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnInitMenuPopup(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnMouseMove(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnLButtonDown(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnLButtonUp(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnDpiChanged(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnNotifyIcon(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnTimeAlarm(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnTaskBarCreated(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam);


// ���̴���ʵ��
static LRESULT OnCreate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // ��ʼ����������ʱ��Ҫ������
    pWndData->bWndFixed = FALSE;
    pWndData->wndSizeUnit = BASE_WNDSIZE_PIXELS * GetDpiForWindow(pWndData->hWnd) / 96; // DPI ��֪

    // ���ͼ��
    AddNotifyIcon(
        pWndData->hWnd, ID_NIDMAIN, WM_NOTIFYICON,
        LoadIconW(
            GetModuleHandleW(NULL),
            MAKEINTRESOURCEW(IsSystemDarkTheme() ? IDI_APPICON_LIGHT : IDI_APPICON_DARK)
        )
    );

    // ����ͼ����ʾ��Ϣ
    WCHAR szTip[MAX_NIDTIP] = { 0 };
    LoadStringW(GetModuleHandleW(NULL), IDS_APPNAME, szTip, MAX_NIDTIP);
    SetNotifyIconTip(pWndData->hWnd, ID_NIDMAIN, szTip);

    // �������������˽������
    DWORD cbData = 0;
    PBYTE pFontData = GetResPointer(IDR_TTF1, L"TTF", &cbData);
    pWndData->pFontColl = new PrivateFontCollection;
    pWndData->pFontColl->AddMemoryFont((PVOID)pFontData, (INT)cbData);
    
    // ÿ 1s ˢ��һ����ʾ
    SetTimer(pWndData->hWnd, IDT_REFRESHRECT, REFRESHINTERVAL, (TIMERPROC)NULL);

    // Ӧ��������
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    if (pCfgData != NULL)
    {
        LoadConfigFromReg(pCfgData);

        // ���ն�ȡ���������ʼ����������
        pWndData->bFloatWnd = pCfgData->bFloatWnd;

        // ����
        pWndData->bInfoSound = pCfgData->bInfoSound;

        // ����
        pWndData->bDarkTheme = pCfgData->bDarkTheme;

        // ����͸����
        SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);

        // ������ʾ���ݾ������ڴ�С
        pWndData->byShowContent = pCfgData->byShowContent;
        SIZE sizeWnd = { 0 };
        GetWndSizeByShowContent(pWndData, &sizeWnd, pCfgData->byShowContent);

        // ��ȡ��ǰ��Ļ�ֱ���
        // ��������һ������ʱ����Ĵ������굽�·ֱ���
        GetScreenResolution(&pWndData->runtimeResolution);

        POINT newPos = { 0 };
        ConvertPointForResolution(
            &pCfgData->ptLastFloatPos,
            &pCfgData->sizeLastRuntimeResolution, &pWndData->runtimeResolution,
            &newPos
        );

        // ����λ�ô�С
        SetWindowPos(
            pWndData->hWnd, HWND_TOPMOST,
            newPos.x, newPos.y,
            sizeWnd.cx, sizeWnd.cy,
            SWP_NOACTIVATE
        );

        // �Ƿ���ʾ��������
        if (pCfgData->bFloatWnd)
        {
            ShowWindow(pWndData->hWnd, SW_SHOWNA);
            InvalidateRect(pWndData->hWnd, NULL, TRUE);
        }

        // �Ƿ����㱨ʱ
        if (pCfgData->bTimeAlarm)
        {
            SetTimer(pWndData->hWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
        }

        DefFreeMem(pCfgData);
    }
    else
    {
        // ������Ϣ�ڴ����ʧ��
        return -1;
    }
    return 0;
}

static LRESULT OnDestroy(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    delete pWndData->pFontColl;
    DeleteNotifyIcon(pWndData->hWnd, ID_NIDMAIN);
    PostQuitMessage(EXIT_SUCCESS);
    return 0;
}

static LRESULT OnActivate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // ���Ǹ�����ʧȥ����
    if (!pWndData->bFloatWnd && !wParam)
    {
        pWndData->bWndFixed = FALSE;
        ShowWindow(pWndData->hWnd, SW_HIDE);
    }
    return 0;
}

static LRESULT OnClose(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // ������ͨ�˳�, ֻ��ͨ���˵��˳�
    return 0;
}

static LRESULT OnPaint(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // �õ���������
    PERFDATA perfData = { 0 };
    GetPerfData(&perfData);
    WCHAR szDataBuffer[16] = { 0 };     // �ַ���������
    INT nLevel = 0;
    Color statusColor;

    // ��ʼ��ͼ
    PAINTSTRUCT ps = { 0 };
    HDC hdc = BeginPaint(pWndData->hWnd, &ps);

    REAL sizeUnit = (REAL)pWndData->wndSizeUnit;

    // �õ���ͼ�����С
    RECT rcClient;
    GetClientRect(pWndData->hWnd, &rcClient);
    SizeF sizeClient(REAL(rcClient.right - rcClient.left), REAL(rcClient.bottom - rcClient.top));

    // ʹ�û�������ͼ
    Bitmap* pBmpMem = new Bitmap((INT)sizeClient.Width, (INT)sizeClient.Height);
    Graphics graphicsMem(pBmpMem);

    // ���û�ͼģʽ
    graphicsMem.SetSmoothingMode(SmoothingModeAntiAlias);                       // ͼ����Ⱦ�����
    graphicsMem.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);        // ������Ⱦ�����

    // ��ͼ���Զ���
    SizeF drawSize(sizeUnit * 3, sizeUnit * 3);                 // ���ƾ���
    Pen pen(Color::Green, sizeUnit/6);                                                   // ͼ����ɫ
    SolidBrush textbrush(pWndData->bDarkTheme ? Color::White : Color::Black);   // �ı���ɫ
    SolidBrush bgbrush(pWndData->bDarkTheme ? Color::Black : Color::White);     // ������ɫ

    // �������д���Ҫʹ�õ�����
    FontFamily fontFamily;
    INT found = 0;
    pWndData->pFontColl->GetFamilies(1, &fontFamily, &found);
    Font textFont(&fontFamily, sizeUnit*2/3, FontStyleRegular, UnitPixel);             // �����С������ֵ����
    

    StringFormat strformat(StringFormatFlagsNoClip);                            // ���־��ھ�������
    strformat.SetAlignment(StringAlignmentCenter);
    strformat.SetLineAlignment(StringAlignmentCenter);

    // ˢ�ڱ���
    graphicsMem.FillRectangle(&bgbrush, 0., 0., sizeClient.Width, sizeClient.Height);

    // ����CPU��MEM
    if (pWndData->byShowContent & SHOWCONTENT_CPUMEM)
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
            drawSize.Height / 2 - sizeUnit*0.3f,
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
    if (pWndData->byShowContent & SHOWCONTENT_NETSPEED)
    {
        drawSize.Width = sizeUnit * 3;
        drawSize.Height = sizeUnit * 1;
        graphicsMem.TranslateTransform(0, 0);

        if (pWndData->byShowContent & SHOWCONTENT_CPUMEM)
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
            RectF(sizeUnit*0.4f, sizeUnit * 0.08f, drawSize.Width - sizeUnit * 0.8f, drawSize.Height - sizeUnit * 0.16f),
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
    EndPaint(pWndData->hWnd, &ps);

    delete pCachedBmp;
    delete pBmpMem;
    return 0;
}

static LRESULT OnSettingChange(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // �Զ�����ͼ����ɫ
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
    // ����ContextMenu
    HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
    HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);
    
    // ����ڲ˵��ⵥ������˵�����ʧ������
    SetForegroundWindow(pWndData->hWnd);

    // ��ʾ�˵�
    TrackPopupMenuEx(
        hContextMenu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON,
        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
        pWndData->hWnd, NULL
    );

    DestroyMenu(hContextMenuBar);
    return 0;
}

static LRESULT OnDisplayChange(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{

    SIZE newResolution = { 0 };
    newResolution.cx = LOWORD(lParam);
    newResolution.cy = HIWORD(lParam);

    // �����ʾ�˴�������Ҫ�������ڵ�ǰλ��
    if (pWndData->bFloatWnd)
    {

        RECT oldPos = { 0 };
        GetWindowRect(pWndData->hWnd, &oldPos);
        POINT newPos = { 0 };
        ConvertPointForResolution((PPOINT)&oldPos, &pWndData->runtimeResolution, &newResolution, &newPos);

        SetWindowPos(
            pWndData->hWnd, HWND_TOPMOST,
            newPos.x, newPos.y,
            0, 0,
            SWP_NOACTIVATE | SWP_NOSIZE
        );
    }

    // �������ڵķֱ���
    CopySize(&newResolution, &pWndData->runtimeResolution);
    return 0;
}

static LRESULT OnCommand(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) Menu: FALSE, Accelerator: TRUE
    // LOWORD(wParam) identifier
    // lParam: 0

    // ��ȡ����
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    if (pCfgData != NULL)
    {
        LoadConfigFromReg(pCfgData);

        switch (LOWORD(wParam))
        {
            // ��������
        case IDM_FLOATWND:
        {
            pCfgData->bFloatWnd = !pCfgData->bFloatWnd;
            pWndData->bFloatWnd = pCfgData->bFloatWnd;
            if (pCfgData->bFloatWnd)
            {
                // ������ʾλ��
                POINT newPos = { 0 };
                ConvertPointForResolution(
                    &pCfgData->ptLastFloatPos,
                    &pCfgData->sizeLastRuntimeResolution, &pWndData->runtimeResolution,
                    &newPos
                );

                SetWindowPos(
                    pWndData->hWnd, HWND_TOPMOST,
                    newPos.x, newPos.y,
                    0, 0,
                    SWP_NOACTIVATE | SWP_NOSIZE
                );

                // ��ʾ����
                ShowWindow(pWndData->hWnd, SW_SHOWNA);
                InvalidateRect(pWndData->hWnd, NULL, TRUE);
            }
            else
            {
                // �������ʾ�������ھͱ���һ�����ڵĴ���λ�úͶ�Ӧ�ķֱ���
                RECT rcWnd = { 0 };
                GetWindowRect(pWndData->hWnd, &rcWnd);
                CopyPoint((PPOINT)&rcWnd, &pCfgData->ptLastFloatPos);
                CopySize(&pWndData->runtimeResolution, &pCfgData->sizeLastRuntimeResolution);
                ShowWindow(pWndData->hWnd, SW_HIDE);
            }
            break;
        }
        // ��ʾ�Ӳ˵�
        case IDM_SHOWCPUMEM:
        {
            if (pCfgData->byShowContent == SHOWCONTENT_CPUMEM)
            {
                WCHAR szTitle[MAX_LOADSTRING];
                WCHAR szMsg[MAX_LOADSTRING];
                LoadStringW(GetModuleHandleW(NULL), IDS_SHOWWARNINGTITLE, szTitle, MAX_LOADSTRING);
                LoadStringW(GetModuleHandleW(NULL), IDS_SHOWWARNINGMSG, szMsg, MAX_LOADSTRING);
                MessageBoxW(NULL, szMsg, szTitle, MB_OK);
            }
            else
            {
                pCfgData->byShowContent ^= SHOWCONTENT_CPUMEM;
                pWndData->byShowContent = pCfgData->byShowContent;
                SIZE sizeWnd = { 0 };
                GetWndSizeByShowContent(pWndData, &sizeWnd, pCfgData->byShowContent);
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
                WCHAR szTitle[MAX_LOADSTRING];
                WCHAR szMsg[MAX_LOADSTRING];
                LoadStringW(GetModuleHandleW(NULL), IDS_SHOWWARNINGTITLE, szTitle, MAX_LOADSTRING);
                LoadStringW(GetModuleHandleW(NULL), IDS_SHOWWARNINGMSG, szMsg, MAX_LOADSTRING);
                MessageBoxW(NULL, szMsg, szTitle, MB_OK);
            }
            else
            {
                pCfgData->byShowContent ^= SHOWCONTENT_NETSPEED;
                pWndData->byShowContent = pCfgData->byShowContent;
                SIZE sizeWnd = { 0 };
                GetWndSizeByShowContent(pWndData, &sizeWnd, pCfgData->byShowContent);
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
        case IDM_TRANS_100:
        {
            pCfgData->byTransparency = PercentToAlpha(100);
            pWndData->byTransparency = pCfgData->byTransparency;
            SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
            break;
        }
        case IDM_TRANS_75:
        {
            pCfgData->byTransparency = PercentToAlpha(75);
            pWndData->byTransparency = pCfgData->byTransparency;
            SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
            break;
        }
        case IDM_TRANS_50:
        {
            pCfgData->byTransparency = PercentToAlpha(50);
            pWndData->byTransparency = pCfgData->byTransparency;
            SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
            break;
        }
        case IDM_TRANS_25:
        {
            pCfgData->byTransparency = PercentToAlpha(25);
            pWndData->byTransparency = pCfgData->byTransparency;
            SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
            break;
        }
        case IDM_TRANSPARENCY:
        {
            if (IsWindowEnabled(pWndData->hWnd))
            {
                PTRANSDLGFORM pInitForm = (PTRANSDLGFORM)DefAllocMem(sizeof(TRANSDLGFORM));
                if (pInitForm != NULL)
                {
                    // ������ʼֵ
                    pInitForm->byTransparency = pCfgData->byTransparency;

                    // ��ʾ�Ի���
                    DialogBoxTrans(GetModuleHandleW(NULL), pWndData->hWnd, pInitForm);

                    // ��ȡ���ر�ֵ, �޸�������
                    pCfgData->byTransparency = pInitForm->byTransparency;

                    // Ӧ������
                    SetLayeredWindowAttributes(pWndData->hWnd, 0, pCfgData->byTransparency, LWA_ALPHA);
                    DefFreeMem(pInitForm);
                }
            }
            break;
        }
        // �����Ӳ˵�
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
        // �˳�
        case IDM_EXIT:
        {
            if (pCfgData->bFloatWnd)
            {
                // �����ǰ��ʾ���������˳�ʱ�ͱ���һ�����ڵĴ���λ�úͷֱ���
                RECT rcWnd = { 0 };
                GetWindowRect(pWndData->hWnd, &rcWnd);
                CopyPoint((PPOINT)&rcWnd, &pCfgData->ptLastFloatPos);
                CopySize(&pWndData->runtimeResolution, &pCfgData->sizeLastRuntimeResolution);
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

        // �����޸�֮�������
        SaveConfigToReg(pCfgData);
        DefFreeMem(pCfgData);
    }
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

    // ��ȡ�˵����
    HMENU hMenu = (HMENU)wParam;

    // ���ò˵�״̬
    SetMenuItemState(hMenu, IDM_FLOATWND, FALSE, pCfgData->bFloatWnd ? MFS_CHECKED : MFS_UNCHECKED);

    // �Ӳ˵�
    SetMenuItemState(hMenu, IDM_AUTORUN, FALSE, pCfgData->bAutoRun ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_TIMEALARM, FALSE, pCfgData->bTimeAlarm ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_INFOSOUND, FALSE, pCfgData->bInfoSound ? MFS_CHECKED : MFS_UNCHECKED);

    // �Ӳ˵�
    SetMenuItemState(hMenu, IDM_SHOWCPUMEM, FALSE, (pCfgData->byShowContent & SHOWCONTENT_CPUMEM) ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_SHOWNETSPEED, FALSE, (pCfgData->byShowContent & SHOWCONTENT_NETSPEED) ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_DARKTHEME, FALSE, pCfgData->bDarkTheme ? MFS_CHECKED : MFS_UNCHECKED);
    switch (pCfgData->byTransparency)
    {
    case PercentToAlpha(100):
        SetMenuItemState(hMenu, IDM_TRANS_100, FALSE, MFS_CHECKED);
        break;
    case PercentToAlpha(75):
        SetMenuItemState(hMenu, IDM_TRANS_75, FALSE, MFS_CHECKED);
        break;
    case PercentToAlpha(50):
        SetMenuItemState(hMenu, IDM_TRANS_50, FALSE, MFS_CHECKED);
        break;
    case PercentToAlpha(25):
        SetMenuItemState(hMenu, IDM_TRANS_25, FALSE, MFS_CHECKED);
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
    // ������λ��
    pWndData->ptDragSrc.x = GET_X_LPARAM(lParam);
    pWndData->ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
}

static LRESULT OnLButtonUp(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // TODO
    return DefWindowProcW(pWndData->hWnd, WM_LBUTTONUP, wParam, lParam);
}

static LRESULT OnDpiChanged(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // �嵥�ļ�����Ҫ������� DPI Awareness ����Ϊ Per Monitor High DPI Aware ���ܽ��մ���Ϣ
    // ���¼��㴰���С
    pWndData->wndSizeUnit = BASE_WNDSIZE_PIXELS * GetDpiForWindow(pWndData->hWnd) / 96;
    SIZE sizeWnd = { 0 };
    GetWndSizeByShowContent(pWndData, &sizeWnd, pWndData->byShowContent);

    // �����µĴ����С
    SetWindowPos(
        pWndData->hWnd, HWND_TOPMOST,
        0, 0,
        sizeWnd.cx, sizeWnd.cy,
        SWP_NOACTIVATE | SWP_NOMOVE
    );

    // �ػ洰������
    InvalidateRect(pWndData->hWnd, NULL, TRUE);

    return 0;
}

static LRESULT OnNotifyIcon(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(lParam))
    {
    case WM_LBUTTONDBLCLK:
    {
        // TODO
        break;
    }
    case WM_CONTEXTMENU:
    {
        // ����ContextMenu
        HMENU hContextMenuBar = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_CONTEXTMENU));
        HMENU hContextMenu = GetSubMenu(hContextMenuBar, 0);
        
        // ����ڲ˵��ⵥ������˵�����ʧ������
        SetForegroundWindow(pWndData->hWnd);
        
        // ��ʾ�˵�
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
    //{
    //    // DEBUG
    //    WCHAR debugstr[128] = { 0 };
    //    int x = GET_X_LPARAM(wParam);
    //    int y = GET_Y_LPARAM(wParam);
    //    int dpi = GetDpiForWindow(pWndData->hWnd);
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
        if (!pWndData->bFloatWnd)
        {
            // ���Ǹ���������¹̶�ס������ʾ
            pWndData->bWndFixed = TRUE;
        }

        // ֻ�е����ͼ�����Ҫ����ǰ������
        SetForegroundWindow(pWndData->hWnd);
    }
    case NIN_POPUPOPEN:
    {
        // ���Ǹ������������ʾ��ʾ����
        if (!pWndData->bFloatWnd)
        {
            UINT uDirection = GetShellTrayDirection();

            // ֪ͨ����ͼ��λ�ô�С
            RECT rcNotifyIcon = { 0 };
            GetNotifyIconRect(pWndData->hWnd, ID_NIDMAIN, &rcNotifyIcon);
            SIZE sizeNotifyIcon = { rcNotifyIcon.right - rcNotifyIcon.left, rcNotifyIcon.bottom - rcNotifyIcon.top };

            // ����λ�ô�С
            RECT rcWnd = { 0 };
            GetWindowRect(pWndData->hWnd, &rcWnd);
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
            SetWindowPos(
                pWndData->hWnd, HWND_TOPMOST,
                ptWnd.x, ptWnd.y, 0, 0,
                SWP_NOACTIVATE | SWP_NOSIZE
            );
            ShowWindow(pWndData->hWnd, SW_SHOWNA);

            // ��Ҫ�����ػ洰��
            InvalidateRect(pWndData->hWnd, NULL, TRUE);
        }
        break;
    }
    case NIN_POPUPCLOSE:
    {
        // ���Ǹ�����û�й̶������ص���
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


// ��Ϣӳ��
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
    case WM_DISPLAYCHANGE:
        return OnDisplayChange(pWndData, wParam, lParam);
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
    case WM_DPICHANGED:
        return OnDpiChanged(pWndData, wParam, lParam);
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
    // ע���������ؽ���Ϣ
    uMsgTaskbarCreated = RegisterWindowMessageW(SZMSG_TASKBARCREATED);

    // ע�ᴰ��
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

DWORD GetWndSizeByShowContent(PMAINWNDDATA pWndData, PSIZE psizeWnd, BYTE byShowContent)
{
    psizeWnd->cx = pWndData->wndSizeUnit * 6;
    psizeWnd->cy = 0;
    if (byShowContent & SHOWCONTENT_CPUMEM)
    {
        psizeWnd->cy += pWndData->wndSizeUnit * 3;
    }
    if (byShowContent & SHOWCONTENT_NETSPEED)
    {
        psizeWnd->cy += pWndData->wndSizeUnit * 2;
    }
    return 0;
}

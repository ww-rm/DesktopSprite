#include "framework.h"
#include "util.h"
#include "config.h"
#include "notifyicon.h"
#include "perfdata.h"
#include "mainwnd.h"

// ��������
static UINT     const   REFRESHINTERVAL                 = 1000;                     // ��Ļ��ʾˢ�¼��
static UINT     const   ANIMATIONTIME                   = 100;                      // ����Ч������ʱ��
static UINT     const   ID_NIDMAIN                      = 1;                        // ͼ�� ID
static INT      const   MAINWNDSIZE_CX                  = 100;                      // ���� cx
static INT      const   MAINWNDSIZE_CY                  = 400;                      // ���� cy

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


// ���̴���ʵ��
static LRESULT OnCreate(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // Ӧ��������
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    LoadConfigFromReg(pCfgData);

    // ���ն�ȡ���������ʼ����������
    pWndData->bFloatWnd = pCfgData->bFloatWnd;

    // �Ƿ񸡶�����
    if (pCfgData->bFloatWnd)
    {
        // ������ʾλ��
        SetWindowPos(
            pWndData->hWnd, HWND_TOPMOST,
            pCfgData->ptLastFloatPos.x, pCfgData->ptLastFloatPos.y,
            MAINWNDSIZE_CX, MAINWNDSIZE_CY,
            SWP_NOACTIVATE
        );

        // ��ʾ����
        AnimateWindow(pWndData->hWnd, ANIMATIONTIME, AW_BLEND);
        InvalidateRect(pWndData->hWnd, NULL, TRUE);
    }

    // �Ƿ����㱨ʱ
    if (pCfgData->bTimeAlarm)
    {
        SetTimer(pWndData->hWnd, IDT_TIMEALARM, GetHourTimeDiff(), (TIMERPROC)NULL);
    }

    // ����, ��ɫ, ����
    pWndData->hTextFont = CreateFontIndirectW(&pCfgData->lfText);
    pWndData->rgbText = pCfgData->rgbTextColor;
    pWndData->bInfoSound = pCfgData->bInfoSound;
    DefFreeMem(pCfgData);

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

    // ���� 85% ͸����
    SetLayeredWindowAttributes(pWndData->hWnd, RGB(0, 0, 0), (255 * 85) / 100, LWA_ALPHA);

    // ÿ 1s ˢ��һ����ʾ
    SetTimer(pWndData->hWnd, IDT_REFRESHRECT, REFRESHINTERVAL, (TIMERPROC)NULL);
    return 0;
}

static LRESULT OnDestroy(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    if (pWndData->hTextFont != NULL)
    {
        DeleteObject(pWndData->hTextFont);
        pWndData->hTextFont = NULL;
    }
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
        AnimateWindow(pWndData->hWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
    }
    return 0;
}

static LRESULT OnClose(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    AnimateWindow(pWndData->hWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
    return 0;
}

static LRESULT OnPaint(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // �õ���ʽ�����ַ������� xx.xx% xx.xxKB/s
    PERFDATA perfData = { 0 };
    GetPerfData(&perfData);
    WCHAR szDataBuffer[4][16] = { 0 };  // CPU:MEM:UPLOAD:DOWNLOAD
    StringCchPrintfW(szDataBuffer[0], 16, L"%.2f%%", perfData.cpuPercent);
    StringCchPrintfW(szDataBuffer[1], 16, L"%.2f%%", perfData.memPercent);
    ConvertSpeed(perfData.uploadSpeed, szDataBuffer[2], 16);
    ConvertSpeed(perfData.downloadSpeed, szDataBuffer[3], 16);

    // ��ʼ��ͼ
    PAINTSTRUCT ps = { 0 };
    HDC hdc = BeginPaint(pWndData->hWnd, &ps);
    HFONT hFontPre = SelectFont(hdc, pWndData->hTextFont);
    SetTextColor(hdc, pWndData->rgbText);
    SetBkMode(hdc, TRANSPARENT);

    HINSTANCE hInstance = GetModuleHandleW(NULL);
    WCHAR szTip[MAX_LOADSTRING] = { 0 };
    RECT rcClient = { 0 };
    GetClientRect(pWndData->hWnd, &rcClient);
    INT nClientHeight = rcClient.bottom - rcClient.top;
    RECT rcDraw = { 0 };
    CopyRect(&rcDraw, &rcClient);
    SIZE_T nTextLength = 0;

    // ����CPU
    LoadStringW(hInstance, IDS_TIP_CPU, szTip, MAX_LOADSTRING);
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);
    rcDraw.top = 0;
    rcDraw.bottom = rcDraw.top + nClientHeight / 8;
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_CENTER);

    StringCchLengthW(szDataBuffer[0], 16, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szDataBuffer[0], (INT)nTextLength, &rcDraw, DT_CENTER);

    // �����ڴ�
    LoadStringW(hInstance, IDS_TIP_MEM, szTip, MAX_LOADSTRING);
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_CENTER);

    StringCchLengthW(szDataBuffer[1], 16, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szDataBuffer[1], (INT)nTextLength, &rcDraw, DT_CENTER);

    // �����ϴ�
    LoadStringW(hInstance, IDS_TIP_UPLOAD, szTip, MAX_LOADSTRING);
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_CENTER);

    StringCchLengthW(szDataBuffer[2], 16, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szDataBuffer[2], (INT)nTextLength, &rcDraw, DT_CENTER);

    // ��������
    LoadStringW(hInstance, IDS_TIP_DOWNLOAD, szTip, MAX_LOADSTRING);
    StringCchLengthW(szTip, MAX_LOADSTRING, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szTip, (INT)nTextLength, &rcDraw, DT_CENTER);

    StringCchLengthW(szDataBuffer[3], 16, &nTextLength);
    rcDraw.top += nClientHeight / 8;
    rcDraw.bottom += nClientHeight / 8;
    DrawTextW(hdc, szDataBuffer[3], (INT)nTextLength, &rcDraw, DT_CENTER);

    // ������ͼ
    SelectObject(hdc, hFontPre);
    EndPaint(pWndData->hWnd, &ps);
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

static LRESULT OnCommand(PMAINWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) Menu: FALSE, Accelerator: TRUE
    // LOWORD(wParam) identifier
    // lParam: 0

    // ��ȡ����
    PCFGDATA pCfgData = (PCFGDATA)DefAllocMem(sizeof(CFGDATA));
    LoadConfigFromReg(pCfgData);
    switch (LOWORD(wParam))
    {
    case IDM_FLOATWND:
    {
        pCfgData->bFloatWnd = !pCfgData->bFloatWnd;
        pWndData->bFloatWnd = pCfgData->bFloatWnd;
        if (pCfgData->bFloatWnd)
        {
            // ������ʾλ��
            SetWindowPos(
                pWndData->hWnd, HWND_TOPMOST,
                pCfgData->ptLastFloatPos.x, pCfgData->ptLastFloatPos.y,
                MAINWNDSIZE_CX, MAINWNDSIZE_CY,
                SWP_NOACTIVATE
            );

            // ��ʾ����
            AnimateWindow(pWndData->hWnd, ANIMATIONTIME, AW_BLEND);
            InvalidateRect(pWndData->hWnd, NULL, TRUE);
        }
        else
        {
            // �������ʾ�������ھͱ���һ�����ڵĴ���λ��
            RECT rcWnd = { 0 };
            GetWindowRect(pWndData->hWnd, &rcWnd);
            pCfgData->ptLastFloatPos.x = rcWnd.left;
            pCfgData->ptLastFloatPos.y = rcWnd.top;
            AnimateWindow(pWndData->hWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
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
    case IDM_EXIT:
    {
        if (pCfgData->bFloatWnd)
        {
            // �����ǰ��ʾ���������˳�ʱ�ͱ���һ�����ڵĴ���λ��
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

    // �����޸�֮�������
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

    HMENU hMenu = (HMENU)wParam;
    SetMenuItemState(hMenu, IDM_AUTORUN, pCfgData->bAutoRun ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_FLOATWND, pCfgData->bFloatWnd ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_TIMEALARM, pCfgData->bTimeAlarm ? MFS_CHECKED : MFS_UNCHECKED);
    SetMenuItemState(hMenu, IDM_INFOSOUND, pCfgData->bInfoSound ? MFS_CHECKED : MFS_UNCHECKED);

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
                MAINWNDSIZE_CX, MAINWNDSIZE_CY, 
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
            //SetLayeredWindowAttributes;
            //UpdateLayeredWindow;
            //UpdateLayeredWindowIndirect;
            RECT rcNotifyIcon = { 0 };
            GetNotifyIconRect(pWndData->hWnd, ID_NIDMAIN, &rcNotifyIcon);

            SetWindowPos(
                pWndData->hWnd, HWND_TOPMOST,
                rcNotifyIcon.left - ((MAINWNDSIZE_CX - (rcNotifyIcon.right - rcNotifyIcon.left)) / 2),
                rcNotifyIcon.top - MAINWNDSIZE_CY,
                MAINWNDSIZE_CX, MAINWNDSIZE_CY,
                SWP_NOACTIVATE
            );
            AnimateWindow(pWndData->hWnd, ANIMATIONTIME, AW_BLEND);

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
            AnimateWindow(pWndData->hWnd, ANIMATIONTIME, AW_HIDE | AW_BLEND);
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
//        // ������ʾλ��
//        SetWindowPos(
//            hMainWnd, HWND_TOPMOST,
//            pCfgData->ptLastFloatPos.x, pCfgData->ptLastFloatPos.y,
//            MAINWNDSIZE_CX, MAINWNDSIZE_CY,
//            SWP_NOACTIVATE
//        );
//
//        // ��ʾ����
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

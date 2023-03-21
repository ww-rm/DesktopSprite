#include <ds/spritewnd.h>
#include <ds/framework.h>

#include <ds/util.h>


using namespace Gdiplus;

// ��������
static UINT     const   REFRESHINTERVAL = 1000;                         // ��Ļ��ʾˢ�¼��
static UINT     const   BASE_WNDSIZE_PIXELS = 20;                       // �����ڵĻ�����Ԫ�����ش�С

// ���̺�������
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT OnCreate(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnDestroy(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnActivate(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnClose(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnPaint(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnSettingChange(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnContextMenu(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnDisplayChange(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnCommand(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnTimer(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnInitMenuPopup(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnMouseMove(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnLButtonDown(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnLButtonUp(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);
static LRESULT OnDpiChanged(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam);


// ���̴���ʵ��
static LRESULT OnCreate(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

static LRESULT OnDestroy(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

static LRESULT OnActivate(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

static LRESULT OnClose(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // ������ͨ�˳�, ֻ��ͨ���˵��˳�
    DestroyWindow(pWndData->hWnd);
    return 0;
}

static LRESULT OnPaint(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    
    return 0;
}

static LRESULT OnSettingChange(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

static LRESULT OnContextMenu(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
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

static LRESULT OnDisplayChange(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{

    SIZE newResolution = { 0 };
    newResolution.cx = LOWORD(lParam);
    newResolution.cy = HIWORD(lParam);
    return 0;
}

static LRESULT OnCommand(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) Menu: FALSE, Accelerator: TRUE
    // LOWORD(wParam) identifier
    // lParam: 0
    return 0;
}

static LRESULT OnTimer(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{

    return 0;
}

static LRESULT OnInitMenuPopup(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{

    return 0;
}

static LRESULT OnMouseMove(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{

    return 0;
}

static LRESULT OnLButtonDown(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // ������λ��
    pWndData->ptDragSrc.x = GET_X_LPARAM(lParam);
    pWndData->ptDragSrc.y = GET_Y_LPARAM(lParam);
    return 0;
}

static LRESULT OnLButtonUp(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // TODO
    return DefWindowProcW(pWndData->hWnd, WM_LBUTTONUP, wParam, lParam);
}

static LRESULT OnDpiChanged(PSPRITEWNDDATA pWndData, WPARAM wParam, LPARAM lParam)
{
    // �嵥�ļ�����Ҫ������� DPI Awareness ����Ϊ Per Monitor High DPI Aware ���ܽ��մ���Ϣ
    // ���¼��㴰���С

    // �ػ洰������
    InvalidateRect(pWndData->hWnd, NULL, TRUE);

    return 0;
}

// ��Ϣӳ��
static LRESULT CALLBACK SpriteWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PSPRITEWNDDATA pWndData = (PSPRITEWNDDATA)GetWndData(hWnd);
    switch (uMsg)
    {
    case WM_NCCREATE:
        pWndData = (PSPRITEWNDDATA)DefAllocMem(sizeof(MAINWNDDATA));
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
    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}


ATOM RegisterSpriteWnd(HINSTANCE hInstance)
{
    // ע�ᴰ��
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.lpszClassName = SPRITEWNDCLASSNAME;
    wcex.lpfnWndProc = SpriteWndProc;
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

HWND CreateSpriteWnd(HINSTANCE hInstance, LPVOID pAppData)
{
    return CreateWindowExW(
        //WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        WS_EX_OVERLAPPEDWINDOW,
        SPRITEWNDCLASSNAME, NULL,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        NULL, NULL, hInstance, pAppData
    );
}


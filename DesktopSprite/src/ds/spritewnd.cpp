#include <ds/spritewnd.h>
#include <ds/framework.h>

#include <ds/util.h>


using namespace Gdiplus;

// 常量定义
static UINT     const   REFRESHINTERVAL = 1000;                         // 屏幕显示刷新间隔
static UINT     const   BASE_WNDSIZE_PIXELS = 20;                       // 主窗口的基本单元格像素大小

// 过程函数声明
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


// 过程处理实现
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
    // 屏蔽普通退出, 只能通过菜单退出
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
    // 保存点击位置
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
    // 清单文件里需要将程序的 DPI Awareness 设置为 Per Monitor High DPI Aware 才能接收此消息
    // 重新计算窗体大小

    // 重绘窗体内容
    InvalidateRect(pWndData->hWnd, NULL, TRUE);

    return 0;
}

// 消息映射
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
    // 注册窗体
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


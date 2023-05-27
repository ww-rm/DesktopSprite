#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/basewindow.h>

LRESULT CALLBACK BaseWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BaseWindow* this_ = (BaseWindow*)GetWndData(hWnd);
    if (uMsg == WM_NCCREATE)
    {
        this_ = (BaseWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
        this_->hWnd = hWnd; // 必须在这里赋值, 否则之后的 OnCreate 消息无法正确获得窗口句柄
        SetWndData(hWnd, (LONG_PTR)this_);
    }

    if (this_)
    {
        return this_->HandleMessage(uMsg, wParam, lParam);
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

inline HWND BaseWindow::GetWindowHandle() const
{
    return this->hWnd;
}

BOOL BaseWindow::CreateWindow_(HINSTANCE hInst)
{
    if (this->hWnd)
    {
        return TRUE;
    }

    if (!hInst)
    {
        hInst = GetModuleHandleW(NULL);
    }

    // 注册窗口类
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.lpszClassName = this->GetClassName_();
    wcex.lpfnWndProc = BaseWindow::WindowProc;
    wcex.hInstance = hInst;
    wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.hIconSm = NULL;
    if (!RegisterClassExW(&wcex))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    // 默认窗口置顶, 无任务栏按钮, 可以半透明绘制
    // POPUP 窗口位置和大小参数忽略 CW_USEDEFAULT, 所以直接设置全 0, 然后窗口创建过程自己处理位置和大小.
    if (!CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        this->GetClassName_(),
        NULL, WS_POPUP,
        0, 0, 1, 1, // 这里要留有一点点窗口大小, 否则有些系统效果在刚创建窗口时不会生效, 比如 CS_DROPSHADOW
        NULL, NULL,
        hInst,
        (LPVOID)this
    ))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    // 注册任务栏重建消息
    this->uMsgTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");

    return TRUE;
}

INT_PTR CALLBACK BaseDialog::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BaseDialog* this_ = (BaseDialog*)GetDlgData(hDlg);
    if (uMsg == WM_INITDIALOG)
    {
        this_ = (BaseDialog*)lParam;
        this_->hDlg = hDlg;
        SetDlgData(hDlg, (LONG_PTR)this_);
    }

    if (this_)
    {
        return this_->HandleMessage(uMsg, wParam, lParam);
    }
    return FALSE;
}

inline HWND BaseDialog::GetDialogHandle() const
{
    return this->hDlg;
}

INT_PTR BaseDialog::ShowDialogBox(HINSTANCE hInst, HWND hWndParent)
{
    return DialogBoxParamW(hInst, this->GetTemplateName(), hWndParent, this->DialogProc, (LPARAM)this);
}
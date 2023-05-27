#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/basewindow.h>

LRESULT CALLBACK BaseWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BaseWindow* this_ = (BaseWindow*)GetWndData(hWnd);
    if (uMsg == WM_NCCREATE)
    {
        this_ = (BaseWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
        this_->hWnd = hWnd; // ���������︳ֵ, ����֮��� OnCreate ��Ϣ�޷���ȷ��ô��ھ��
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

    // ע�ᴰ����
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

    // Ĭ�ϴ����ö�, ����������ť, ���԰�͸������
    // POPUP ����λ�úʹ�С�������� CW_USEDEFAULT, ����ֱ������ȫ 0, Ȼ�󴰿ڴ��������Լ�����λ�úʹ�С.
    if (!CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        this->GetClassName_(),
        NULL, WS_POPUP,
        0, 0, 1, 1, // ����Ҫ����һ��㴰�ڴ�С, ������ЩϵͳЧ���ڸմ�������ʱ������Ч, ���� CS_DROPSHADOW
        NULL, NULL,
        hInst,
        (LPVOID)this
    ))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    // ע���������ؽ���Ϣ
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
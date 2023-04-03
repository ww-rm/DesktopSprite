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

HWND BaseWindow::WindowHandle() const
{
    return this->hWnd;
}
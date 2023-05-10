#pragma once
#ifndef DS_BASEWINDOW_H
#define DS_BASEWINDOW_H

#include <ds/framework.h>

#define WM_NOTIFYICON                   (WM_USER + 1)           // 通知区域消息
#define WM_TIMEALARM                    (WM_USER + 2)           // 报时消息

class BaseWindow
{
protected:
    HWND hWnd = NULL;
    UINT uMsgTaskbarCreated = 0;

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual PCWSTR GetClassName_() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

public:
    HWND GetWindowHandle() const { return this->hWnd; }
    BOOL CreateWindow_(HINSTANCE hInst = NULL);
};


#endif // !DS_BASEWINDOW_H

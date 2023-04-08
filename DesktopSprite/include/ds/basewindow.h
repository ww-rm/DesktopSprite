#pragma once
#ifndef DS_BASEWINDOW_H
#define DS_BASEWINDOW_H

#include <ds/framework.h>

class BaseWindow
{
protected:
    HWND hWnd = NULL;

protected:
    virtual PCWSTR  GetClassName_() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

public:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HWND GetWindowHandle() const;
};


#endif // !DS_BASEWINDOW_H

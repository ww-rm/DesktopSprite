#pragma once

#include <ds/framework.h>
#include <spine/spine.h>

#include <ds/basewindow.h>

using namespace Gdiplus;


class SpineChar
{
private:
    HANDLE dataMutex        = NULL;

public:
    BOOL Lock();
    BOOL UnLock();
};

class SpineRenderer
{
public:
    SpineRenderer(SpineChar* spineChar, HWND hWnd);
};

class SpriteWindow : public BaseWindow
{
private:
    POINT                               ptDragSrc = { 0 };              // 拖动窗口时的源点

public:
    PCWSTR GetClassName_() const { return L"DesktopSpriteSpriteWndClass"; }

public:
    SpriteWindow() {}

private:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
    LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
};
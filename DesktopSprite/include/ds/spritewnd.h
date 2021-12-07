#pragma once

#include <ds/framework.h>
#include <ds/mainwnd.h>

using namespace Gdiplus;

#define SPRITEWNDCLASSNAME                L"SpriteWndClass"


typedef struct _SPRITEWNDDATA 
{
    PMAINWNDDATA pAppData;                  // 应用数据


    HWND hWnd;                              // 窗体句柄

    POINT ptDragSrc;
} SPRITEWNDDATA, *PSPRITEWNDDATA;

// 注册 Sprite 窗口
ATOM RegisterSpriteWnd(HINSTANCE hInstance);

// Sprite 窗口创建函数
HWND CreateSpriteWnd(HINSTANCE hInstance, LPVOID pAppData);
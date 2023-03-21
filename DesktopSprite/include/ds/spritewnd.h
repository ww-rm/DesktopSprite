#pragma once

#include <ds/framework.h>
#include <ds/mainwnd.h>

using namespace Gdiplus;

#define SPRITEWNDCLASSNAME                L"SpriteWndClass"


typedef struct _SPRITEWNDDATA 
{
    PMAINWNDDATA pAppData;                  // Ӧ������


    HWND hWnd;                              // ������

    POINT ptDragSrc;
} SPRITEWNDDATA, *PSPRITEWNDDATA;

// ע�� Sprite ����
ATOM RegisterSpriteWnd(HINSTANCE hInstance);

// Sprite ���ڴ�������
HWND CreateSpriteWnd(HINSTANCE hInstance, LPVOID pAppData);
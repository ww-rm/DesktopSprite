#pragma once
#include "framework.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAINWNDCLASSNAME                L"DesktopSpriteMainWndClass"

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define WM_NOTIFYICON                   (WM_USER + 1)           // ֪ͨ������Ϣ
#define WM_TIMEALARM                    (WM_USER + 2)           // ��ʱ��Ϣ

    // ע��������
    ATOM RegisterMainWnd(HINSTANCE hInstance);

    // �����ڴ�������
    HWND CreateMainWnd(HINSTANCE hInstance);

    // Ӧ��������
    //DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

#ifdef __cplusplus
}
#endif // __cplusplus
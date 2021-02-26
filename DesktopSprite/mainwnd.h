#pragma once
#include "framework.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAINWNDCLASSNAME                L"DesktopSpriteMainWndClass"

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define ID_NIDMAIN                      1

#define REFRESHINTERVAL                 1000

#define WM_NOTIFYICON                   (WM_USER + 1)           // ֪ͨ������Ϣ
#define WM_TIMEALARM                    (WM_USER + 2)           // ��ʱ��Ϣ

    // ע��������
    ATOM RegisterMainWnd(HINSTANCE hInstance);

    // �����ڴ�������
    HWND CreateMainWnd(HINSTANCE hInstance);

    // Ӧ��������
    DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

    // ��ȡ������ʱ������
    UINT GetHourTimeDiff();

    // ���ò˵���״̬
    DWORD SetMenuItemState(HMENU hMenu, UINT uIdentifier, UINT uState);

    // ��ò˵���״̬
    UINT GetMenuItemState(HMENU hMenu, UINT uIdentifier);

#ifdef __cplusplus
}
#endif // __cplusplus
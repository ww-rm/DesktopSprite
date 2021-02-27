#pragma once
#include "framework.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAINWNDCLASSNAME                L"DesktopSpriteMainWndClass"

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define WM_NOTIFYICON                   (WM_USER + 1)           // 通知区域消息
#define WM_TIMEALARM                    (WM_USER + 2)           // 报时消息

    // 注册主窗口
    ATOM RegisterMainWnd(HINSTANCE hInstance);

    // 主窗口创建函数
    HWND CreateMainWnd(HINSTANCE hInstance);

    // 应用设置项
    //DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

#ifdef __cplusplus
}
#endif // __cplusplus
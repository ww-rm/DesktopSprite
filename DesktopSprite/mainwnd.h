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

    // 窗体数据
    typedef struct _MAINWNDDATA
    {
        HWND            hWnd;

        // 运行时设置项
        BOOL            bFloatWnd;                    // 是否桌面浮窗
        HFONT           hTextFont;                     // 显示文本的字体
        COLORREF        rgbText;             // 显示文本的颜色
        //WCHAR           szBalloonIconPath[MAX_PATH];                    // 气球图标文件路径
        BOOL            bInfoSound;                    // 气球提示是否有声音

        // 运行时数据
        BOOL            bWndFixed;                    // 窗口是否通过图标点击长期显示
        POINT           ptDragSrc;                    // 拖动窗口时的源点
    }*PMAINWNDDATA, MAINWNDDATA;

    // 注册主窗口
    ATOM RegisterMainWnd(HINSTANCE hInstance);

    // 主窗口创建函数
    HWND CreateMainWnd(HINSTANCE hInstance);

    // 应用设置项
    //DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

#ifdef __cplusplus
}
#endif // __cplusplus
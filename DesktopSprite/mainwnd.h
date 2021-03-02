#pragma once
#include "framework.h"
#include "config.h"

using namespace Gdiplus;

#define MAINWNDCLASSNAME                L"DesktopSpriteMainWndClass"

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define WM_NOTIFYICON                   (WM_USER + 1)           // 通知区域消息
#define WM_TIMEALARM                    (WM_USER + 2)           // 报时消息

// 窗体数据
typedef struct _MAINWNDDATA
{
    HWND            hWnd;

    // 设置项
    BOOL            bFloatWnd;                      // 是否桌面浮窗
    BOOL            bInfoSound;                     // 气球提示是否有声音
    //WCHAR           szBalloonIconPath[MAX_PATH];    // 气球图标文件路径

    // 运行时
    BOOL            bWndFixed;                      // 窗口是否通过图标点击长期显示
    POINT           ptDragSrc;                      // 拖动窗口时的源点
    LOGFONTW        lfText;                         // 显示文本的字体
}*PMAINWNDDATA, MAINWNDDATA;

// 注册主窗口
ATOM RegisterMainWnd(HINSTANCE hInstance);

// 主窗口创建函数
HWND CreateMainWnd(HINSTANCE hInstance);

// 辅助函数画圆
BOOL DrawCircle(
    Graphics& graphics,
    Pen& pen,
    const PointF& ptCenter,
    const REAL& nOuterRadius,
    const REAL& sweepPercent
);

// 辅助函数画网速图
BOOL DrawSpeedStair(
    Graphics& graphics,
    const Color& color,
    const RectF& rect,
    const BOOL& bUp,
    const INT& nLevel,
    const INT& nMaxLevel = 6
);

// 应用设置项
//DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

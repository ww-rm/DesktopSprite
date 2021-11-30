#pragma once
#include "framework.h"
#include "config.h"

using namespace Gdiplus;

#define MAINWNDCLASSNAME                L"DesktopSpriteMainWndClass"

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define WM_NOTIFYICON                   (WM_USER + 1)           // 通知区域消息
#define WM_TIMEALARM                    (WM_USER + 2)           // 报时消息

#define STATUSCOLOR_LOW                 0xff00ff00
#define STATUSCOLOR_MIDDLE              0xffff8000
#define STATUSCOLOR_HIGH                0xffff0000

// 窗体数据
typedef struct _MAINWNDDATA
{
    HWND            hWnd;

    // 主窗口设置项
    BOOL            bFloatWnd;                      // 是否桌面浮窗
    BOOL            bInfoSound;                     // 气球提示是否有声音
    //WCHAR           szBalloonIconPath[MAX_PATH];    // 气球图标文件路径
    BOOL            bDarkTheme;                     // 是否用深色主题
    BYTE            byTransparency;                 // 整体透明度    // TODO: 不保留透明度
    BYTE            byShowContent;                  // 要显示的内容(调整窗体大小)

    // 主窗口运行时
    INT             wndSizeUnit;                    // 窗口单元格大小, 在运行时随 DPI 进行缩放
    BOOL            bWndFixed;                      // 窗口是否通过图标点击长期显示
    POINT           ptDragSrc;                      // 拖动窗口时的源点
    PrivateFontCollection* pFontColl;               // 文本字体容器

    // 程序运行时
    SIZE            runtimeResolution;              // 最近运行时系统分辨率
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

// 从字体文件创建字体, 需要使用 delete 释放指针
Font* CreateFontFromFile(PCWSTR szFontFilePath, REAL emSize = 9, INT style = 0, Unit unit = UnitPoint);

// 根据当前显示内容与窗体单元格大小计算窗体大小, 像素单位
DWORD GetWndSizeByShowContent(PMAINWNDDATA pWndData, PSIZE psizeWnd, BYTE byShowContent);

// 应用设置项
//DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

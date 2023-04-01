#pragma once
#ifndef DS_MAINWND_H
#define DS_MAINWND_H

#include <ds/framework.h>
#include <ds/perfmonitor.h>
#include <ds/config.h>
#include <ds/basewindow.h>
#include <ds/notifyicon.h>
#include <ds/winapp.h>

using namespace Gdiplus;

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
Font* CreateFontFromFile(PCWSTR szFontFilePath, REAL emSize = 9, INT style = 0, Unit unit = Unit::UnitPoint);

class MainWindow :BaseWindow
{
private:
    UINT                uMsgTaskbarCreated = 0;
    const WinApp*       app = NULL;
    AppConfig           config;
    PerfMonitor         perfMonitor;
    NotifyIcon*         pNotifyIcon = NULL;

// 运行时数据
private:
    // 主窗口运行时
    INT                                 wndSizeUnit = 0;                // 窗口单元格大小, 在运行时随 DPI 进行缩放
    BOOL                                bWndFixed = FALSE;              // 窗口是否通过图标点击长期显示
    BOOL                                bClocked = FALSE;               // 保存整点内是否报过时
    POINT                               ptDragSrc = { 0 };              // 拖动窗口时的源点
    Gdiplus::PrivateFontCollection      fontColl;                       // 文本字体容器

    POINT                               lastFloatPos = { 0 };           // 最近一次浮动窗口位置
    SIZE                                lastResolution = { 0 };         // 最近运行时系统分辨率

public:
    PCWSTR GetClassName_() const;
    PCWSTR GetConfigPath() const;

public:
    MainWindow(const WinApp* app);

    DWORD LoadFloatPosDataFromReg();
    DWORD UpdateFloatPosDataToRegByCurrentResolution();

    DWORD ApplyConfig();
    DWORD ApplyConfig(PCFGDATA pcfgdata); // 应用更改, 只修改发生变化的设置项

    DWORD GetWndSizeByShowContent(PSIZE psizeWnd, BYTE byShowContent);
    DWORD TimeAlarm();

public:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    LRESULT OnActivate(WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnSettingChange(WPARAM wParam, LPARAM lParam);
    LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
    LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
    //LRESULT OnPowerBroadcast(WPARAM wParam, LPARAM lParam);
    LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimeAlarm(WPARAM wParam, LPARAM lParam);
    LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
};

#endif // !DS_MAINWND_H

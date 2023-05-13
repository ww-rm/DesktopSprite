#pragma once
#ifndef DS_MAINWND_H
#define DS_MAINWND_H

#include <ds/framework.h>
#include <ds/perfmonitor.h>
#include <ds/notifyicon.h>
#include <ds/config.h>
#include <ds/basewindow.h>
#include <ds/winapp.h>


// 窗口消息相关宏定义

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

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

class MainWindow :public BaseWindow
{
private:
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
    POINT                               currentFloatPos = { 0 };        // 在显示弹出窗口时临时保存当前浮动窗口位置
    Gdiplus::PrivateFontCollection      fontColl;                       // 文本字体容器
    HICON                               balloonIcon = NULL;             // 气泡消息的图标资源

public:
    PCWSTR GetClassName_() const { return L"DesktopSpriteMainWndClass"; }
    PCWSTR GetConfigPath() const { return L"config.json"; }

public:
    MainWindow(WinApp* app) : app(app) {}

private:
    // 显示右键菜单
    BOOL ShowContextMenu(INT x, INT y);

    // 获取在屏幕上的默认位置
    BOOL GetDefaultWindowPos(POINT* pt);

    // 根据任务栏位置计算窗口的位置
    BOOL GetPopupWindowPos(POINT* pt);

    // 是否窗口矩形区与桌面边缘相交
    BOOL IsIntersectDesktop();

    // 一对用来存储窗口位置到注册表的方法
    BOOL LoadLastPosFromReg(POINT* pt);
    BOOL SaveCurrentPosToReg();

    BOOL ApplyConfig();
    BOOL ApplyConfig(const CFGDATA* pcfgdata); // 应用更改, 只修改发生变化的设置项

    void GetWndSizeByShowContent(PSIZE psizeWnd, BYTE byShowContent);
    BOOL TimeAlarm();
    INT ShowNoConentWarningMsg();

private:
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

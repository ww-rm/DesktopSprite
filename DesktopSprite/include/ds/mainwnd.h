#pragma once
#ifndef DS_MAINWND_H
#define DS_MAINWND_H

#include <ds/framework.h>
#include <ds/perfmonitor.h>
#include <ds/notifyicon.h>
#include <ds/config.h>
#include <ds/winapp.h>
#include <ds/basewindow.h>
#include <ds/spritewnd.h>
#include <ds/configdlg.h>
#include <ds/aboutdlg.h>

class MainWindow :public BaseWindow
{
private:
    NotifyIcon* pNotifyIcon = NULL;
    SpriteWindow* spritewnd = NULL;

private:
    INT                                 wndSizeUnit = 0;                // 窗口单元格大小, 在运行时随 DPI 进行缩放
    BOOL                                bClocked = FALSE;               // 保存整点内是否报过时
    POINT                               ptDragSrc = { 0 };              // 拖动窗口时的源点
    SIZE                                sysDragSize = { 0 };            // 系统拖动判定大小
    BOOL                                isDragging = FALSE;
    POINT                               currentFloatPos = { 0 };        // 在显示弹出窗口时临时保存当前浮动窗口位置
    Gdiplus::PrivateFontCollection      fontColl;                       // 文本字体容器
    HICON                               balloonIcon = NULL;             // 气泡消息的图标资源
    WCHAR                               fontPath[MAX_PATH] = { 0 };     // 字体路径
    PerfMonitor::PERFDATA               perfData = { 0 };               // 性能数据缓冲
    ConfigDlg*                          configDlg = NULL;
    AboutDlg*                           aboutDlg = NULL;

public:
    MainWindow();

    PCWSTR GetClassName_() const;
    PCWSTR GetFontPath() const;
    SpriteWindow* GetSpriteWnd();

    // 应用更改, 只修改发生变化的设置项, 空参数则应用全局设置
    BOOL ApplyConfig(const AppConfig::AppConfig* newConfig = NULL);

    // 显示右键菜单
    BOOL ShowContextMenu(INT x, INT y);

    // 获取在屏幕上的默认位置
    BOOL GetDefaultWindowPos(POINT* pt);

    // 根据任务栏位置计算窗口的位置
    BOOL GetPopupWindowPos(POINT* pt);

    // 显示/隐藏弹出窗口
    BOOL PopupOpen();
    BOOL PopupClose();

    // 是否窗口矩形区与桌面边缘相交
    BOOL IsIntersectDesktop();

    // 一对用来存储窗口位置到注册表的方法
    BOOL LoadLastPosFromReg(POINT* pt);
    BOOL SaveCurrentPosToReg();

    // 根据电脑颜色主题加载通知区域图标
    HICON LoadNotifyIconBySysTheme();

    void GetWndSizeByShowContent(PSIZE psizeWnd, BYTE byShowContent);

    BOOL TimeAlarm();
    BOOL CheckAndTimeAlarm();

    INT ShowNoConentWarningMsg();

private:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
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
    LRESULT OnLButtonDBClick(WPARAM wParam, LPARAM lParam);
    LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
    LRESULT OnPerfDataUpdated(WPARAM wParam, LPARAM lParam);
    LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
};

#endif // !DS_MAINWND_H

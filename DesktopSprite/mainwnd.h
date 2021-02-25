#pragma once
#include "framework.h"
#include "config.h"

#define MAINWNDCLASSNAME                L"MainWndClass"

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define ID_NIDMAIN                      1

#define REFRESHINTERVAL                 1000

#define WM_NOTIFYICON                   (WM_USER + 1)           // 通知区域消息
#define WM_TIMEALARM                    (WM_USER + 2)           // 报时消息

// 注册主窗口
ATOM RegisterMainWnd(HINSTANCE hInstance);

// 主窗口创建函数
HWND CreateMainWnd(HINSTANCE hInstance);

// 应用设置项
DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

// 获取与整点时间差毫秒
UINT GetHourTimeDiff();

// 设置菜单项状态
DWORD SetMenuItemState(HMENU hMenu, UINT uIdentifier, UINT uState);

// 获得菜单项状态
UINT GetMenuItemState(HMENU hMenu, UINT uIdentifier);

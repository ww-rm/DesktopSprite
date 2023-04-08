#pragma once
#ifndef DS_CONFIG_H
#define DS_CONFIG_H

#include <ds/framework.h>

// 标记要显示的内容
#define SHOWCONTENT_CPUMEM              0x1
#define SHOWCONTENT_NETSPEED            0x2

typedef struct _CFGDATA
{
    BOOL        bFloatWnd;                      // 是否显示主窗口
    BOOL        bAutoRun;                       // 开机自启
    BOOL        bTimeAlarm;                     // 整点报时
    WCHAR       szBalloonIconPath[MAX_PATH];    // 气泡图标路径
    BOOL        bInfoSound;                     // 气泡消息声音
    BOOL        bDarkTheme;                     // 是否使用深色主题
    DOUBLE      transparencyPercent;            // 透明度
    BYTE        byShowContent;                  // 要显示的内容
}CFGDATA;

class AppConfig
{
public:
    BOOL    bFloatWnd = TRUE;                                           // 是否显示主窗口
    BOOL    bAutoRun = FALSE;                                           // 开机自启
    BOOL    bTimeAlarm = TRUE;                                          // 整点报时
    WCHAR   szBalloonIconPath[MAX_PATH] = L"res\\image\\timealarm.ico"; // 气泡图标路径
    BOOL    bInfoSound = TRUE;                                          // 气泡消息声音
    BOOL    bDarkTheme = TRUE;                                          // 是否使用深色主题
    DOUBLE  transparencyPercent = 75.0;                                 // 透明度
    BYTE    byShowContent = SHOWCONTENT_CPUMEM | SHOWCONTENT_NETSPEED;  // 要显示的内容

public:
    void Get(CFGDATA* pcfgdata) const;;
    void Set(const CFGDATA* pcfgdata);

    DWORD LoadFromFile(PCWSTR path);
    DWORD SaveToFile(PCWSTR path);
};

#endif // !DS_CONFIG_H

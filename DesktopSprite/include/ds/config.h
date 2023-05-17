#pragma once
#ifndef DS_CONFIG_H
#define DS_CONFIG_H

#include <ds/framework.h>

// 标记要显示的内容
#define SHOWCONTENT_CPUMEM              0x1
#define SHOWCONTENT_NETSPEED            0x2

class AppConfig
{
private:
    WCHAR   rootDir[MAX_PATH] = { 0 };

private:
    PCWSTR  GetRootDir() const { return this->rootDir; }

public:
    BOOL    bFloatWnd = TRUE;                                           // 是否显示主窗口
    BOOL    bAutoRun = FALSE;                                           // 开机自启
    BOOL    bTimeAlarm = TRUE;                                          // 整点报时
    WCHAR   szBalloonIconPath[MAX_PATH] = { 0 };                        // 气泡图标路径
    BOOL    bInfoSound = TRUE;                                          // 气泡消息声音
    BOOL    bDarkTheme = TRUE;                                          // 是否使用深色主题
    UINT    transparencyPercent = 75;                                   // 透明度
    BYTE    byShowContent = SHOWCONTENT_CPUMEM | SHOWCONTENT_NETSPEED;  // 要显示的内容

public:
    AppConfig();
    AppConfig(const AppConfig& other);
    AppConfig& operator= (const AppConfig& other);

    BOOL LoadFromFile(PCWSTR path);
    BOOL SaveToFile(PCWSTR path);
};

#endif // !DS_CONFIG_H

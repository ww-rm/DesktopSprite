#pragma once
#ifndef DS_CONFIG_H
#define DS_CONFIG_H

#include <ds/framework.h>

// 标记要显示的内容
#define SHOWCONTENT_CPUMEM              0x1
#define SHOWCONTENT_NETSPEED            0x2

namespace AppConfig {

    class AppConfig
    {
    public:
        // 程序和主窗口设置
        BOOL    bFloatWnd = TRUE;                                           // 是否显示主窗口
        BOOL    bAutoRun = FALSE;                                           // 开机自启
        BOOL    bTimeAlarm = TRUE;                                          // 整点报时
        WCHAR   szBalloonIconPath[MAX_PATH] = { 0 };                        // 气泡图标路径
        BOOL    bInfoSound = TRUE;                                          // 气泡消息声音
        BOOL    bDarkTheme = TRUE;                                          // 是否使用深色主题
        UINT    transparencyPercent = 75;                                   // 透明度
        BYTE    byShowContent = SHOWCONTENT_CPUMEM | SHOWCONTENT_NETSPEED;  // 要显示的内容

        // sprite 设置
        BOOL bShowSprite = TRUE;
        UINT maxFps = 30;
        UINT scale = 100;
        UINT spTransparencyPercent = 100;
        WCHAR szSpineAtlasPath[MAX_PATH] = { 0 };
        WCHAR szSpineSkelPath[MAX_PATH] = { 0 };

        // spine 设置
        WCHAR spAnimeIdle[MAX_PATH] = L"normal";
        WCHAR spAnimeDrag[MAX_PATH] = L"tuozhuai";
        WCHAR spAnimeWork[MAX_PATH] = L"walk";
        WCHAR spAnimeSleep[MAX_PATH] = L"sleep";
        WCHAR spAnimeStand[MAX_PATH] = L"stand";
        WCHAR spAnimeTouch[MAX_PATH] = L"touch";
        WCHAR spAnimeWink[MAX_PATH] = L"motou";
        WCHAR spAnimeVictory[MAX_PATH] = L"victory";
        WCHAR spAnimeDance[MAX_PATH] = L"dance";
        WCHAR spAnimeDizzy[MAX_PATH] = L"yun";

    public:
        AppConfig();
        AppConfig(const AppConfig& other);
        AppConfig& operator= (const AppConfig& other);

        BOOL LoadFromFile(PCWSTR path);
        BOOL SaveToFile(PCWSTR path);
    };

    BOOL Init();
    BOOL Uninit();

    const AppConfig* Get();
    void Get(AppConfig* other);
    void Set(const AppConfig* other);

    BOOL LoadFromFile(PCWSTR path);
    BOOL SaveToFile(PCWSTR path);
}

#endif // !DS_CONFIG_H

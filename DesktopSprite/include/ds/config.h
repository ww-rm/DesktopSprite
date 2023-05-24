#pragma once
#ifndef DS_CONFIG_H
#define DS_CONFIG_H

#include <ds/framework.h>

// ���Ҫ��ʾ������
#define SHOWCONTENT_CPUMEM              0x1
#define SHOWCONTENT_NETSPEED            0x2

class SpriteConfig
{
public:
    INT maxFps = 30;
    WCHAR szSpineAtlasPath[MAX_PATH] = { 0 };
    WCHAR szSpineSkelPath[MAX_PATH] = { 0 };

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
};

class AppConfig
{
public:
    BOOL    bFloatWnd = TRUE;                                           // �Ƿ���ʾ������
    BOOL    bAutoRun = FALSE;                                           // ��������
    BOOL    bTimeAlarm = TRUE;                                          // ���㱨ʱ
    WCHAR   szBalloonIconPath[MAX_PATH] = { 0 };                        // ����ͼ��·��
    BOOL    bInfoSound = TRUE;                                          // ������Ϣ����
    BOOL    bDarkTheme = TRUE;                                          // �Ƿ�ʹ����ɫ����
    UINT    transparencyPercent = 75;                                   // ͸����
    BYTE    byShowContent = SHOWCONTENT_CPUMEM | SHOWCONTENT_NETSPEED;  // Ҫ��ʾ������

public:
    AppConfig();
    AppConfig(const AppConfig& other);
    AppConfig& operator= (const AppConfig& other);

    BOOL LoadFromFile(PCWSTR path);
    BOOL SaveToFile(PCWSTR path);
};

#endif // !DS_CONFIG_H

#pragma once
#ifndef DS_CONFIG_H
#define DS_CONFIG_H

#include <ds/framework.h>

// ���Ҫ��ʾ������
#define SHOWCONTENT_CPUMEM              0x1
#define SHOWCONTENT_NETSPEED            0x2

class AppConfig
{
public:
    BOOL    bFloatWnd = TRUE;                                           // �Ƿ���ʾ������
    BOOL    bAutoRun = FALSE;                                           // ��������
    BOOL    bTimeAlarm = TRUE;                                          // ���㱨ʱ
    WCHAR   szBalloonIconPath[MAX_PATH] = L"res\\image\\timealarm.ico"; // ����ͼ��·��
    BOOL    bInfoSound = TRUE;                                          // ������Ϣ����
    BOOL    bDarkTheme = TRUE;                                          // �Ƿ�ʹ����ɫ����
    UINT    transparencyPercent = 75;                                 // ͸����
    BYTE    byShowContent = SHOWCONTENT_CPUMEM | SHOWCONTENT_NETSPEED;  // Ҫ��ʾ������

public:
    AppConfig() {};
    AppConfig(const AppConfig& other);
    AppConfig& operator= (const AppConfig& other);

    BOOL LoadFromFile(PCWSTR path);
    BOOL SaveToFile(PCWSTR path);
};

#endif // !DS_CONFIG_H

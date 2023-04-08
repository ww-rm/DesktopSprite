#pragma once
#ifndef DS_CONFIG_H
#define DS_CONFIG_H

#include <ds/framework.h>

// ���Ҫ��ʾ������
#define SHOWCONTENT_CPUMEM              0x1
#define SHOWCONTENT_NETSPEED            0x2

typedef struct _CFGDATA
{
    BOOL        bFloatWnd;                      // �Ƿ���ʾ������
    BOOL        bAutoRun;                       // ��������
    BOOL        bTimeAlarm;                     // ���㱨ʱ
    WCHAR       szBalloonIconPath[MAX_PATH];    // ����ͼ��·��
    BOOL        bInfoSound;                     // ������Ϣ����
    BOOL        bDarkTheme;                     // �Ƿ�ʹ����ɫ����
    DOUBLE      transparencyPercent;            // ͸����
    BYTE        byShowContent;                  // Ҫ��ʾ������
}CFGDATA;

class AppConfig
{
public:
    BOOL    bFloatWnd = TRUE;                                           // �Ƿ���ʾ������
    BOOL    bAutoRun = FALSE;                                           // ��������
    BOOL    bTimeAlarm = TRUE;                                          // ���㱨ʱ
    WCHAR   szBalloonIconPath[MAX_PATH] = L"res\\image\\timealarm.ico"; // ����ͼ��·��
    BOOL    bInfoSound = TRUE;                                          // ������Ϣ����
    BOOL    bDarkTheme = TRUE;                                          // �Ƿ�ʹ����ɫ����
    DOUBLE  transparencyPercent = 75.0;                                 // ͸����
    BYTE    byShowContent = SHOWCONTENT_CPUMEM | SHOWCONTENT_NETSPEED;  // Ҫ��ʾ������

public:
    void Get(CFGDATA* pcfgdata) const;;
    void Set(const CFGDATA* pcfgdata);

    DWORD LoadFromFile(PCWSTR path);
    DWORD SaveToFile(PCWSTR path);
};

#endif // !DS_CONFIG_H

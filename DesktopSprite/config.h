#pragma once
#include "framework.h"

// ���Ҫ��ʾ������
#define SHOWCONTENT_CPUMEM          0x1
#define SHOWCONTENT_NETSPEED        0x2

// ��������������
typedef struct _CFGDATA
{
    // TODO: Ӧ������
    BOOL        bFloatWnd;                      // �Ƿ���ʾ������
    BOOL        bAutoRun;                       // ��������
    BOOL        bTimeAlarm;                     // ���㱨ʱ
    //WCHAR       szBalloonIconPath[MAX_PATH];    // ����ͼ��·��
    BOOL        bInfoSound;                     // ������Ϣ����
    BOOL        bDarkTheme;                     // �Ƿ�ʹ����ɫ����
    BYTE        byTransparency;                 // ͸����
    BYTE        byShowContent;               // Ҫ��ʾ������

    // ��������
    POINT       ptLastFloatPos;                 // ��һ�θ�������Ļλ��
}*PCFGDATA, CFGDATA;

// ����Ĭ�����ò���
DWORD LoadDefaultConfig(PCFGDATA pCfgData);

//DWORD LoadConfigFromFile(PCFGDATA pCfgData);

//DWORD SaveConfigToFile(PCFGDATA pCfgData);

DWORD LoadConfigFromReg(PCFGDATA pCfgData);

DWORD SaveConfigToReg(PCFGDATA pCfgData);

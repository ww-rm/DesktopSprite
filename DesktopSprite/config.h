#pragma once
#include "framework.h"

// ��������������
typedef struct _CFGDATA
{
    // TODO: Ӧ������
    BOOL        bFloatWnd;                      // �Ƿ���ʾ������
    BOOL        bAutoRun;                       // ��������
    BOOL        bTimeAlarm;                     // ���㱨ʱ
    //WCHAR       szBalloonIconPath[MAX_PATH];    // ����ͼ��·��
    BOOL        bInfoSound;                     // ������Ϣ����
    LOGFONTW    lfText;                         // ���ڵ�����
    COLORREF    rgbTextColor;                   // ����������ɫ
}*PCFGDATA, CFGDATA;

// ����Ĭ�����ò���
DWORD LoadDefaultConfig(PCFGDATA pCfgData);

//DWORD LoadConfigFromFile(PCFGDATA pCfgData);

//DWORD SaveConfigToFile(PCFGDATA pCfgData);

DWORD LoadConfigFromReg(PCFGDATA pCfgData);

DWORD SaveConfigToReg(PCFGDATA pCfgData);

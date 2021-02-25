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

// ��ע����Ӽ�д�����������
#define RegSetBinValue(hKey, lpValueName, lpData, cbData) \
        RegSetValueExW((hKey), (lpValueName), 0, REG_BINARY, (PBYTE)(lpData), (cbData))

// ��ע����Ӽ���ѯ����������
#define RegQueryBinValue(hKey, lpValueName, lpData, lpcbData) \
        RegQueryValueExW((hKey), (lpValueName), NULL, NULL, (PBYTE)(lpData), (lpcbData))

// �� HEKY_CURRENT_USER\SOFTWARE �Ӽ�
#define RegOpenSoftwareKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE", 0, KEY_ALL_ACCESS, (phkResult))

// �� HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
#define RegOpenRunKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, (phkResult))

// ����Ĭ�����ò���
DWORD LoadDefaultConfig(PCFGDATA pCfgData);

//DWORD LoadConfigFromFile(PCFGDATA pCfgData);

//DWORD SaveConfigToFile(PCFGDATA pCfgData);

DWORD LoadConfigFromReg(PCFGDATA pCfgData);

DWORD SaveConfigToReg(PCFGDATA pCfgData);

DWORD SetAppAutoRun();

DWORD UnsetAppAutoRun();

DWORD GetSystemCapitalFont(PLOGFONTW pLogFont);
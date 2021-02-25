#pragma once
#include "framework.h"

// 程序所有设置项
typedef struct _CFGDATA
{
    // TODO: 应用设置
    BOOL        bFloatWnd;                      // 是否显示主窗口
    BOOL        bAutoRun;                       // 开机自启
    BOOL        bTimeAlarm;                     // 整点报时
    //WCHAR       szBalloonIconPath[MAX_PATH];    // 气泡图标路径
    BOOL        bInfoSound;                     // 气泡消息声音
    LOGFONTW    lfText;                         // 窗口的字体
    COLORREF    rgbTextColor;                   // 窗口字体颜色
}*PCFGDATA, CFGDATA;

// 向注册表子键写入二进制数据
#define RegSetBinValue(hKey, lpValueName, lpData, cbData) \
        RegSetValueExW((hKey), (lpValueName), 0, REG_BINARY, (PBYTE)(lpData), (cbData))

// 从注册表子键查询二进制数据
#define RegQueryBinValue(hKey, lpValueName, lpData, lpcbData) \
        RegQueryValueExW((hKey), (lpValueName), NULL, NULL, (PBYTE)(lpData), (lpcbData))

// 打开 HEKY_CURRENT_USER\SOFTWARE 子键
#define RegOpenSoftwareKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE", 0, KEY_ALL_ACCESS, (phkResult))

// 打开 HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
#define RegOpenRunKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, (phkResult))

// 加载默认配置参数
DWORD LoadDefaultConfig(PCFGDATA pCfgData);

//DWORD LoadConfigFromFile(PCFGDATA pCfgData);

//DWORD SaveConfigToFile(PCFGDATA pCfgData);

DWORD LoadConfigFromReg(PCFGDATA pCfgData);

DWORD SaveConfigToReg(PCFGDATA pCfgData);

DWORD SetAppAutoRun();

DWORD UnsetAppAutoRun();

DWORD GetSystemCapitalFont(PLOGFONTW pLogFont);
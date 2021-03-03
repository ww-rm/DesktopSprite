#pragma once
#include "framework.h"

// 标记要显示的内容
#define SHOWCONTENT_CPUMEM          0x1
#define SHOWCONTENT_NETSPEED        0x2

// 程序所有设置项
typedef struct _CFGDATA
{
    // TODO: 应用设置
    BOOL        bFloatWnd;                      // 是否显示主窗口
    BOOL        bAutoRun;                       // 开机自启
    BOOL        bTimeAlarm;                     // 整点报时
    //WCHAR       szBalloonIconPath[MAX_PATH];    // 气泡图标路径
    BOOL        bInfoSound;                     // 气泡消息声音
    BOOL        bDarkTheme;                     // 是否使用深色主题
    BYTE        byTransparency;                 // 透明度
    BYTE        byShowContent;               // 要显示的内容

    // 记忆数据
    POINT       ptLastFloatPos;                 // 上一次浮窗的屏幕位置
}*PCFGDATA, CFGDATA;

// 加载默认配置参数
DWORD LoadDefaultConfig(PCFGDATA pCfgData);

//DWORD LoadConfigFromFile(PCFGDATA pCfgData);

//DWORD SaveConfigToFile(PCFGDATA pCfgData);

DWORD LoadConfigFromReg(PCFGDATA pCfgData);

DWORD SaveConfigToReg(PCFGDATA pCfgData);

#pragma once
#include "framework.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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

        // 上一次的运行数据
        POINT       ptLastFloatPos;                 // 上一次浮窗的屏幕位置
    }*PCFGDATA, CFGDATA;

    // 加载默认配置参数
    DWORD LoadDefaultConfig(PCFGDATA pCfgData);

    //DWORD LoadConfigFromFile(PCFGDATA pCfgData);

    //DWORD SaveConfigToFile(PCFGDATA pCfgData);

    DWORD LoadConfigFromReg(PCFGDATA pCfgData);

    DWORD SaveConfigToReg(PCFGDATA pCfgData);

#ifdef __cplusplus
}
#endif // __cplusplus

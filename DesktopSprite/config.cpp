#include "framework.h"
#include "util.h"

#include "config.h"

// TODO: 设置项注册表名称
static PCWSTR const REGVAL_SHOWMAINWND              = L"IsFloatWnd";
static PCWSTR const REGVAL_AUTORUN                  = L"IsAutoRun";
static PCWSTR const REGVAL_TIMEAlARM                = L"IsTimeAlarm";
//static PCWSTR const REGVAL_BALLOONICONPATH          = L"BalloonIconPath";
static PCWSTR const REGVAL_INFOSOUND                = L"IsInfoSound";
//static PCWSTR const REGVAL_DARKTHEME                = L"IsDarkTheme";
//static PCWSTR const REGVAL_TRANSPARENCY             = L"Transparency";

static PCWSTR const REGVAL_LASTFLOATPOS             = L"LastFloatPos";


DWORD LoadDefaultConfig(PCFGDATA pCfgData)
{
    // TODO: 应用默认设置
    pCfgData->bFloatWnd = FALSE;                            // 隐藏浮窗
    pCfgData->bAutoRun = FALSE;                             // 禁止开机自启
    pCfgData->bTimeAlarm = TRUE;                            // 开启整点报时

    //WCHAR szExeFullDir[MAX_PATH] = { 0 };
    //GetModuleFileNameW(NULL, szExeFullDir, MAX_PATH);
    //PathCchRemoveFileSpec(szExeFullDir, MAX_PATH);
    //PathCchCombine(pCfgData->szBalloonIconPath, MAX_PATH, szExeFullDir, L"data\\default_balloonicon.ico");

    pCfgData->bInfoSound = TRUE;                            // 开启提示声音

    // 默认位置是屏幕的 1/6 处
    RECT rcScreen = { 0 };
    GetWindowRect(GetDesktopWindow(), &rcScreen);
    pCfgData->ptLastFloatPos.x = rcScreen.right * 5 / 6;
    pCfgData->ptLastFloatPos.y = rcScreen.bottom * 1 / 6;
    return 0;
}

DWORD LoadConfigFromReg(PCFGDATA pCfgData)
{
    LoadDefaultConfig(pCfgData); // 保证有默认值

    DWORD dwErrorCode = ERROR_SUCCESS;

    // 打开注册表项
    HKEY hkSoftware = NULL;
    HKEY hkApp = NULL;
    DWORD cbData = 0;

    dwErrorCode = RegOpenSoftwareKey(&hkSoftware);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        dwErrorCode = RegOpenKeyExW(hkSoftware, APPNAME, 0, KEY_ALL_ACCESS, &hkApp);
        if (dwErrorCode == ERROR_SUCCESS)
        {
            // TODO: 读取配置
            cbData = sizeof(BOOL);
            RegQueryAnyValue(hkApp, REGVAL_SHOWMAINWND, &pCfgData->bFloatWnd, &cbData);
            cbData = sizeof(BOOL);
            RegQueryAnyValue(hkApp, REGVAL_AUTORUN, &pCfgData->bAutoRun, &cbData);
            cbData = sizeof(BOOL);
            RegQueryAnyValue(hkApp, REGVAL_TIMEAlARM, &pCfgData->bTimeAlarm, &cbData);
            //cbData = MAX_PATH;
            //RegQueryValueExW(hkApp, REGVAL_BALLOONICONPATH, 0, NULL, (PBYTE)pCfgData->szBalloonIconPath, &cbData);
            cbData = sizeof(BOOL);
            RegQueryAnyValue(hkApp, REGVAL_INFOSOUND, &pCfgData->bInfoSound, &cbData);

            cbData = sizeof(POINT);
            RegQueryAnyValue(hkApp, REGVAL_LASTFLOATPOS, &pCfgData->ptLastFloatPos, &cbData);
        }
        else if (dwErrorCode != ERROR_FILE_NOT_FOUND)
        {
            // TODO: 错误处理
        }
    }

    // 关闭注册表
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    if (hkSoftware != NULL)
    {
        RegCloseKey(hkSoftware);
    }
    return dwErrorCode;
}

DWORD SaveConfigToReg(PCFGDATA pCfgData)
{
    DWORD dwErrorCode = ERROR_SUCCESS;

    // 打开注册表项
    HKEY hkSoftware = NULL;
    HKEY hkApp = NULL;
    DWORD cbData = 0;

    dwErrorCode = RegOpenSoftwareKey(&hkSoftware);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        dwErrorCode = RegCreateKeyExW(hkSoftware, APPNAME, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, NULL);
        if (dwErrorCode == ERROR_SUCCESS)
        {
            // TODO: 保存应用配置
            RegSetBinValue(hkApp, REGVAL_SHOWMAINWND, &pCfgData->bFloatWnd, sizeof(BOOL));
            RegSetBinValue(hkApp, REGVAL_AUTORUN, &pCfgData->bAutoRun, sizeof(BOOL));
            RegSetBinValue(hkApp, REGVAL_TIMEAlARM, &pCfgData->bTimeAlarm, sizeof(BOOL));
            //SIZE_T cbData = 0;
            //StringCchLengthW(pCfgData->szBalloonIconPath, MAX_PATH, &cbData);
            //RegSetValueExW(hkApp, REGVAL_BALLOONICONPATH, 0, REG_SZ, (PBYTE)pCfgData->szBalloonIconPath, (DWORD)(cbData+1)*sizeof(WCHAR));
            RegSetBinValue(hkApp, REGVAL_INFOSOUND, &pCfgData->bInfoSound, sizeof(BOOL));

            RegSetBinValue(hkApp, REGVAL_LASTFLOATPOS, &pCfgData->ptLastFloatPos, sizeof(POINT));
        }
        else
        {
            // TODO: 错误处理
        }
    }

    // 关闭注册表
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    if (hkSoftware != NULL)
    {
        RegCloseKey(hkSoftware);
    }
    return dwErrorCode;
}

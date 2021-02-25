#include "framework.h"

#include "config.h"

// TODO: 设置项注册表名称
static PCWSTR const REGVAL_SHOWMAINWND              = L"IsFloatWnd";
static PCWSTR const REGVAL_AUTORUN                  = L"IsAutoRun";
static PCWSTR const REGVAL_TIMEAlARM                = L"IsTimeAlarm";
//static PCWSTR const REGVAL_BALLOONICONPATH          = L"BalloonIconPath";
static PCWSTR const REGVAL_INFOSOUND                = L"IsInfoSound";

static PCWSTR const REGVAL_TEXTFONT                 = L"TextFont";
static PCWSTR const REGVAL_TEXTCOLOR                = L"TextColor";


DWORD LoadDefaultConfig(PCFGDATA pCfgData)
{
    // TODO: 应用默认设置
    pCfgData->bFloatWnd = FALSE;
    pCfgData->bAutoRun = FALSE;
    pCfgData->bTimeAlarm = TRUE;

    WCHAR szExeFullDir[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, szExeFullDir, MAX_PATH);
    PathCchRemoveFileSpec(szExeFullDir, MAX_PATH);
    //PathCchCombine(pCfgData->szBalloonIconPath, MAX_PATH, szExeFullDir, L"data\\default_balloonicon.ico");

    pCfgData->bInfoSound = TRUE;
    GetSystemCapitalFont(&pCfgData->lfText);
    pCfgData->rgbTextColor = RGB(255, 255, 255);
    return 0;
}

DWORD LoadConfigFromReg(PCFGDATA pCfgData)
{
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
            RegQueryBinValue(hkApp, REGVAL_SHOWMAINWND, &pCfgData->bFloatWnd, &cbData);
            cbData = sizeof(BOOL);
            RegQueryBinValue(hkApp, REGVAL_AUTORUN, &pCfgData->bAutoRun, &cbData);
            cbData = sizeof(BOOL);
            RegQueryBinValue(hkApp, REGVAL_TIMEAlARM, &pCfgData->bTimeAlarm, &cbData);
            //cbData = MAX_PATH;
            //RegQueryValueExW(hkApp, REGVAL_BALLOONICONPATH, 0, NULL, (PBYTE)pCfgData->szBalloonIconPath, &cbData);
            cbData = sizeof(BOOL);
            RegQueryBinValue(hkApp, REGVAL_INFOSOUND, &pCfgData->bInfoSound, &cbData);

            cbData = sizeof(LOGFONTW);
            RegQueryBinValue(hkApp, REGVAL_TEXTFONT, &pCfgData->lfText, &cbData);
            cbData = sizeof(COLORREF);
            RegQueryBinValue(hkApp, REGVAL_TEXTCOLOR, &pCfgData->rgbTextColor, &cbData);
        }
        else if (dwErrorCode == ERROR_FILE_NOT_FOUND)
        {
            dwErrorCode = LoadDefaultConfig(pCfgData);
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

            RegSetBinValue(hkApp, REGVAL_TEXTFONT, &pCfgData->lfText, sizeof(LOGFONTW));
            RegSetBinValue(hkApp, REGVAL_TEXTCOLOR, &pCfgData->rgbTextColor, sizeof(COLORREF));
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

DWORD SetAppAutoRun()
{
    DWORD dwErrorCode = ERROR_SUCCESS;
    HKEY hkRun = NULL;

    // 打开启动项  
    dwErrorCode = RegOpenRunKey(&hkRun);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        // 得到本程序自身的全路径
        WCHAR szExeFullPath[MAX_PATH];
        GetModuleFileNameW(NULL, szExeFullPath, MAX_PATH);
        PathQuoteSpacesW(szExeFullPath); // 补上双引号

        // 得到要写入的数据大小
        size_t cchPath = 0;
        StringCchLengthW(szExeFullPath, MAX_PATH, &cchPath);

        dwErrorCode = RegSetValueExW(
            hkRun, APPNAME, 0, REG_SZ, 
            (LPBYTE)szExeFullPath, (DWORD)(sizeof(WCHAR)*(cchPath+1))
        );
    }

    if (hkRun != NULL)
    {
        RegCloseKey(hkRun);
    }
    return dwErrorCode;
}

DWORD UnsetAppAutoRun()
{
    DWORD dwErrorCode = ERROR_SUCCESS;
    HKEY hkRun = NULL;

    // 打开启动项  
    dwErrorCode = RegOpenRunKey(&hkRun);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        dwErrorCode = RegDeleteValueW(hkRun, APPNAME);

    }
    if (hkRun != NULL)
    {
        RegCloseKey(hkRun);
    }
    return dwErrorCode;
}

DWORD GetSystemCapitalFont(PLOGFONTW pLogFont)
{
    NONCLIENTMETRICSW ncMetrics = { 0 };
    ncMetrics.cbSize = sizeof(NONCLIENTMETRICSW);

    // 获取当前系统标题栏的字体
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncMetrics, 0);
    CopyMemory(pLogFont, &ncMetrics.lfCaptionFont, sizeof(LOGFONTW));
    return 0;
}
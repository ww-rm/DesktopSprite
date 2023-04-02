#include <ds/framework.h>
#include <ds/utils.h>
#include <json/json.h>

#include <ds/config.h>

static PCWSTR const CFGKEY_FLOATWND                 = L"IsFloatWnd";
static PCWSTR const CFGKEY_AUTORUN                  = L"IsAutoRun";
static PCWSTR const CFGKEY_TIMEAlARM                = L"IsTimeAlarm";
static PCWSTR const CFGKEY_BALLOONICONPATH          = L"BalloonIconPath";
static PCWSTR const CFGKEY_INFOSOUND                = L"IsInfoSound";
static PCWSTR const CFGKEY_DARKTHEME                = L"IsDarkTheme";
static PCWSTR const CFGKEY_TRANSPARENCY             = L"Transparency";
static PCWSTR const CFGKEY_SHOWCONTENT              = L"ShowContent";

void AppConfig::Get(PCFGDATA pcfgdata) const
{
    pcfgdata->bFloatWnd = this->bFloatWnd;
    pcfgdata->bAutoRun = this->bAutoRun;
    pcfgdata->bTimeAlarm = this->bTimeAlarm;
    StringCchCopyW(pcfgdata->szBalloonIconPath, MAX_PATH, this->szBalloonIconPath);
    pcfgdata->bInfoSound = this->bInfoSound;
    pcfgdata->bDarkTheme = this->bDarkTheme;
    pcfgdata->transparencyPercent = this->transparencyPercent;
    pcfgdata->byShowContent = this->byShowContent;
}

void AppConfig::Set(const PCFGDATA pcfgdata)
{
    this->bFloatWnd = pcfgdata->bFloatWnd;
    this->bAutoRun = pcfgdata->bAutoRun;
    this->bTimeAlarm = pcfgdata->bTimeAlarm;
    StringCchCopyW(this->szBalloonIconPath, MAX_PATH, pcfgdata->szBalloonIconPath);
    this->bInfoSound = pcfgdata->bInfoSound;
    this->bDarkTheme = pcfgdata->bDarkTheme;
    this->transparencyPercent = pcfgdata->transparencyPercent;
    this->byShowContent = pcfgdata->byShowContent;
}

DWORD AppConfig::LoadFromReg(PCWSTR appname)
{
    DWORD dwErrorCode = ERROR_SUCCESS;

    // 打开注册表项
    HKEY hkApp = NULL;
    DWORD dwDisposition = 0;
    DWORD cbData = 0;

    WCHAR subkey[128] = { 0 };
    StringCchPrintfW(subkey, 128, L"SOFTWARE\\%s", appname);

    dwErrorCode = RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, &dwDisposition);
    if (dwErrorCode == ERROR_SUCCESS && dwDisposition == REG_OPENED_EXISTING_KEY)
    {
        // TODO: 读取配置
        cbData = sizeof(BOOL);
        RegQueryAnyValue(hkApp, CFGKEY_FLOATWND, (PBYTE)&this->bFloatWnd, &cbData);
        cbData = sizeof(BOOL);
        RegQueryAnyValue(hkApp, CFGKEY_AUTORUN, (PBYTE)&this->bAutoRun, &cbData);
        cbData = sizeof(BOOL);
        RegQueryAnyValue(hkApp, CFGKEY_TIMEAlARM, (PBYTE)&this->bTimeAlarm, &cbData);
        //cbData = MAX_PATH;
        //RegQueryValueExW(hkApp, REGVAL_BALLOONICONPATH, 0, NULL, (PBYTE)this->szBalloonIconPath, &cbData);
        cbData = sizeof(BOOL);
        RegQueryAnyValue(hkApp, CFGKEY_INFOSOUND, (PBYTE)&this->bInfoSound, &cbData);
        cbData = sizeof(BOOL);
        RegQueryAnyValue(hkApp, CFGKEY_DARKTHEME, (PBYTE)&this->bDarkTheme, &cbData);
        cbData = sizeof(DOUBLE);
        RegQueryAnyValue(hkApp, CFGKEY_TRANSPARENCY, (PBYTE)&this->transparencyPercent, &cbData);
        cbData = sizeof(BYTE);
        RegQueryAnyValue(hkApp, CFGKEY_SHOWCONTENT, (PBYTE)&this->byShowContent, &cbData);
    }
    else
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
    }
    // 关闭注册表
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    return dwErrorCode;
}

DWORD AppConfig::SaveToReg(PCWSTR appname)
{
    DWORD dwErrorCode = ERROR_SUCCESS;

    // 打开注册表项
    HKEY hkApp = NULL;
    DWORD dwDisposition = 0;
    DWORD cbData = 0;

    WCHAR subkey[128] = { 0 };
    StringCchPrintfW(subkey, 128, L"SOFTWARE\\%s", appname);

    dwErrorCode = RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, &dwDisposition);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        // TODO: 保存应用配置
        RegSetBinValue(hkApp, CFGKEY_FLOATWND, (PBYTE)&this->bFloatWnd, sizeof(BOOL));
        RegSetBinValue(hkApp, CFGKEY_AUTORUN, (PBYTE)&this->bAutoRun, sizeof(BOOL));
        RegSetBinValue(hkApp, CFGKEY_TIMEAlARM, (PBYTE)&this->bTimeAlarm, sizeof(BOOL));
        //SIZE_T cbData = 0;
        //StringCchLengthW(this->szBalloonIconPath, MAX_PATH, &cbData);
        //RegSetValueExW(hkApp, REGVAL_BALLOONICONPATH, 0, REG_SZ, (PBYTE)this->szBalloonIconPath, (DWORD)(cbData+1)*sizeof(WCHAR));
        RegSetBinValue(hkApp, CFGKEY_INFOSOUND, (PBYTE)&this->bInfoSound, sizeof(BOOL));
        RegSetBinValue(hkApp, CFGKEY_DARKTHEME, (PBYTE)&this->bDarkTheme, sizeof(BOOL));
        RegSetBinValue(hkApp, CFGKEY_TRANSPARENCY, (PBYTE)&this->transparencyPercent, sizeof(DOUBLE));
        RegSetBinValue(hkApp, CFGKEY_SHOWCONTENT, (PBYTE)&this->byShowContent, sizeof(BYTE));
    }
    else
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
    }

    // 关闭注册表
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    return dwErrorCode;
}

DWORD AppConfig::LoadFromFile(PCWSTR path)
{
    Json::Value root;

    std::ifstream file(path);
    file >> root;
    file.close();

    this->bFloatWnd = (BOOL)root.get(CFGKEY_FLOATWND, (bool)this->bFloatWnd).asBool();
    this->bAutoRun = (BOOL)root.get(CFGKEY_AUTORUN, (bool)this->bAutoRun).asBool();
    this->bTimeAlarm = (BOOL)root.get(CFGKEY_TIMEAlARM, (bool)this->bTimeAlarm).asBool();
    StrAtoW(root.get(CFGKEY_BALLOONICONPATH, this->szBalloonIconPath).asCString(), this->szBalloonIconPath, MAX_PATH);
    this->bInfoSound = (BOOL)root.get(CFGKEY_INFOSOUND, (bool)this->bInfoSound).asBool();
    this->bDarkTheme = (BOOL)root.get(CFGKEY_DARKTHEME, (bool)this->bDarkTheme).asBool();
    this->transparencyPercent = (DOUBLE)root.get(CFGKEY_TRANSPARENCY, (double)this->transparencyPercent).asDouble();
    this->byShowContent = (BOOL)root.get(CFGKEY_SHOWCONTENT, (bool)this->byShowContent).asBool();

    return 0;
}

DWORD AppConfig::SaveToFile(PCWSTR path)
{
    Json::Value root;

    root[CFGKEY_FLOATWND] = (bool)this->bFloatWnd;
    root[CFGKEY_AUTORUN] = (bool)this->bAutoRun;
    root[CFGKEY_TIMEAlARM] = (bool)this->bTimeAlarm;
    root[CFGKEY_BALLOONICONPATH] = this->szBalloonIconPath;
    root[CFGKEY_INFOSOUND] = (bool)this->bInfoSound;
    root[CFGKEY_DARKTHEME] = (bool)this->bDarkTheme;
    root[CFGKEY_TRANSPARENCY] = (double)this->transparencyPercent;
    root[CFGKEY_SHOWCONTENT] = (bool)this->byShowContent;

    std::ofstream file(path);
    file << root;
    file.close();
    return 0;
}
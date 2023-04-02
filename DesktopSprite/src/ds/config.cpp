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

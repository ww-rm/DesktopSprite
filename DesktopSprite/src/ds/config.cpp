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

AppConfig::AppConfig()
{
    // 获得程序路径
    GetModuleFileNameW(NULL, this->rootDir, MAX_PATH);
    PathCchRemoveFileSpec(this->rootDir, MAX_PATH);

    // 路径默认值
    PathCchCombine(this->szBalloonIconPath, MAX_PATH, this->GetRootDir(), L"res\\image\\timealarm.ico");
}

AppConfig::AppConfig(const AppConfig& other)
{
    this->bFloatWnd = other.bFloatWnd;
    this->bAutoRun = other.bAutoRun;
    this->bTimeAlarm = other.bTimeAlarm;
    StringCchCopyW(this->szBalloonIconPath, MAX_PATH, other.szBalloonIconPath);
    this->bInfoSound = other.bInfoSound;
    this->bDarkTheme = other.bDarkTheme;
    this->transparencyPercent = other.transparencyPercent;
    this->byShowContent = other.byShowContent;
}

AppConfig& AppConfig::operator= (const AppConfig& other)
{
    this->AppConfig::AppConfig(other);
    return *this;
}

BOOL AppConfig::LoadFromFile(PCWSTR path)
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
    this->transparencyPercent = (UINT)root.get(CFGKEY_TRANSPARENCY, (UINT)this->transparencyPercent).asUInt();
    this->byShowContent = (BYTE)root.get(CFGKEY_SHOWCONTENT, (UINT)this->byShowContent).asUInt();

    return TRUE;
}

BOOL AppConfig::SaveToFile(PCWSTR path)
{
    Json::Value root;

    root[CFGKEY_FLOATWND] = (bool)this->bFloatWnd;
    root[CFGKEY_AUTORUN] = (bool)this->bAutoRun;
    root[CFGKEY_TIMEAlARM] = (bool)this->bTimeAlarm;
    root[CFGKEY_BALLOONICONPATH] = this->szBalloonIconPath;
    root[CFGKEY_INFOSOUND] = (bool)this->bInfoSound;
    root[CFGKEY_DARKTHEME] = (bool)this->bDarkTheme;
    root[CFGKEY_TRANSPARENCY] = (UINT)this->transparencyPercent;
    root[CFGKEY_SHOWCONTENT] = (UINT)this->byShowContent;

    std::ofstream file(path);
    file << root;
    file.close();

    return TRUE;
}

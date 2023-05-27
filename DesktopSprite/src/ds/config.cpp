#include <ds/framework.h>
#include <ds/winapp.h>
#include <ds/utils.h>
#include <json/json.h>

#include <ds/config.h>

namespace AppConfig {

    AppConfig::AppConfig()
    {
        // 路径默认值
        PathCchCombine(this->szBalloonIconPath, MAX_PATH, WinApp::GetDir(), L"res\\image\\timealarm.ico");
        PathCchCombine(this->szSpineAtlasPath, MAX_PATH, WinApp::GetDir(), L"res\\spine\\guanghui_2.atlas");
        PathCchCombine(this->szSpineSkelPath, MAX_PATH, WinApp::GetDir(), L"res\\spine\\guanghui_2.skel");
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
        
        // sprite 设置
        this->maxFps = other.maxFps;
        this->scale = other.scale;
        this->spTransparencyPercent = other.spTransparencyPercent;
        StringCchCopyW(this->szSpineAtlasPath, MAX_PATH, other.szSpineAtlasPath);
        StringCchCopyW(this->szSpineSkelPath, MAX_PATH, other.szSpineSkelPath);

        // spine 设置
        StringCchCopyW(this->spAnimeIdle, MAX_PATH, other.spAnimeIdle);
        StringCchCopyW(this->spAnimeDrag, MAX_PATH, other.spAnimeDrag);
        StringCchCopyW(this->spAnimeWork, MAX_PATH, other.spAnimeWork);
        StringCchCopyW(this->spAnimeSleep, MAX_PATH, other.spAnimeSleep);
        StringCchCopyW(this->spAnimeStand, MAX_PATH, other.spAnimeStand);
        StringCchCopyW(this->spAnimeTouch, MAX_PATH, other.spAnimeTouch);
        StringCchCopyW(this->spAnimeWink, MAX_PATH, other.spAnimeWink);
        StringCchCopyW(this->spAnimeVictory, MAX_PATH, other.spAnimeVictory);
        StringCchCopyW(this->spAnimeDance, MAX_PATH, other.spAnimeDance);
        StringCchCopyW(this->spAnimeDizzy, MAX_PATH, other.spAnimeDizzy);
    }

    AppConfig& AppConfig::operator= (const AppConfig& other)
    {
        this->AppConfig::AppConfig(other);
        return *this;
    }

    BOOL AppConfig::LoadFromFile(PCWSTR path)
    {
        if (!PathFileExistsW(path))
            return FALSE;

        Json::Value root;

        std::ifstream file(path);
        file >> root;
        file.close();

        this->bFloatWnd = (BOOL)root.get("IsFloatWnd", (bool)this->bFloatWnd).asBool();
        this->bAutoRun = (BOOL)root.get("IsAutoRun", (bool)this->bAutoRun).asBool();
        this->bTimeAlarm = (BOOL)root.get("IsTimeAlarm", (bool)this->bTimeAlarm).asBool();
        StrAtoW(root.get("BalloonIconPath", this->szBalloonIconPath).asCString(), this->szBalloonIconPath, MAX_PATH);
        this->bInfoSound = (BOOL)root.get("IsInfoSound", (bool)this->bInfoSound).asBool();
        this->bDarkTheme = (BOOL)root.get("IsDarkTheme", (bool)this->bDarkTheme).asBool();
        this->transparencyPercent = (UINT)root.get("Transparency", (UINT)this->transparencyPercent).asUInt();
        this->byShowContent = (BYTE)root.get("ShowContent", (UINT)this->byShowContent).asUInt();

        // sprite 设置
        Json::Value sprite = root.get("Sprite", Json::Value("{}"));

        this->maxFps = sprite.get("MaxFPS", (UINT)this->maxFps).asUInt();
        this->scale = sprite.get("Scale", (UINT)this->scale).asUInt();
        this->spTransparencyPercent = sprite.get("Transparency", (UINT)this->spTransparencyPercent).asUInt();
        StrAtoW(sprite.get("AtlasPath", this->szSpineAtlasPath).asCString(), this->szSpineAtlasPath, MAX_PATH);
        StrAtoW(sprite.get("SkelPath", this->szSpineSkelPath).asCString(), this->szSpineSkelPath, MAX_PATH);

        // spine anime settings
        Json::Value spriteAnime = sprite.get("Anime", Json::Value("{}"));
        StrAtoW(spriteAnime.get("Idle", this->spAnimeIdle).asCString(), this->spAnimeIdle, MAX_PATH);
        StrAtoW(spriteAnime.get("Drag", this->spAnimeDrag).asCString(), this->spAnimeDrag, MAX_PATH);
        StrAtoW(spriteAnime.get("Work", this->spAnimeWork).asCString(), this->spAnimeWork, MAX_PATH);
        StrAtoW(spriteAnime.get("Sleep", this->spAnimeSleep).asCString(), this->spAnimeSleep, MAX_PATH);
        StrAtoW(spriteAnime.get("Stand", this->spAnimeStand).asCString(), this->spAnimeStand, MAX_PATH);
        StrAtoW(spriteAnime.get("Touch", this->spAnimeTouch).asCString(), this->spAnimeTouch, MAX_PATH);
        StrAtoW(spriteAnime.get("Wink", this->spAnimeWink).asCString(), this->spAnimeWink, MAX_PATH);
        StrAtoW(spriteAnime.get("Victory", this->spAnimeVictory).asCString(), this->spAnimeVictory, MAX_PATH);
        StrAtoW(spriteAnime.get("Dance", this->spAnimeDance).asCString(), this->spAnimeDance, MAX_PATH);
        StrAtoW(spriteAnime.get("Dizzy", this->spAnimeDizzy).asCString(), this->spAnimeDizzy, MAX_PATH);

        return TRUE;
    }

    BOOL AppConfig::SaveToFile(PCWSTR path)
    {
        // spine anime settings
        Json::Value spriteAnime;
        spriteAnime["Idle"] = this->spAnimeIdle;
        spriteAnime["Drag"] = this->spAnimeDrag;
        spriteAnime["Work"] = this->spAnimeWork;
        spriteAnime["Sleep"] = this->spAnimeSleep;
        spriteAnime["Stand"] = this->spAnimeStand;
        spriteAnime["Touch"] = this->spAnimeTouch;
        spriteAnime["Wink"] = this->spAnimeWink;
        spriteAnime["Victory"] = this->spAnimeVictory;
        spriteAnime["Dance"] = this->spAnimeDance;
        spriteAnime["Dizzy"] = this->spAnimeDizzy;

        // sprite settings
        Json::Value sprite;
        sprite["Anime"] = spriteAnime;

        sprite["MaxFPS"] = (UINT)this->maxFps;
        sprite["Scale"] = (UINT)this->scale;
        sprite["Transparency"] = (UINT)this->spTransparencyPercent;
        sprite["AtlasPath"] = this->szSpineAtlasPath;
        sprite["SkelPath"] = this->szSpineSkelPath;


        Json::Value root;
        root["Sprite"] = sprite;

        root["IsFloatWnd"] = (bool)this->bFloatWnd;
        root["IsAutoRun"] = (bool)this->bAutoRun;
        root["IsTimeAlarm"] = (bool)this->bTimeAlarm;
        root["BalloonIconPath"] = this->szBalloonIconPath;
        root["IsInfoSound"] = (bool)this->bInfoSound;
        root["IsDarkTheme"] = (bool)this->bDarkTheme;
        root["Transparency"] = (UINT)this->transparencyPercent;
        root["ShowContent"] = (UINT)this->byShowContent;

        std::ofstream file(path);
        file << root;
        file.close();

        return TRUE;
    }

    // global functions
    static AppConfig* g_appConfig = NULL;

    BOOL Init()
    {
        if (!g_appConfig)
        {
            g_appConfig = new AppConfig();
        }
        return TRUE;
    }

    BOOL Uninit()
    {
        if (g_appConfig)
        {
            delete g_appConfig;
            g_appConfig = NULL;
        }
        return TRUE;
    }

    inline const AppConfig& Get()
    {
        return *g_appConfig;
    }

    inline void Get(AppConfig* other)
    {
        *other = *g_appConfig;
    }

    inline void Set(const AppConfig* other)
    {
        *g_appConfig = *other;
    }

    inline BOOL LoadFromFile(PCWSTR path)
    {
        return g_appConfig->LoadFromFile(path);
    }

    inline BOOL SaveToFile(PCWSTR path)
    {
        return g_appConfig->SaveToFile(path);
    }
}
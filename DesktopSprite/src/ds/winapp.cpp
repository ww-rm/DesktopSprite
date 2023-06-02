#include <ds/framework.h>
#include <ds/utils.h>

#include <ds/spritewnd.h>
#include <ds/mainwnd.h>
#include <ds/winapp.h>

namespace WinApp {

    WinApp::WinApp()
    {
        // 获得程序路径
        GetModuleFileNameW(NULL, this->szExeFullDir, MAX_PATH);
        PathCchRemoveFileSpec(this->szExeFullDir, MAX_PATH);
        GetModuleFileNameW(NULL, this->szExeFullPath, MAX_PATH);

        // 获取 %APPDATA% 路径
        WCHAR appDataPath[MAX_PATH] = { 0 };
        SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath);

        // 获取程序的用户数据文件夹
        PathCchCombine(this->szAppDataDir, MAX_PATH, appDataPath, this->GetName());
        if (!CreateDirectoryW(this->szAppDataDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            MessageBoxW(NULL, L"数据文件夹创建失败，将无法保存配置信息！", L"错误信息", MB_ICONINFORMATION);
        }

        // 获得配置路径, 这里要放在用户数据文件夹下面, 否则 C 盘有写入权限问题
        PathCchCombine(this->szConfigFullPath, MAX_PATH, this->szAppDataDir, L"config.json");
    }

    BOOL WinApp::Initialize()
    {
#ifdef _DEBUG
        this->hAppMutex = CreateMutexW(NULL, FALSE, L"DesktopSpriteMutex_d");
#else
        this->hAppMutex = CreateMutexW(NULL, FALSE, L"DesktopSpriteMutex");
#endif // _DEBUG

        if (!this->hAppMutex)
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            exit(EXIT_FAILURE);
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            MessageBoxW(NULL, L"程序已在运行, 请勿重复启动", L"提示消息", MB_OK | MB_ICONINFORMATION);
            exit(EXIT_SUCCESS);
        }

        // 初始化 CommonControls
        INITCOMMONCONTROLSEX cce = { sizeof(cce), 0xffff };
        if (!InitCommonControlsEx(&cce))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            exit(EXIT_FAILURE);
        }

        // 初始化, 这里不用 CoInitializeEx, 其他通用对话框会死锁 (不知道啥原因)
        if (FAILED(CoInitialize(NULL)))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            exit(EXIT_FAILURE);
        }

        // 初始化 GDI+.
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        if (Gdiplus::Status::Ok != Gdiplus::GdiplusStartup(&this->gdiplusToken, &gdiplusStartupInput, NULL))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            exit(EXIT_FAILURE);
        }

        // 读取配置, 不能放构造函数里, 因为全局的 WinApp 指针还没初始化, 不能调用全局接口
        AppConfig::Init();
        AppConfig::LoadFromFile(this->GetConfigPath());

        // 启动性能监视器
        PerfMonitor::Init();

        return TRUE;
    }

    BOOL WinApp::Uninitialize()
    {
        PerfMonitor::Uninit();
        AppConfig::SaveToFile(this->GetConfigPath());
        Gdiplus::GdiplusShutdown(this->gdiplusToken);
        CoUninitialize();
        CloseHandle(this->hAppMutex);
        return TRUE;
    }

    INT WinApp::Mainloop()
    {
        INT errCode = EXIT_SUCCESS;

        MainWindow* mainWindow = new MainWindow();
        mainWindow->CreateWindow_();

        BOOL bRet;
        MSG msg;
        // 开启消息循环
        while ((bRet = GetMessageW(&msg, NULL, 0, 0)))
        {
            if (bRet == -1)
            {
                errCode = EXIT_FAILURE;
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }

        delete mainWindow;
        return errCode;
    }

    PCWSTR WinApp::GetName() const
    {
#ifdef _DEBUG
        return L"DesktopSprite_d";
#else
        return L"DesktopSprite";
#endif // _DEBUG
    };

    PCWSTR WinApp::GetPath() const
    {
        return this->szExeFullPath;
    }

    PCWSTR WinApp::GetDir() const
    {
        return this->szExeFullDir;
    }

    PCWSTR WinApp::GetAppDataDir() const
    {
        return this->szAppDataDir;
    }

    PCWSTR WinApp::GetConfigPath() const
    {
        return this->szConfigFullPath;
    }

    // global functions

    static WinApp* g_winApp = NULL;

    BOOL Init()
    {
        if (!g_winApp)
        {
            g_winApp = new WinApp();
            g_winApp->Initialize();
        }
        return TRUE;
    }

    BOOL Uninit()
    {
        if (g_winApp)
        {
            g_winApp->Uninitialize();
            delete g_winApp;
            g_winApp = NULL;
        }
        return TRUE;
    }

    INT Mainloop()
    {
        return g_winApp->Mainloop();
    }

    PCWSTR GetName()
    {
        if (!g_winApp) return NULL;
        return g_winApp->GetName();
    }

    PCWSTR GetPath()
    {
        if (!g_winApp) return NULL;
        return g_winApp->GetPath();
    }

    PCWSTR GetDir()
    {
        if (!g_winApp) return NULL;
        return g_winApp->GetDir();
    }

    PCWSTR GetAppDataDir()
    {
        if (!g_winApp) return NULL;
        return g_winApp->GetAppDataDir();
    }

    PCWSTR GetConfigPath()
    {
        if (!g_winApp) return NULL;
        return g_winApp->GetConfigPath();
    }
}

INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nShowCmd)
{
    INT errCode = EXIT_SUCCESS;
    WinApp::Init();
    errCode = WinApp::Mainloop();
    WinApp::Uninit();
    return errCode;
}

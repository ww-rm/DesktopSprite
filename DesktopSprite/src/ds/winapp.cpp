#include <ds/framework.h>
#include <ds/utils.h>

#include <ds/spritewnd.h>
#include <ds/mainwnd.h>
#include <ds/winapp.h>

namespace WinApp {

    WinApp::WinApp()
    {
        // ��ó���·��
        GetModuleFileNameW(NULL, this->szExeFullDir, MAX_PATH);
        PathCchRemoveFileSpec(this->szExeFullDir, MAX_PATH);
        GetModuleFileNameW(NULL, this->szExeFullPath, MAX_PATH);

        // ��ȡ %APPDATA% ·��
        WCHAR appDataPath[MAX_PATH] = { 0 };
        SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath);

        // ��ȡ������û������ļ���
        PathCchCombine(this->szAppDataDir, MAX_PATH, appDataPath, this->GetName());
        if (!CreateDirectoryW(this->szAppDataDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            MessageBoxW(NULL, L"�����ļ��д���ʧ�ܣ����޷�����������Ϣ��", L"������Ϣ", MB_ICONINFORMATION);
        }

        // �������·��, ����Ҫ�����û������ļ�������, ���� C ����д��Ȩ������
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
            MessageBoxW(NULL, L"������������, �����ظ�����", L"��ʾ��Ϣ", MB_OK | MB_ICONINFORMATION);
            exit(EXIT_SUCCESS);
        }

        // ��ʼ�� CommonControls
        INITCOMMONCONTROLSEX cce = { sizeof(cce), 0xffff };
        if (!InitCommonControlsEx(&cce))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            exit(EXIT_FAILURE);
        }

        // ��ʼ��, ���ﲻ�� CoInitializeEx, ����ͨ�öԻ�������� (��֪��ɶԭ��)
        if (FAILED(CoInitialize(NULL)))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            exit(EXIT_FAILURE);
        }

        // ��ʼ�� GDI+.
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        if (Gdiplus::Status::Ok != Gdiplus::GdiplusStartup(&this->gdiplusToken, &gdiplusStartupInput, NULL))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            exit(EXIT_FAILURE);
        }

        // ��ȡ����, ���ܷŹ��캯����, ��Ϊȫ�ֵ� WinApp ָ�뻹û��ʼ��, ���ܵ���ȫ�ֽӿ�
        AppConfig::Init();
        AppConfig::LoadFromFile(this->GetConfigPath());

        // �������ܼ�����
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
        // ������Ϣѭ��
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

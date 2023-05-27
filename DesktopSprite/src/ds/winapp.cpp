#include <ds/framework.h>
#include <ds/utils.h>

#include <ds/mainwnd.h>
#include <ds/spritewnd.h>
#include <ds/winapp.h>

namespace WinApp {

    WinApp::WinApp()
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

        // ��ó���·��
        GetModuleFileNameW(NULL, this->szExeFullDir, MAX_PATH);
        PathCchRemoveFileSpec(this->szExeFullDir, MAX_PATH);
        GetModuleFileNameW(NULL, this->szExeFullPath, MAX_PATH);

        // �������·��
        PathCchCombine(this->szConfigFullPath, MAX_PATH, this->szExeFullDir, L"config.json");

        // ��ȡ����
        AppConfig::Init();
        AppConfig::LoadFromFile(this->GetConfigPath());

        // �������ܼ�����
        PerfMonitor::Init();
    }

    WinApp::~WinApp()
    {
        PerfMonitor::Uninit();
        AppConfig::SaveToFile(this->GetConfigPath());
        GdiplusShutdown(this->gdiplusToken);
        CoUninitialize();
        CloseHandle(this->hAppMutex);
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
        }
        return TRUE;
    }

    BOOL Uninit()
    {
        if (g_winApp)
        {
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

#include <ds/framework.h>
#include <ds/utils.h>

#include <ds/mainwnd.h>
#include <ds/spritewnd.h>
#include <ds/winapp.h>

WinApp::WinApp()
{
#ifdef _DEBUG
    this->hAppMutex = CreateMutexW(NULL, FALSE, L"DesktopSpriteMutex_D");
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
    INITCOMMONCONTROLSEX cce = { sizeof(cce), 0xffff };

    // 初始化 CommonControls
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

    // 获得程序路径
    GetModuleFileNameW(NULL, this->szExeFullDir, MAX_PATH);
    PathCchRemoveFileSpec(this->szExeFullDir, MAX_PATH);
    GetModuleFileNameW(NULL, this->szExeFullPath, MAX_PATH);
}

WinApp::~WinApp()
{
    GdiplusShutdown(this->gdiplusToken);
    CoUninitialize();
    CloseHandle(this->hAppMutex);
}

INT WinApp::Mainloop()
{
    INT errCode = EXIT_SUCCESS;

    MainWindow* mainWindow = new MainWindow(this);
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

INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nShowCmd)
{
    INT errCode = EXIT_SUCCESS;
    WinApp* app = new WinApp();
    errCode = app->Mainloop();
    delete app;
    return errCode;
}

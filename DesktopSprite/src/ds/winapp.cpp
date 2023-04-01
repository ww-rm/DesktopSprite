#include <ds/framework.h>
#include <ds/utils.h>

#include <ds/mainwnd.h>
#include <ds/winapp.h>
#include <ds/spritewnd.h>

WinApp::WinApp()
{
    this->hAppMutex = CreateMutexW(NULL, FALSE, L"DesktopSpriteMutexDEBUG");
    if (!this->hAppMutex)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxW(NULL, L"程序已在运行, 请勿重复启动", L"提示消息", MB_OK | MB_ICONWARNING);
        exit(EXIT_SUCCESS);
    }

    // 初始化 GDI+.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::Status status = Gdiplus::GdiplusStartup(&this->gdiplusToken, &gdiplusStartupInput, NULL);
    if (status != Gdiplus::Status::Ok)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // 获得程序路径
    GetModuleFileNameW(NULL, this->szExeFullDir, MAX_PATH);
    PathCchRemoveFileSpec(this->szExeFullDir, MAX_PATH);
    GetModuleFileNameW(NULL, this->szExeFullPath, MAX_PATH);

    // 修改当前目录
    if (!SetCurrentDirectoryW(this->GetAppDir()))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }
}

WinApp::~WinApp()
{
    GdiplusShutdown(this->gdiplusToken);
    CloseHandle(this->hAppMutex);
}

INT WinApp::Mainloop()
{
    INT errCode = EXIT_SUCCESS;

    MainWindow* mainWindow = new MainWindow(this);
    HWND h = CreateSpriteWnd(GetModuleHandleW(NULL), NULL);
    ShowWindow(h, SW_SHOWNA);

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

PCWSTR WinApp::GetAppName() const
{
    return L"DesktopSprite";
}

PCWSTR WinApp::GetAppPath() const
{
    return this->szExeFullPath;
}

PCWSTR WinApp::GetAppDir() const
{
    return this->szExeFullDir;
}

INT APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd)
{
    INT errCode = EXIT_SUCCESS;
    WinApp* app = new WinApp();
    errCode = app->Mainloop();
    delete app;
    return errCode;
}
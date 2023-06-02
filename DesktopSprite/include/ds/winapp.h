#pragma once
#ifndef DS_WINAPP_H
#define DS_WINAPP_H

#include <ds/framework.h>

namespace WinApp {

    class WinApp
    {
    private:
        HANDLE hAppMutex = NULL;
        ULONG_PTR gdiplusToken = 0;
        WCHAR szExeFullDir[MAX_PATH] = { 0 };
        WCHAR szExeFullPath[MAX_PATH] = { 0 };
        WCHAR szAppDataDir[MAX_PATH] = { 0 };  // %APPDATA%/WinApp::GetName()
        WCHAR szConfigFullPath[MAX_PATH] = { 0 };

    public:
        WinApp();

        BOOL Initialize();
        BOOL Uninitialize();

        INT Mainloop();

        PCWSTR GetName() const;
        PCWSTR GetPath() const;
        PCWSTR GetDir() const;
        PCWSTR GetAppDataDir() const;
        PCWSTR GetConfigPath() const;
    };

    // global functions

    BOOL Init();
    BOOL Uninit();

    PCWSTR GetName();
    PCWSTR GetPath();
    PCWSTR GetDir();
    PCWSTR GetAppDataDir();
    PCWSTR GetConfigPath();
}

#endif // !DS_WINAPP_H

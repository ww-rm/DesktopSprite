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
        WCHAR szConfigFullPath[MAX_PATH] = { 0 };

    public:
        WinApp();
        ~WinApp();
        INT Mainloop();

    public:
        PCWSTR GetName() const;
        PCWSTR GetPath() const;
        PCWSTR GetDir() const;
        PCWSTR GetConfigPath() const;
    };

    // global functions

    BOOL InitializeWinApp();
    BOOL UnInitializeWinApp();

    PCWSTR GetName();
    PCWSTR GetPath();
    PCWSTR GetDir();
    PCWSTR GetConfigPath();
}

#endif // !DS_WINAPP_H

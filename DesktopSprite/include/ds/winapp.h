#pragma once
#ifndef DS_WINAPP_H
#define DS_WINAPP_H

#include <ds/framework.h>

class WinApp
{
private:
    HANDLE hAppMutex = NULL;
    ULONG_PTR gdiplusToken = 0;
    WCHAR szExeFullDir[MAX_PATH] = { 0 };
    WCHAR szExeFullPath[MAX_PATH] = { 0 };

public:
    WinApp();
    ~WinApp();
    INT Mainloop();

public:
#ifdef _DEBUG
    PCWSTR GetAppName() const { return L"DesktopSprite_d"; };
#else
    PCWSTR GetAppName() const { return L"DesktopSprite"; };
#endif // _DEBUG
    PCWSTR GetAppPath() const { return this->szExeFullPath; }
    PCWSTR GetAppDir() const { return this->szExeFullDir; }
};

#endif // !DS_WINAPP_H

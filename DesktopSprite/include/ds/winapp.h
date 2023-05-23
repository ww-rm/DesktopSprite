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
    WCHAR szConfigFullPath[MAX_PATH] = { 0 };

public:
    WinApp();
    ~WinApp();
    INT Mainloop();

public:
#ifdef _DEBUG
    PCWSTR GetName() const { return L"DesktopSprite_d"; };
#else
    PCWSTR GetName() const { return L"DesktopSprite"; };
#endif // _DEBUG
    PCWSTR GetPath() const { return this->szExeFullPath; }
    PCWSTR GetDir() const { return this->szExeFullDir; }
    PCWSTR GetConfigPath() const { return this->szConfigFullPath; }
};

extern WinApp* g_winApp;

#endif // !DS_WINAPP_H

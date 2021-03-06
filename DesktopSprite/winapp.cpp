#include "framework.h"
#include "mainwnd.h"
#include "perfdata.h"

#include "winapp.h"

using namespace Gdiplus;

static  PCWSTR  const       APPMUTEXNAME    = L"DesktopSpriteMutex";    // 防止重复启动的 MutexName
static  HANDLE              hAppMutex       = NULL;                     // 防止重复启动的互斥锁

INT APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow)
{
    hAppMutex = CreateMutexW(NULL, FALSE, APPMUTEXNAME);
    if (hAppMutex != NULL)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            // 初始工作
            RegisterMainWnd(hInstance);
            OpenPerfMonitor();

            // 初始化 GDI+.
            GdiplusStartupInput gdiplusStartupInput;
            ULONG_PTR           gdiplusToken;
            GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

            HWND hMainWnd = CreateMainWnd(hInstance);
            if (hMainWnd != NULL)
            {
                BOOL bRet;
                MSG msg;
                // 开启消息循环
                while ((bRet = GetMessageW(&msg, NULL, 0, 0)))
                {
                    if (bRet == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        TranslateMessage(&msg);
                        DispatchMessageW(&msg);
                    }
                }
            }

            // 清理工作
            GdiplusShutdown(gdiplusToken);
            ClosePerfMonitor();
        }
        else
        {
            // TODO: 重复启动处理
        }
    }
    else
    {
        // TODO: 错误处理
    }
    return EXIT_SUCCESS;
}
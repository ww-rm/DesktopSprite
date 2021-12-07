#include <ds/framework.h>
#include <ds/perfdata.h>

#include <ds/mainwnd.h>
#include <ds/winapp.h>

using namespace Gdiplus;

static  PCWSTR  const       APPMUTEXNAME    = L"DesktopSpriteMutex";    // ��ֹ�ظ������� MutexName
static  HANDLE              hAppMutex       = NULL;                     // ��ֹ�ظ������Ļ�����

INT APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ INT nShowCmd
)
{
    hAppMutex = CreateMutexW(NULL, FALSE, APPMUTEXNAME);
    if (hAppMutex != NULL)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            // ��ʼ����
            RegisterMainWnd(hInstance);
            OpenPerfMonitor();

            // ��ʼ�� GDI+.
            GdiplusStartupInput gdiplusStartupInput;
            ULONG_PTR           gdiplusToken;
            GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

            HWND hMainWnd = CreateMainWnd(hInstance);
            if (hMainWnd != NULL)
            {
                BOOL bRet;
                MSG msg;
                // ������Ϣѭ��
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

            // ������
            GdiplusShutdown(gdiplusToken);
            ClosePerfMonitor();
        }
        else
        {
            // TODO: �ظ���������
        }
    }
    else
    {
        // TODO: ������
    }
    return EXIT_SUCCESS;
}
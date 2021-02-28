#include "framework.h"
#include "mainwnd.h"
#include "perfdata.h"
#include "util.h"

#include "winapp.h"

static  PCWSTR  const       TTFRESTMPPATH   = L"%TEMP%\\tmp_ds.ttf";
static  PCWSTR  const       APPMUTEXNAME    = L"DesktopSpriteMutex";    // ��ֹ�ظ������� MutexName
static  HANDLE              hAppMutex       = NULL;                     // ��ֹ�ظ������Ļ�����

INT APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow)
{
    hAppMutex = CreateMutexW(NULL, FALSE, APPMUTEXNAME);
    if (hAppMutex != NULL)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            // ��ʼ����
            RegisterMainWnd(hInstance);
            OpenPerfMonitor();
            
            // �����Ҫ��˽��������Դ
            WCHAR szFullTTFResPath[MAX_PATH] = { 0 };
            ExpandEnvironmentStringsW(TTFRESTMPPATH, szFullTTFResPath, MAX_PATH);
            ExtractResTTF(IDR_TTF1, szFullTTFResPath);
            AddFontResourceExW(szFullTTFResPath, FR_PRIVATE, 0);

            HWND hMainWnd = CreateMainWnd(hInstance);

            // ������Ϣѭ��
            BOOL bRet;
            MSG msg;
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

            // ������
            RemoveFontResourceExW(szFullTTFResPath, FR_PRIVATE, 0);
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
#include "framework.h"
#include "mainwnd.h"
#include "perfdata.h"

#include "winapp.h"

static  HANDLE  hAppMutex   =   NULL;       // ��ֹ�ظ������Ļ�����

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
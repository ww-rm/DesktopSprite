#pragma once
#include "framework.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAINWNDCLASSNAME                L"DesktopSpriteMainWndClass"

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define WM_NOTIFYICON                   (WM_USER + 1)           // ֪ͨ������Ϣ
#define WM_TIMEALARM                    (WM_USER + 2)           // ��ʱ��Ϣ

    // ��������
    typedef struct _MAINWNDDATA
    {
        HWND            hWnd;

        // ����ʱ������
        BOOL            bFloatWnd;                    // �Ƿ����渡��
        HFONT           hTextFont;                     // ��ʾ�ı�������
        COLORREF        rgbText;             // ��ʾ�ı�����ɫ
        //WCHAR           szBalloonIconPath[MAX_PATH];                    // ����ͼ���ļ�·��
        BOOL            bInfoSound;                    // ������ʾ�Ƿ�������

        // ����ʱ����
        BOOL            bWndFixed;                    // �����Ƿ�ͨ��ͼ����������ʾ
        POINT           ptDragSrc;                    // �϶�����ʱ��Դ��
    }*PMAINWNDDATA, MAINWNDDATA;

    // ע��������
    ATOM RegisterMainWnd(HINSTANCE hInstance);

    // �����ڴ�������
    HWND CreateMainWnd(HINSTANCE hInstance);

    // Ӧ��������
    //DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

#ifdef __cplusplus
}
#endif // __cplusplus
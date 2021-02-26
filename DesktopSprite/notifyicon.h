#pragma once
#include "framework.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAX_NIDTIP                      64                      // ֪ͨ�����Tip��Ϣ��󳤶�
#define MAX_NIDINFOTITLE                64                      //
#define MAX_NIDINFO                     256                     //

    // ���֪ͨ����ͼ��
    DWORD AddNotifyIcon(HWND hWnd, UINT uID, UINT uCallbackMessage, HICON hIcon);

    // ����֪ͨ����ͼ��
    DWORD SetNotifyIcon(HWND hWnd, UINT uID, HICON hIcon);

    // ����ͼ��Tip��Ϣ
    DWORD SetNotifyIconTip(HWND hWnd, UINT uID, PCWSTR szTip);

    // ����������Ϣ
    DWORD SetNotifyIconInfo(HWND hWnd, UINT uID, PCWSTR szInfoTitle, PCWSTR szInfo, HICON hBalloonIcon, BOOL bSound);

    // NIM_SETFOCUS
    DWORD SetFocusNotifyIcon(HWND hWnd, UINT uID);

    // ɾ��ͼ��
    DWORD DeleteNotifyIcon(HWND hWnd, UINT uID);

    // ���ͼ����η�Χ
    DWORD GetNotifyIconRect(HWND hWnd, UINT uID, PRECT pRect);

#ifdef __cplusplus
}
#endif // __cplusplus
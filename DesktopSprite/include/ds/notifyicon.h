#pragma once
#ifndef DS_NOTIFYICON_H
#define DS_NOTIFYICON_H

#include <ds/framework.h>

#define MAX_NIDTIP                      64                      // ֪ͨ�����Tip��Ϣ��󳤶�
#define MAX_NIDINFOTITLE                64                      //
#define MAX_NIDINFO                     256                     //

class NotifyIcon
{
private:
    HWND hWnd = NULL;
    UINT uID = 0;

public:
    NotifyIcon(HWND hWnd, UINT uID, UINT uCallbackMessage, HICON hIcon);
    ~NotifyIcon();

    // ����֪ͨ����ͼ��
    DWORD SetIcon(HICON hIcon);

    // ����ͼ�� Tip ��Ϣ
    DWORD SetTip(PCWSTR szTip);

    // ����������Ϣ, �� hBalloonIcon ��Чʱ, ϵͳʹ�ú�֪ͨ��һ����ͼ����Ϊ����ͼ��
    DWORD PopIconInfo(PCWSTR szInfoTitle, PCWSTR szInfo, HICON hBalloonIcon, BOOL bSound);

    // NIM_SETFOCUS
    DWORD SetFocus();

    // ���ͼ����η�Χ
    DWORD GetRect(PRECT pRect);
};

#endif // !DS_NOTIFYICON_H

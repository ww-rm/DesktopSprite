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
    NotifyIcon(HWND hWnd, UINT uID) : hWnd(hWnd), uID(uID) {}

    // ���ͼ��, �ظ���ӷ��� FALSE
    BOOL Add(UINT uCallbackMessage = 0, HICON hIcon = NULL, PCWSTR szTip = NULL);

    // ɾ��ͼ��
    BOOL Delete();

    // �޸Ļص���Ϣ
    BOOL ModifyCallbackMessage(UINT uCallbackMessage);

    // ����֪ͨ����ͼ��
    BOOL ModifyIcon(HICON hIcon);

    // ����ͼ�� Tip ��Ϣ
    BOOL ModifyTip(PCWSTR szTip);

    // ����������Ϣ, �� hBalloonIcon ��Чʱ, ϵͳʹ�ú�֪ͨ��һ����ͼ����Ϊ����ͼ��
    BOOL PopupIconInfo(PCWSTR szInfoTitle, PCWSTR szInfo, HICON hBalloonIcon, BOOL bSound = TRUE);

    // NIM_SETFOCUS
    BOOL SetFocus();

    // ���ͼ����η�Χ
    BOOL GetRect(PRECT pRect);
};

#endif // !DS_NOTIFYICON_H

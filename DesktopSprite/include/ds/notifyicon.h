#pragma once
#ifndef DS_NOTIFYICON_H
#define DS_NOTIFYICON_H

#include <ds/framework.h>

#define MAX_NIDTIP                      64                      // 通知区域的Tip消息最大长度
#define MAX_NIDINFOTITLE                64                      //
#define MAX_NIDINFO                     256                     //

class NotifyIcon
{
private:
    HWND hWnd = NULL;
    UINT uID = 0;

public:
    NotifyIcon(HWND hWnd, UINT uID) : hWnd(hWnd), uID(uID) {}

    // 添加图标, 重复添加返回 FALSE
    BOOL Add(UINT uCallbackMessage = 0, HICON hIcon = NULL, PCWSTR szTip = NULL);

    // 删除图标
    BOOL Delete();

    // 修改回调消息
    BOOL ModifyCallbackMessage(UINT uCallbackMessage);

    // 修饰通知区域图标
    BOOL ModifyIcon(HICON hIcon);

    // 设置图标 Tip 消息
    BOOL ModifyTip(PCWSTR szTip);

    // 弹出气泡消息, 当 hBalloonIcon 无效时, 系统使用和通知栏一样的图标作为气泡图标
    BOOL PopupIconInfo(PCWSTR szInfoTitle, PCWSTR szInfo, HICON hBalloonIcon, BOOL bSound = TRUE);

    // NIM_SETFOCUS
    BOOL SetFocus();

    // 获得图标矩形范围
    BOOL GetRect(PRECT pRect);
};

#endif // !DS_NOTIFYICON_H

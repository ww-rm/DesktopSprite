#pragma once
#include "framework.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAX_NIDTIP                      64                      // 通知区域的Tip消息最大长度
#define MAX_NIDINFOTITLE                64                      //
#define MAX_NIDINFO                     256                     //

    // 添加通知区域图标
    DWORD AddNotifyIcon(HWND hWnd, UINT uID, UINT uCallbackMessage, HICON hIcon);

    // 修饰通知区域图标
    DWORD SetNotifyIcon(HWND hWnd, UINT uID, HICON hIcon);

    // 设置图标Tip消息
    DWORD SetNotifyIconTip(HWND hWnd, UINT uID, PCWSTR szTip);

    // 弹出气泡消息
    DWORD SetNotifyIconInfo(HWND hWnd, UINT uID, PCWSTR szInfoTitle, PCWSTR szInfo, HICON hBalloonIcon, BOOL bSound);

    // NIM_SETFOCUS
    DWORD SetFocusNotifyIcon(HWND hWnd, UINT uID);

    // 删除图标
    DWORD DeleteNotifyIcon(HWND hWnd, UINT uID);

    // 获得图标矩形范围
    DWORD GetNotifyIconRect(HWND hWnd, UINT uID, PRECT pRect);

#ifdef __cplusplus
}
#endif // __cplusplus
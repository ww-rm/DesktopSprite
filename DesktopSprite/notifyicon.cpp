#include "framework.h"

#include "notifyicon.h"

DWORD AddNotifyIcon(HWND hWnd, UINT uID, UINT uCallbackMessage, HICON hIcon)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uVersion = NOTIFYICON_VERSION_4;
    nid.uFlags = NIF_MESSAGE | NIF_ICON;
    nid.uCallbackMessage = uCallbackMessage;
    nid.hIcon = hIcon;

    Shell_NotifyIconW(NIM_ADD, &nid);
    Shell_NotifyIconW(NIM_SETVERSION, &nid);
    return 0;
}

DWORD SetNotifyIcon(HWND hWnd, UINT uID, HICON hIcon)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_ICON;
    nid.hIcon = hIcon;

    Shell_NotifyIconW(NIM_MODIFY, &nid);
    return 0;
}

DWORD SetNotifyIconTip(HWND hWnd, UINT uID, PCWSTR szTip)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_TIP;
    StringCchCopyW(nid.szTip, MAX_NIDTIP, szTip);

    Shell_NotifyIconW(NIM_MODIFY, &nid);
    return 0;
}

DWORD SetNotifyIconInfo(HWND hWnd, UINT uID, PCWSTR szInfoTitle, PCWSTR szInfo, HICON hBalloonIcon, BOOL bSound)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_INFO | NIF_REALTIME;
    nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON | (bSound ? 0 : NIIF_NOSOUND);
    StringCchCopyW(nid.szInfoTitle, MAX_NIDINFOTITLE, szInfoTitle);
    StringCchCopyW(nid.szInfo, MAX_NIDINFO, szInfo);
    nid.hBalloonIcon = hBalloonIcon;

    Shell_NotifyIconW(NIM_MODIFY, &nid);
    return 0;
}

DWORD SetFocusNotifyIcon(HWND hWnd, UINT uID)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWnd;
    nid.uID = uID;

    Shell_NotifyIconW(NIM_SETFOCUS, &nid);
    return 0;
}

DWORD DeleteNotifyIcon(HWND hWnd, UINT uID)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWnd;
    nid.uID = uID;

    Shell_NotifyIconW(NIM_DELETE, &nid);
    return 0;
}

DWORD GetNotifyIconRect(HWND hWnd, UINT uID, PRECT pRect)
{
    NOTIFYICONIDENTIFIER niid = { 0 };
    niid.cbSize = sizeof(NOTIFYICONIDENTIFIER);
    niid.hWnd = hWnd;
    niid.uID = uID;
    Shell_NotifyIconGetRect(&niid, pRect);
    return 0;
}
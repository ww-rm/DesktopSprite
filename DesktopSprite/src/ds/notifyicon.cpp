#include <ds/framework.h>
#include <ds/notifyicon.h>

NotifyIcon::NotifyIcon(HWND hWnd, UINT uID, UINT uCallbackMessage, HICON hIcon)
{
    this->hWnd = hWnd;
    this->uID = uID;

    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    nid.uVersion = NOTIFYICON_VERSION_4;
    nid.uFlags = NIF_MESSAGE | NIF_ICON;
    nid.uCallbackMessage = uCallbackMessage;
    nid.hIcon = hIcon;

    Shell_NotifyIconW(NIM_ADD, &nid);
    Shell_NotifyIconW(NIM_SETVERSION, &nid);
}

NotifyIcon::~NotifyIcon()
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;

    Shell_NotifyIconW(NIM_DELETE, &nid);
}

DWORD NotifyIcon::SetIcon(HICON hIcon)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    nid.uFlags = NIF_ICON;
    nid.hIcon = hIcon;

    Shell_NotifyIconW(NIM_MODIFY, &nid);
    return 0;
}

DWORD NotifyIcon::SetTip(PCWSTR szTip)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    nid.uFlags = NIF_TIP;
    StringCchCopyW(nid.szTip, MAX_NIDTIP, szTip);
    Shell_NotifyIconW(NIM_MODIFY, &nid);

    return 0;
}

DWORD NotifyIcon::PopIconInfo(PCWSTR szInfoTitle, PCWSTR szInfo, HICON hBalloonIcon, BOOL bSound)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    nid.uFlags = NIF_INFO | NIF_REALTIME;
    nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON | (bSound ? 0 : NIIF_NOSOUND);
    StringCchCopyW(nid.szInfoTitle, MAX_NIDINFOTITLE, szInfoTitle);
    StringCchCopyW(nid.szInfo, MAX_NIDINFO, szInfo);
    nid.hBalloonIcon = hBalloonIcon;

    Shell_NotifyIconW(NIM_MODIFY, &nid);
    return 0;
}

DWORD NotifyIcon::SetFocus()
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;

    Shell_NotifyIconW(NIM_SETFOCUS, &nid);
    return 0;
}

DWORD NotifyIcon::GetRect(PRECT pRect)
{
    NOTIFYICONIDENTIFIER niid = { 0 };
    niid.cbSize = sizeof(NOTIFYICONIDENTIFIER);
    niid.hWnd = this->hWnd;
    niid.uID = this->uID;
    Shell_NotifyIconGetRect(&niid, pRect);
    return 0;
}

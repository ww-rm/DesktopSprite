#include <ds/framework.h>
#include <ds/notifyicon.h>

BOOL NotifyIcon::Add(UINT uCallbackMessage, HICON hIcon, PCWSTR szTip)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    nid.uVersion = NOTIFYICON_VERSION_4;
    nid.uFlags = 0;
    if (uCallbackMessage)
    {
        nid.uFlags = nid.uFlags | NIF_MESSAGE;
        nid.uCallbackMessage = uCallbackMessage;
    }
    if (hIcon)
    {
        nid.uFlags = nid.uFlags | NIF_ICON;
        nid.hIcon = hIcon;
    }
    if (szTip)
    {
        nid.uFlags = nid.uFlags | NIF_TIP;
        StringCchCopyW(nid.szTip, MAX_NIDTIP, szTip);
    }
    return (BOOL)(Shell_NotifyIconW(NIM_ADD, &nid) && Shell_NotifyIconW(NIM_SETVERSION, &nid));
}

BOOL NotifyIcon::Delete()
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    return Shell_NotifyIconW(NIM_DELETE, &nid);
}

BOOL NotifyIcon::ModifyCallbackMessage(UINT uCallbackMessage)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    nid.uFlags = NIF_MESSAGE;
    nid.uCallbackMessage = uCallbackMessage;
    return Shell_NotifyIconW(NIM_MODIFY, &nid);
}

BOOL NotifyIcon::ModifyIcon(HICON hIcon)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    nid.uFlags = NIF_ICON;
    nid.hIcon = hIcon;
    return Shell_NotifyIconW(NIM_MODIFY, &nid);
}

BOOL NotifyIcon::ModifyTip(PCWSTR szTip)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    nid.uFlags = NIF_TIP;
    StringCchCopyW(nid.szTip, MAX_NIDTIP, szTip);
    return Shell_NotifyIconW(NIM_MODIFY, &nid);
}

BOOL NotifyIcon::PopupIconInfo(PCWSTR szInfoTitle, PCWSTR szInfo, HICON hBalloonIcon, BOOL bSound)
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
    return Shell_NotifyIconW(NIM_MODIFY, &nid);
}

BOOL NotifyIcon::SetFocus()
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = this->hWnd;
    nid.uID = this->uID;
    return Shell_NotifyIconW(NIM_SETFOCUS, &nid);
}

HRESULT NotifyIcon::GetRect(PRECT pRect)
{
    NOTIFYICONIDENTIFIER niid = { 0 };
    niid.cbSize = sizeof(NOTIFYICONIDENTIFIER);
    niid.hWnd = this->hWnd;
    niid.uID = this->uID;
    return Shell_NotifyIconGetRect(&niid, pRect);
}

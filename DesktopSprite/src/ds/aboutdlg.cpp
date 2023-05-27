#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/version.h>
#include <resource.h>

#include <ds/aboutdlg.h>

PCWSTR AboutDlg::GetTemplateName() const
{
    return MAKEINTRESOURCEW(IDD_ABOUT);
}

INT_PTR AboutDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return this->OnInitDialog(wParam, lParam);
    case WM_NOTIFY:
        return this->OnNotify(wParam, lParam);
    case WM_COMMAND:
        return this->OnCommand(wParam, lParam);
    default:
        return FALSE;
    }
}

INT_PTR AboutDlg::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    SetDlgItemTextA(this->hDlg, IDC_LABELVER, VERSION_STR);

    WCHAR projectLink[256] = { 0 };
    StringCchPrintfW(projectLink, 256, L"<a href=\"%s\">%s</a>", PROJECT_LINK, PROJECT_LINK);
    SetDlgItemTextW(this->hDlg, IDC_PROJECTLINK, projectLink);
    return TRUE;
}

INT_PTR AboutDlg::OnNotify(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR nmhdr = (LPNMHDR)lParam;
    switch (nmhdr->code)
    {
    case NM_CLICK:
    case NM_RETURN:
        LITEM item = ((PNMLINK)lParam)->item;
        ShellExecuteW(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
        return TRUE;
    default:
        return FALSE;
    }
}

INT_PTR AboutDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) notification code
    // LOWORD(wParam) control identifier
    // lParam is the HWND of control
    switch (LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
        return EndDialog(this->hDlg, TRUE);
    default:
        return FALSE;
    }
}

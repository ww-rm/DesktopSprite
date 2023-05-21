#include <ds/framework.h>
#include <ds/utils.h>

#include <ds/configdlg.h>

BOOL ConfigDlg::CheckValidFormData()
{
    // check
    if (!IsDlgButtonChecked(this->hDlg, IDC_CHECK_SHOWUSAGE) && !IsDlgButtonChecked(this->hDlg, IDC_CHECK_SHOWNETSPEED))
    {
        this->mainwnd->ShowNoConentWarningMsg();
        return FALSE;
    }

    // collect
    this->form.bAutoRun = IsDlgButtonChecked(this->hDlg, IDC_CHECK_AUTORUN);
    this->form.bTimeAlarm = IsDlgButtonChecked(this->hDlg, IDC_CHECK_TIMEALARM);
    this->form.bInfoSound = IsDlgButtonChecked(this->hDlg, IDC_CHECK_INFOSOUND);
    GetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, this->form.szBalloonIconPath, MAX_PATH);

    this->form.bFloatWnd = IsDlgButtonChecked(this->hDlg, IDC_CHECK_FLOATWND);

    if (IsDlgButtonChecked(this->hDlg, IDC_CHECK_SHOWUSAGE))
    {
        this->form.byShowContent |= SHOWCONTENT_CPUMEM;
    }
    else
    {
        this->form.byShowContent &= ~SHOWCONTENT_CPUMEM;
    }

    if (IsDlgButtonChecked(this->hDlg, IDC_CHECK_SHOWNETSPEED))
    {
        this->form.byShowContent |= SHOWCONTENT_NETSPEED;
    }
    else
    {
        this->form.byShowContent &= ~SHOWCONTENT_NETSPEED;
    }

    this->form.bDarkTheme = IsDlgButtonChecked(this->hDlg, IDC_CHECK_DARKTHEME);
    this->form.transparencyPercent = GetDlgItemInt(this->hDlg, IDC_STATIC_TRANSPARENCY, NULL, FALSE);
    return TRUE;
}

BOOL ConfigDlg::ShowBalloonIconPathSelectDlg()
{
    // 获得当前显示路径
    WCHAR ballooniconPath[MAX_PATH] = { 0 };
    GetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, ballooniconPath, MAX_PATH);

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = this->hDlg;
    ofn.lpstrFile = ballooniconPath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"图像文件 (*.jpg;*.jpeg;*.png;*.bmp;*.ico)\0*.jpg;*.jpeg;*.png;*.bmp;*.ico\0所有文件 (*.*)\0*.*\0";
    ofn.lpstrTitle = L"选择气泡图标文件";

    // OFN_NOCHANGEDIR: 文档里说对 GetOpenFileName 无效, 但其实有效
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn))
    {
        SetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, ballooniconPath);
    }

    return TRUE;
}

INT_PTR ConfigDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return this->OnInitDialog(wParam, lParam);
    case WM_COMMAND:
        return this->OnCommand(wParam, lParam);
    case WM_HSCROLL:
        return this->OnHScroll(wParam, lParam);
    default:
        return FALSE;
    }
}

INT_PTR ConfigDlg::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    CheckDlgButton(this->hDlg, IDC_CHECK_AUTORUN, this->form.bAutoRun ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_TIMEALARM, this->form.bTimeAlarm ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_INFOSOUND, this->form.bInfoSound ? BST_CHECKED : BST_UNCHECKED);
    SetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, this->form.szBalloonIconPath);

    CheckDlgButton(this->hDlg, IDC_CHECK_FLOATWND, this->form.bFloatWnd ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_SHOWUSAGE, (this->form.byShowContent & SHOWCONTENT_CPUMEM) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_SHOWNETSPEED, (this->form.byShowContent & SHOWCONTENT_NETSPEED) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_DARKTHEME, this->form.bDarkTheme ? BST_CHECKED : BST_UNCHECKED);

    SendDlgItemMessageW(this->hDlg, IDC_SLIDER_TRANSPARENCY, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100)); // 范围
    SendDlgItemMessageW(this->hDlg, IDC_SLIDER_TRANSPARENCY, TBM_SETPAGESIZE, 0, 1); // 单击一下的跳动范围
    SendDlgItemMessageW(this->hDlg, IDC_SLIDER_TRANSPARENCY, TBM_SETTICFREQ, 10, 0); // 刻度线频率
    SendDlgItemMessageW(this->hDlg, IDC_SLIDER_TRANSPARENCY, TBM_SETPOS, TRUE, this->form.transparencyPercent); // 当前位置

    SetDlgItemInt(this->hDlg, IDC_STATIC_TRANSPARENCY, this->form.transparencyPercent, FALSE);

    return TRUE;
}

INT_PTR ConfigDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) notification code
    // LOWORD(wParam) control identifier
    // lParam is the HWND of control
    switch (LOWORD(wParam))
    {
    case IDC_BTN_BALLOONICONPATH:
        this->ShowBalloonIconPathSelectDlg();
        return TRUE;
    case IDOK:
        if (this->CheckValidFormData())
        {
            return EndDialog(this->hDlg, TRUE);
        }
        return TRUE;
    case IDCANCEL:
        return EndDialog(this->hDlg, FALSE);
    case IDC_APPLY:
        if (this->CheckValidFormData())
        {
            mainwnd->ApplyConfig(&this->form);
        }
        return TRUE;
    default:
        return FALSE;
    }
}

INT_PTR ConfigDlg::OnHScroll(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case TB_LINEUP:
    case TB_LINEDOWN:
    case TB_PAGEUP:
    case TB_PAGEDOWN:
    case TB_TOP:
    case TB_BOTTOM:
    case TB_ENDTRACK:
        SetDlgItemInt(this->hDlg, IDC_STATIC_TRANSPARENCY, (UINT)SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0), FALSE);
        return TRUE;
    case TB_THUMBPOSITION:
    case TB_THUMBTRACK:
        SetDlgItemInt(this->hDlg, IDC_STATIC_TRANSPARENCY, HIWORD(wParam), FALSE);
        return TRUE;
    default:
        return FALSE;
    }
}
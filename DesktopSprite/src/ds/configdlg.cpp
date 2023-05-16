#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/version.h>

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
    BOOL bRet = FALSE;

    COMDLG_FILTERSPEC c_rgSaveTypes[2] = {
        {L"图像文件 (*.png; *.jpg; *.jpeg; *.bmp; *.ico)", L"*.png;*.jpg;*.jpeg;*.bmp;*.ico"},
        {L"All (*.*)", L"*.*"}
    };

    IShellItem* pDesktopItem = NULL;
    IFileOpenDialog* pfd = NULL;
    IShellItem* psiResult = NULL;
    PWSTR pszFilePath = NULL;

    if (SUCCEEDED(SHCreateItemInKnownFolder(FOLDERID_Desktop, 0, NULL, IID_PPV_ARGS(&pDesktopItem))) &&
        SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        pfd->SetOptions(FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_DONTADDTORECENT);
        pfd->SetFileTypes(2, c_rgSaveTypes);
        pfd->SetTitle(L"选择气泡图标文件");
        pfd->SetDefaultFolder(pDesktopItem);

        if (SUCCEEDED(pfd->Show(this->hDlg)))
        {
            pfd->GetResult(&psiResult);
            if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
            {
                SetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, pszFilePath);
                bRet = TRUE;
            }
        }
    }

    if (pszFilePath) CoTaskMemFree(pszFilePath);
    if (psiResult) psiResult->Release();
    if (pfd) pfd->Release();
    if (pDesktopItem) pDesktopItem->Release();

    return bRet;
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
    {
        //// !IMPORTANT: GetOpenFileName 有 bug 所以禁用
        //WCHAR ballooniconPath[MAX_PATH] = { 0 };
        //GetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, ballooniconPath, MAX_PATH);
        //OPENFILENAMEW ofn = { 0 };
        //ofn.lStructSize = sizeof(OPENFILENAMEW);
        //ofn.hwndOwner = this->hDlg;
        //ofn.lpstrFile = ballooniconPath;
        //ofn.nMaxFile = MAX_PATH;
        //ofn.lpstrFilter = L"图像文件 (*.jpg;*.jpeg;*.png;*.bmp;*.ico)\0*.jpg;*.jpeg;*.png;*.bmp;*.ico\0ALL\0*.*\0";
        //ofn.lpstrTitle = L"选择气泡图标文件";
        //ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        //ofn.lpstrFileTitle = NULL;
        //ofn.nMaxFileTitle = 0;
        //ofn.lpstrInitialDir = L"D:\\";
        //if (GetOpenFileNameW(&ofn))
        //{
        //    SetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, ballooniconPath);
        //}

        this->ShowBalloonIconPathSelectDlg();

        return TRUE;
    }
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
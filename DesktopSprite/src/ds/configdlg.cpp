#include <ds/framework.h>
#include <resource.h>
#include <ds/utils.h>

#include <ds/configdlg.h>

PCWSTR ConfigDlg::GetTemplateName() const
{
    return MAKEINTRESOURCEW(IDD_CONFIG);
}

ConfigDlg::ConfigDlg(MainWindow* mainwnd) : mainwnd(mainwnd) 
{
    this->animeNames = mainwnd->GetSpriteWnd()->GetSpineChar()->GetAnimeNames();
}

void ConfigDlg::SetFormData(const AppConfig::AppConfig* config)
{
    this->form = *config;
}

void ConfigDlg::GetFormData(AppConfig::AppConfig* config) const
{
    *config = this->form;
}

BOOL ConfigDlg::CheckValidFormData()
{
    // check
    if (!IsDlgButtonChecked(this->hDlg, IDC_CHECK_SHOWUSAGE) && !IsDlgButtonChecked(this->hDlg, IDC_CHECK_SHOWNETSPEED))
    {
        this->mainwnd->ShowNoConentWarningMsg();
        return FALSE;
    }

    // collect

    // 系统设置
    this->form.bAutoRun = IsDlgButtonChecked(this->hDlg, IDC_CHECK_AUTORUN);
    this->form.bTimeAlarm = IsDlgButtonChecked(this->hDlg, IDC_CHECK_TIMEALARM);
    this->form.bInfoSound = IsDlgButtonChecked(this->hDlg, IDC_CHECK_INFOSOUND);
    GetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, this->form.szBalloonIconPath, MAX_PATH);

    // 显示设置
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

    // 精灵设置

    // spine 设置
    return TRUE;
}

BOOL ConfigDlg::ShowPathSelectDlg(INT pathEditID, PCWSTR lpstrTitle, PCWSTR lpstrFilter)
{
    // 获得当前显示路径
    WCHAR path[MAX_PATH] = { 0 };
    GetDlgItemTextW(this->hDlg, pathEditID, path, MAX_PATH);

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = this->hDlg;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = lpstrTitle;
    ofn.lpstrFilter = lpstrFilter;

    // OFN_NOCHANGEDIR: 文档里说对 GetOpenFileName 无效, 但其实有效
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn))
    {
        SetDlgItemTextW(this->hDlg, pathEditID, path);
        return TRUE;
    }

    return FALSE;
}

BOOL ConfigDlg::SetSpinePngPath(PCWSTR atlasPath)
{
    WCHAR pngPath[MAX_PATH] = { 0 };
    WCHAR currentAtlasPath[MAX_PATH] = { 0 };
    if (!atlasPath)
    {
        GetDlgItemTextW(this->hDlg, IDC_EDIT_SPATLASPATH, currentAtlasPath, MAX_PATH);
        atlasPath = currentAtlasPath;
    }
    if (SUCCEEDED(StringCchCopyW(pngPath, MAX_PATH, atlasPath)) && 
        SUCCEEDED(PathCchRenameExtension(pngPath, MAX_PATH, L".png")))
    {
        return SetDlgItemTextW(this->hDlg, IDC_EDIT_SPPNGPATH, pngPath);
    }
    return FALSE;
}

BOOL ConfigDlg::InitTrackBar(INT tbID, INT range1, INT range2, INT pageSize, INT freq, INT pos)
{
    SendDlgItemMessageW(this->hDlg, tbID, TBM_SETRANGE, TRUE, MAKELPARAM(range1, range2)); // 范围
    SendDlgItemMessageW(this->hDlg, tbID, TBM_SETPAGESIZE, 0, pageSize); // 单击一下的跳动范围
    SendDlgItemMessageW(this->hDlg, tbID, TBM_SETTICFREQ, freq, 0); // 刻度线频率
    SendDlgItemMessageW(this->hDlg, tbID, TBM_SETPOS, TRUE, pos); // 当前位置
    return TRUE;
}

BOOL ConfigDlg::InitComboBox(INT cbID, PCWSTR curName, INT minVisible)
{
    SendDlgItemMessageW(this->hDlg, cbID, CB_SETMINVISIBLE, minVisible, 0);
    // 在 spine 加载失败的情况下, 不会填充内容
    if (this->animeNames)
    {
        for (auto it = this->animeNames->begin(); it != this->animeNames->end(); it++)
        {
            SendDlgItemMessageW(this->hDlg, cbID, CB_ADDSTRING, 0, (LPARAM)(*it).c_str());
        }
        SendDlgItemMessageW(this->hDlg, cbID, CB_SELECTSTRING, -1, (LPARAM)curName);
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
    // 系统设置
    CheckDlgButton(this->hDlg, IDC_CHECK_AUTORUN, this->form.bAutoRun ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_TIMEALARM, this->form.bTimeAlarm ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_INFOSOUND, this->form.bInfoSound ? BST_CHECKED : BST_UNCHECKED);
    SetDlgItemTextW(this->hDlg, IDC_EDIT_BALLOONICONPATH, this->form.szBalloonIconPath);

    // 显示设置
    CheckDlgButton(this->hDlg, IDC_CHECK_FLOATWND, this->form.bFloatWnd ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_SHOWUSAGE, (this->form.byShowContent & SHOWCONTENT_CPUMEM) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_SHOWNETSPEED, (this->form.byShowContent & SHOWCONTENT_NETSPEED) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_DARKTHEME, this->form.bDarkTheme ? BST_CHECKED : BST_UNCHECKED);
    this->InitTrackBar(IDC_SLIDER_TRANSPARENCY, 0, 100, 1, 10, this->form.transparencyPercent);
    SetDlgItemInt(this->hDlg, IDC_STATIC_TRANSPARENCY, this->form.transparencyPercent, FALSE);

    // 精灵设置
    SetDlgItemTextW(this->hDlg, IDC_EDIT_SPSKELPATH, this->form.szSpineSkelPath);
    SetDlgItemTextW(this->hDlg, IDC_EDIT_SPATLASPATH, this->form.szSpineAtlasPath);
    this->SetSpinePngPath(this->form.szSpineAtlasPath);
    CheckDlgButton(this->hDlg, IDC_CHECK_SHOWSPRITE, this->form.bShowSprite ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_MOUSEPASS, this->form.bSpriteMousePass ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(this->hDlg, IDC_CHECK_ALWAYSBOTTOM, this->form.bAlwaysBottom ? BST_CHECKED : BST_UNCHECKED);
    this->InitTrackBar(IDC_SLIDER_SPMAXFPS, 12, 60, 1, 6, this->form.maxFps);
    SetDlgItemInt(this->hDlg, IDC_STATIC_SPMAXFPS, this->form.maxFps, FALSE);
    this->InitTrackBar(IDC_SLIDER_SPTRANSPARENCY, 0, 100, 1, 10, this->form.spTransparencyPercent);
    SetDlgItemInt(this->hDlg, IDC_STATIC_SPTRANSPARENCY, this->form.spTransparencyPercent, FALSE);
    this->InitTrackBar(IDC_SLIDER_SPSCALE, 10, 200, 5, 10, this->form.spScale);
    SetDlgItemInt(this->hDlg, IDC_STATIC_SPSCALE, this->form.spScale, FALSE);

    // Spine 设置
    this->InitComboBox(IDC_CB_SPIDLE, this->form.spAnimeIdle, 6);
    this->InitComboBox(IDC_CB_SPDRAG, this->form.spAnimeDrag, 6);
    this->InitComboBox(IDC_CB_SPWORK, this->form.spAnimeWork, 6);
    this->InitComboBox(IDC_CB_SPSLEEP, this->form.spAnimeSleep, 6);
    this->InitComboBox(IDC_CB_SPSTAND, this->form.spAnimeStand, 6);
    this->InitComboBox(IDC_CB_SPTOUCH, this->form.spAnimeTouch, 6);
    this->InitComboBox(IDC_CB_SPWINK, this->form.spAnimeWink, 6);
    this->InitComboBox(IDC_CB_SPVICTORY, this->form.spAnimeVictory, 6);
    this->InitComboBox(IDC_CB_SPDANCE, this->form.spAnimeDance, 6);
    this->InitComboBox(IDC_CB_SPDIZZY, this->form.spAnimeDizzy, 6);

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
        this->ShowPathSelectDlg(IDC_EDIT_BALLOONICONPATH, L"选择气泡图标文件", L"图像文件 (*.jpg; *.jpeg; *.png; *.bmp; *.ico)\0*.jpg;*.jpeg;*.png;*.bmp;*.ico\0所有文件 (*.*)\0*.*\0");
        return TRUE;
    case IDC_BTN_SPATLASPATH:
        this->ShowPathSelectDlg(IDC_EDIT_SPATLASPATH, L"选择 Atlas 文件", L"Atlas 文件 (*.atlas)\0*.atlas\0");
        this->SetSpinePngPath();
        return TRUE;
    case IDC_BTN_SPSKELPATH:
        this->ShowPathSelectDlg(IDC_EDIT_SPSKELPATH, L"选择 Skel 文件", L"Skel 文件 (*.skel; *.json)\0*.skel;*.json\0");
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
    HWND trackBar = (HWND)lParam;
    if (trackBar)
    {
        INT trackBarID = GetDlgCtrlID(trackBar);
        switch (LOWORD(wParam))
        {
        case TB_LINEUP:
        case TB_LINEDOWN:
        case TB_PAGEUP:
        case TB_PAGEDOWN:
        case TB_TOP:
        case TB_BOTTOM:
        case TB_ENDTRACK:
            switch (trackBarID)
            {
            case IDC_SLIDER_TRANSPARENCY:
                SetDlgItemInt(this->hDlg, IDC_STATIC_TRANSPARENCY, (UINT)SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0), FALSE);
                break;
            case IDC_SLIDER_SPMAXFPS:
                SetDlgItemInt(this->hDlg, IDC_STATIC_SPMAXFPS, (UINT)SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0), FALSE);
                break;
            case IDC_SLIDER_SPSCALE:
                SetDlgItemInt(this->hDlg, IDC_STATIC_SPSCALE, (UINT)SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0), FALSE);
                break;
            case IDC_SLIDER_SPTRANSPARENCY:
                SetDlgItemInt(this->hDlg, IDC_STATIC_SPTRANSPARENCY, (UINT)SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0), FALSE);
                break;
            default:
                break;
            }
            return TRUE;
        case TB_THUMBPOSITION:
        case TB_THUMBTRACK:
            switch (trackBarID)
            {
            case IDC_SLIDER_TRANSPARENCY:
                SetDlgItemInt(this->hDlg, IDC_STATIC_TRANSPARENCY, HIWORD(wParam), FALSE);
                break;
            case IDC_SLIDER_SPMAXFPS:
                SetDlgItemInt(this->hDlg, IDC_STATIC_SPMAXFPS, HIWORD(wParam), FALSE);
                break;
            case IDC_SLIDER_SPSCALE:
                SetDlgItemInt(this->hDlg, IDC_STATIC_SPSCALE, HIWORD(wParam), FALSE);
                break;
            case IDC_SLIDER_SPTRANSPARENCY:
                SetDlgItemInt(this->hDlg, IDC_STATIC_SPTRANSPARENCY, HIWORD(wParam), FALSE);
                break;
            default:
                break;
            }
            return TRUE;
        default:
            return FALSE;
        }
    }
    return FALSE;
}

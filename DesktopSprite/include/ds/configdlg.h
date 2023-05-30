#pragma once
#ifndef DS_CONFIGDLG_H
#define DS_CONFIGDLG_H

#include <ds/framework.h>
#include <resource.h>
#include <ds/basewindow.h>
#include <ds/config.h>
#include <ds/mainwnd.h>

class ConfigDlg : public BaseDialog
{
public:
    PCWSTR GetTemplateName() const;

private:
    MainWindow* mainwnd = NULL;
    AppConfig::AppConfig form;
    const std::list<std::wstring>* animeNames = NULL;

public:
    ConfigDlg(MainWindow* mainwnd);

    void SetFormData(const AppConfig::AppConfig* config);
    void GetFormData(AppConfig::AppConfig* config) const;

private:
    // 检查并收集对话框里的数据到 form 里
    BOOL CheckValidFormData();

    // 点了确认返回 TRUE, 其余 FALSE
    BOOL ShowPathSelectDlg(INT pathEditID, PCWSTR lpstrTitle, PCWSTR lpstrFilter); 

    BOOL SetSpinePngPath(PCWSTR atlasPath = NULL);

    BOOL InitTrackBar(INT tbID, INT range1, INT range2, INT pageSize, INT freq, INT pos);
    BOOL InitComboBox(INT cbID, PCWSTR curName, INT minVisible);

    BOOL GetComboBoxSelText(INT cbID, PWSTR curName, INT maxLen);

private:
    INT_PTR HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR OnInitDialog(WPARAM wParam, LPARAM lParam);
    INT_PTR OnCommand(WPARAM wParam, LPARAM lParam);
    INT_PTR OnHScroll(WPARAM wParam, LPARAM lParam);
};

#endif // !DS_CONFIGDLG_H

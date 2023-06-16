#pragma once
#ifndef DS_CONFIGDLG_H
#define DS_CONFIGDLG_H

#include <ds/framework.h>
#include <ds/config.h>
#include <ds/basewindow.h>

class MainWindow;

class ConfigDlg : public BaseDialog
{
private:
    MainWindow* mainwnd = NULL;
    AppConfig::AppConfig form;
    const std::list<std::wstring>* animeNames = NULL;
    std::map<INT, HWND> tooltips;

public:
    ConfigDlg(MainWindow* mainwnd);
    PCWSTR GetTemplateName() const;

    void SetFormData(const AppConfig::AppConfig* config);
    void GetFormData(AppConfig::AppConfig* config) const;

private:
    // 检查并收集对话框里的数据到 form 里
    BOOL CheckValidFormData();

    // 点了确认返回 TRUE, 其余 FALSE
    BOOL ShowPathSelectDlg(INT pathEditID, PCWSTR lpstrTitle, PCWSTR lpstrFilter); 

    BOOL SetSpinePngPath(PCWSTR atlasPath = NULL);
    BOOL SetSpineSkelPath(PCWSTR atlasPath = NULL);

    BOOL InitTrackBar(INT tbID, INT range1, INT range2, INT pageSize, INT freq, INT pos);
    BOOL InitComboBox(INT cbID, PCWSTR curName, INT minVisible);

    BOOL GetComboBoxSelText(INT cbID, PWSTR curName, INT maxLen);

    BOOL AddToolTip(INT toolID, PWSTR tip);

private:
    INT_PTR HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR OnDestroy(WPARAM wParam, LPARAM lParam);
    INT_PTR OnInitDialog(WPARAM wParam, LPARAM lParam);
    INT_PTR OnCommand(WPARAM wParam, LPARAM lParam);
    INT_PTR OnHScroll(WPARAM wParam, LPARAM lParam);
};

#endif // !DS_CONFIGDLG_H

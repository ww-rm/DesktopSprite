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

public:
    ConfigDlg(MainWindow* mainwnd);

    void SetFormData(const AppConfig::AppConfig* config);
    void GetFormData(AppConfig::AppConfig* config) const;

private:
    // 检查并收集对话框里的数据到 form 里
    BOOL CheckValidFormData();

    BOOL ShowBalloonIconPathSelectDlg();

private:
    INT_PTR HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR OnInitDialog(WPARAM wParam, LPARAM lParam);
    INT_PTR OnCommand(WPARAM wParam, LPARAM lParam);
    INT_PTR OnHScroll(WPARAM wParam, LPARAM lParam);
};

#endif // !DS_CONFIGDLG_H

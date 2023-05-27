#pragma once
#ifndef DS_ABOUTDLG_H
#define DS_ABOUTDLG_H

#include <ds/framework.h>
#include <ds/basewindow.h>
#include <resource.h>

class AboutDlg : public BaseDialog
{
public:
    PCWSTR GetTemplateName() const;

private:
    INT_PTR HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR OnInitDialog(WPARAM wParam, LPARAM lParam);
    INT_PTR OnNotify(WPARAM wParam, LPARAM lParam);
    INT_PTR OnCommand(WPARAM wParam, LPARAM lParam);
};

#endif // !DS_ABOUTDLG_H


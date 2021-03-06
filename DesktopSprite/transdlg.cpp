#include "framework.h"
#include "util.h"

#include "transdlg.h"

static INT_PTR CALLBACK TransDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR OnInitDialog(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam);
static INT_PTR OnCommand(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam);

static INT_PTR OnInitDialog(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam)
{
    // 保存传入表单指针
    pDlgData->pResultFormData = (PTRANSDLGFORM)lParam;

    // 拷贝表单数据
    CopyMemory(&pDlgData->formData, pDlgData->pResultFormData, sizeof(TRANSDLGFORM));

    // 初始化对话框控件显示
    SetDlgItemInt(
        pDlgData->hDlg, IDC_EDIT_TRANSPARENCY, 
        AlphaToPercent(pDlgData->pResultFormData->byTransparency), 
        FALSE
    );
    return TRUE;
}

static INT_PTR OnCommand(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam)
{
    // HIWORD(wParam) notification code
    // LOWORD(wParam) control identifier
    // lParam is the HWND of control
    switch (LOWORD(wParam))
    {
    case IDOK:
    {
        // 检查数据合法性
        UINT alphaPercent = ClampNum(
            GetDlgItemInt(pDlgData->hDlg, IDC_EDIT_TRANSPARENCY, NULL, FALSE),
            1, 100
        );
        pDlgData->formData.byTransparency = PercentToAlpha(alphaPercent);
        
        // 拷贝至传入表单
        CopyMemory(pDlgData->pResultFormData, &pDlgData->formData, sizeof(TRANSDLGFORM));
    }
    case IDCANCEL:
    {
        return EndDialog(pDlgData->hDlg, wParam);
    }
    default:
    {
        return FALSE;
    }
    }
}

static INT_PTR CALLBACK TransDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PTRANSDLGDATA pDlgData = (PTRANSDLGDATA)GetWndData(hDlg);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        pDlgData = (PTRANSDLGDATA)DefAllocMem(sizeof(TRANSDLGDATA));
        if (pDlgData == NULL)
            return EndDialog(hDlg, FALSE);
        pDlgData->hDlg = hDlg;
        SetWndData(hDlg, pDlgData);
        return OnInitDialog(pDlgData, wParam, lParam);
    case WM_NCDESTROY:
        DefFreeMem(pDlgData);
        return TRUE;
    case WM_COMMAND:
        return OnCommand(pDlgData, wParam, lParam);
    default:
        return FALSE;
    }
}

INT_PTR DialogBoxTrans(HINSTANCE hInstance, HWND hWndParent, PTRANSDLGFORM pInitForm)
{
    return DialogBoxParamW(
        hInstance, MAKEINTRESOURCEW(IDD_TRANSPARENCY), 
        hWndParent, TransDlgProc, (LPARAM)pInitForm
    );
}
#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/transdlg.h>


static INT_PTR CALLBACK TransDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR OnInitDialog(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam);
static INT_PTR OnCommand(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam);

static INT_PTR OnInitDialog(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam)
{
    // 保存传入表单指针
    pDlgData->pResultFormData = (PTRANSDLGFORM)lParam;

    // 拷贝表单数据
    CopyMemory(&pDlgData->formData, pDlgData->pResultFormData, sizeof(TRANSDLGFORM));

    WCHAR percentStr[128] = { 0 };
    StringCchPrintfW(percentStr, 128, L"%.2lf", pDlgData->pResultFormData->transparencyPercent);

    // 初始化对话框控件显示
    SetDlgItemTextW(pDlgData->hDlg, IDC_EDIT_TRANSPARENCY, percentStr);
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
        WCHAR percentStr[128] = { 0 };
        GetDlgItemTextW(pDlgData->hDlg, IDC_EDIT_TRANSPARENCY, percentStr, 128);
        pDlgData->formData.transparencyPercent = ClampNum(wcstod(percentStr, NULL), 1, 100);
        
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
        pDlgData = new TRANSDLGDATA;
        if (pDlgData == NULL)
            return EndDialog(hDlg, FALSE);
        pDlgData->hDlg = hDlg;
        SetWndData(hDlg, (LONG_PTR)pDlgData);
        return OnInitDialog(pDlgData, wParam, lParam);
    case WM_NCDESTROY:
        delete pDlgData;
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
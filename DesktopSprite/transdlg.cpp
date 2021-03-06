#include "framework.h"
#include "util.h"

#include "transdlg.h"

static INT_PTR CALLBACK TransDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR OnInitDialog(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam);
static INT_PTR OnCommand(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam);

static INT_PTR OnInitDialog(PTRANSDLGDATA pDlgData, WPARAM wParam, LPARAM lParam)
{
    // ���洫���ָ��
    pDlgData->pResultFormData = (PTRANSDLGFORM)lParam;

    // ����������
    CopyMemory(&pDlgData->formData, pDlgData->pResultFormData, sizeof(TRANSDLGFORM));

    // ��ʼ���Ի���ؼ���ʾ
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
        // ������ݺϷ���
        UINT alphaPercent = ClampNum(
            GetDlgItemInt(pDlgData->hDlg, IDC_EDIT_TRANSPARENCY, NULL, FALSE),
            1, 100
        );
        pDlgData->formData.byTransparency = PercentToAlpha(alphaPercent);
        
        // �����������
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
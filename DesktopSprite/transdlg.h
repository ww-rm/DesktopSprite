#pragma once
#include "framework.h"

typedef struct _TRANSDLGFORM
{
    BYTE byTransparency;
}TRANSDLGFORM, *PTRANSDLGFORM;

typedef struct _TRANSDLGDATA
{
    HWND hDlg;
    PTRANSDLGFORM pResultFormData;      // 外部表单指针
    TRANSDLGFORM formData;              // 内部表单信息
}TRANSDLGDATA, *PTRANSDLGDATA;

INT_PTR DialogBoxTrans(HINSTANCE hInstance, HWND hWndParent, PTRANSDLGFORM pInitForm);
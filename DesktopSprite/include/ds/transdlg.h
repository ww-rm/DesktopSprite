#pragma once
#include "framework.h"

typedef struct _TRANSDLGFORM
{
    BYTE byTransparency;
}TRANSDLGFORM, *PTRANSDLGFORM;

typedef struct _TRANSDLGDATA
{
    HWND hDlg;
    PTRANSDLGFORM pResultFormData;      // �ⲿ��ָ��
    TRANSDLGFORM formData;              // �ڲ�����Ϣ
}TRANSDLGDATA, *PTRANSDLGDATA;

INT_PTR DialogBoxTrans(HINSTANCE hInstance, HWND hWndParent, PTRANSDLGFORM pInitForm);
#pragma once
#include "framework.h"

// Ĭ�ϲ��������߳�
#define DefCreateThread(lpStartAddress, lpParameter) \
        CreateThread(NULL, 0, (lpStartAddress), (lpParameter), 0, NULL)

// �ӽ���Ĭ�϶ѷ����ѳ�ʼ������ڴ�
#define DefAllocMem(dwBytes) \
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dwBytes))

// �ӽ���Ĭ�϶��ͷ��ڴ�
#define DefFreeMem(lpMem) \
        HeapFree(GetProcessHeap(), 0, (lpMem))

// ���ļ��ж�ȡͼ���ļ�(��Ҫ��DestroyIcon�ͷ�)
#define LoadIconFromFile(name) \
        LoadImageW(NULL, (name), IMAGE_ICON, 0, 0, LR_LOADFROMFILE)

// ��ȡԭʼ��Сͼ����Դ(��Ҫ��DestroyIcon�ͷ�)
#define LoadIconRawSize(hInst, name) \
        LoadImageW(hInst, name, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR)

#define GetWndData(hWnd) \
        GetWindowLongPtrW((hWnd), GWLP_USERDATA)

#define SetWndData(hWnd, dwNewData) \
        SetWindowLongPtrW((hWnd), GWLP_USERDATA, (LONG_PTR)(dwNewData))

// �����ֽ��ٶȳɴ���λ���ַ���
DWORD ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest);

// �ж��Ƿ�����ɫ����
BOOL IsDarkTheme();

#pragma once
#include "framework.h"

// 默认参数创建线程
#define DefCreateThread(lpStartAddress, lpParameter) \
        CreateThread(NULL, 0, (lpStartAddress), (lpParameter), 0, NULL)

// 从进程默认堆分配已初始化零的内存
#define DefAllocMem(dwBytes) \
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dwBytes))

// 从进程默认堆释放内存
#define DefFreeMem(lpMem) \
        HeapFree(GetProcessHeap(), 0, (lpMem))

// 从文件中读取图标文件(需要用DestroyIcon释放)
#define LoadIconFromFile(name) \
        LoadImageW(NULL, (name), IMAGE_ICON, 0, 0, LR_LOADFROMFILE)

// 读取原始大小图标资源(需要用DestroyIcon释放)
#define LoadIconRawSize(hInst, name) \
        LoadImageW(hInst, name, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR)

#define GetWndData(hWnd) \
        GetWindowLongPtrW((hWnd), GWLP_USERDATA)

#define SetWndData(hWnd, dwNewData) \
        SetWindowLongPtrW((hWnd), GWLP_USERDATA, (LONG_PTR)(dwNewData))

// 换算字节速度成带单位的字符串
DWORD ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest);

// 判断是否是深色主题
BOOL IsDarkTheme();

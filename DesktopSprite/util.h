#pragma once
#include "framework.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
        ((HICON)LoadImageW(NULL, (name), IMAGE_ICON, 0, 0, LR_LOADFROMFILE))

// 读取原始大小图标资源(需要用DestroyIcon释放)
#define LoadIconRawSize(hInst, name) \
        ((HICON)LoadImageW(hInst, name, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR))

#define GetWndData(hWnd) \
        GetWindowLongPtrW((hWnd), GWLP_USERDATA)

#define SetWndData(hWnd, dwNewData) \
        SetWindowLongPtrW((hWnd), GWLP_USERDATA, (LONG_PTR)(dwNewData))

// 向注册表子键写入二进制数据
#define RegSetBinValue(hKey, lpValueName, lpData, cbData) \
        RegSetValueExW((hKey), (lpValueName), 0, REG_BINARY, (PBYTE)(lpData), (cbData))

// 从注册表子键查询二进制数据
#define RegQueryAnyValue(hKey, lpValueName, lpData, lpcbData) \
        RegQueryValueExW((hKey), (lpValueName), NULL, NULL, (PBYTE)(lpData), (lpcbData))

// 打开 HEKY_CURRENT_USER\SOFTWARE 子键
#define RegOpenSoftwareKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE", 0, KEY_ALL_ACCESS, (phkResult))

// 打开 HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
#define RegOpenRunKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, (phkResult))

// 打开 HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize
#define RegOpenPersonalizeKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, (phkResult))

// 设置应用开机自启
    DWORD SetAppAutoRun();

    // 取消应用开机自启
    DWORD UnsetAppAutoRun();

    // 换算字节速度成带单位的字符串
    DWORD ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest);

    // 获取与整点时间差毫秒
    UINT GetHourTimeDiff();

    // 设置菜单项状态
    DWORD SetMenuItemState(HMENU hMenu, UINT uIdentifier, UINT uState);

    // 获得菜单项状态
    UINT GetMenuItemState(HMENU hMenu, UINT uIdentifier);

    // 获取当前系统标题字体信息
    DWORD GetSystemCapitalFont(PLOGFONTW pLogFont);

    // 判断系统是否是深色主题
    BOOL IsSystemDarkTheme();

#ifdef __cplusplus
}
#endif // __cplusplus
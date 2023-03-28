#pragma once
#include <ds/framework.h>

// 默认参数创建线程
#define DefCreateThread(lpStartAddress, lpParameter) \
        (CreateThread(NULL, 0, (lpStartAddress), (lpParameter), 0, NULL))

// 从进程默认堆分配已初始化零的内存
#define DefAllocMem(dwBytes) \
        (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dwBytes)))

#define DefCreateFile(lpFileName, dwDesiredAccess, dwCreationDisposition) \
        (CreateFileW((lpFileName), (dwDesiredAccess), 0, NULL, (dwCreationDisposition), FILE_ATTRIBUTE_NORMAL, NULL))

// 从进程默认堆释放内存
#define DefFreeMem(lpMem) \
        (HeapFree(GetProcessHeap(), 0, (lpMem)))

// 从文件中读取图标文件(需要用DestroyIcon释放)
#define LoadIconFromFile(name) \
        ((HICON)LoadImageW(NULL, (name), IMAGE_ICON, 0, 0, LR_LOADFROMFILE))

// 读取原始大小图标资源(需要用DestroyIcon释放)
#define LoadIconRawSize(hInst, name) \
        ((HICON)LoadImageW((hInst), (name), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR))

// 获取窗口 USERDATA
#define GetWndData(hWnd) \
        (GetWindowLongPtrW((hWnd), GWLP_USERDATA))

// 设置窗口 USERDATA
#define SetWndData(hWnd, dwNewData) \
        (SetWindowLongPtrW((hWnd), GWLP_USERDATA, (LONG_PTR)(dwNewData)))

// 获取对话框 USER
#define GetDlgData(hDlg) \
        (GetWindowLongPtrW((hDlg), DWLP_USER))

// 设置对话框 USER
#define SetDlgData(hDlg, dwNewData) \
        (SetWindowLongPtrW((hDlg), DWLP_USER, (LONG_PTR)(dwNewData)))

// 向注册表子键写入二进制数据
#define RegSetBinValue(hKey, lpValueName, lpData, cbData) \
        (RegSetValueExW((hKey), (lpValueName), 0, REG_BINARY, (PBYTE)(lpData), (cbData)))

// 从注册表子键查询二进制数据
#define RegQueryAnyValue(hKey, lpValueName, lpData, lpcbData) \
        (RegQueryValueExW((hKey), (lpValueName), NULL, NULL, (PBYTE)(lpData), (lpcbData)))

// 打开 HEKY_CURRENT_USER\SOFTWARE 子键
#define RegOpenSoftwareKey(phkResult) \
        (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE", 0, KEY_ALL_ACCESS, (phkResult)))

// 打开 HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
#define RegOpenRunKey(phkResult) \
        (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, (phkResult)))

// 打开 HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize
#define RegOpenPersonalizeKey(phkResult) \
        (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, (phkResult)))

// 裁剪数字
#define ClampNum(numVal, minVal, maxVal) \
        (min(max((minVal), (numVal)), (maxVal)))

// Alpha 和百分比转换
#define AlphaToPercent(alpha) \
        ((BYTE)((INT)(alpha) * 100 / 255))

// 百分比和 Alpha 转换
#define PercentToAlpha(percent) \
        ((BYTE)((INT)(percent) * 255 / 100))

// 设置应用开机自启
DWORD SetAppAutoRun();

// 取消应用开机自启
DWORD UnsetAppAutoRun();

// 换算字节速度成带单位的字符串
INT ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest);

// 换算字节速度, 得到单位, 并且返回换算次数
INT GetSpeedUnit(DOUBLE fSpeed, PWSTR szUnit, SIZE_T cchDest);

// 获取与整点时间差毫秒
UINT GetMillisecondsToNextHour();

// 判断现在是否是整点 (分钟数为 0)
BOOL IsOnTheHour();

// 设置菜单项状态
DWORD SetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uState);

// 获得菜单项状态
UINT GetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition);

// 设置菜单项类型
DWORD SetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uType);

// 获得菜单项类型
UINT GetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition);

// 获取当前系统标题字体信息
DWORD GetSystemCapitalFont(PLOGFONTW pLogFont);

// 判断系统是否是深色主题
BOOL IsSystemDarkTheme();

// 获取系统当前分辨率
DWORD GetScreenResolution(PSIZE psizeResolution);

// 获取任务栏方向
UINT GetShellTrayDirection();

//// 往 FilePath 解压资源
DWORD ExtractRes(UINT uResID, PCWSTR szResType, PCWSTR szFilePath);

// 获取 TTF 资源内存
PBYTE GetResPointer(UINT uResID, PCWSTR szResType, DWORD* cbData);

// 在不同分辨率之间折算坐标相对位置
DWORD ConvertPointForResolution(PPOINT ptOld, PSIZE sizeOld, PSIZE sizeNew, PPOINT ptNew);

// Copy POINT
DWORD CopyPoint(PPOINT ptSrc, PPOINT ptDst);

// Copy SIZE
DWORD CopySize(PSIZE sizeSrc, PSIZE sizeDst);

#pragma once
#ifndef DS_UTILS_H
#define DS_UTILS_H

#include <ds/framework.h>

class HighResolutionTimer
{
private:
    LARGE_INTEGER freq = { 0 };
    BOOL stoped = TRUE;
    LARGE_INTEGER start = { 0 };
    LARGE_INTEGER end = { 0 };

public:
    HighResolutionTimer() { QueryPerformanceFrequency(&this->freq); }
    void Start() { this->stoped = FALSE; QueryPerformanceCounter(&this->start); }
    void Stop() { this->stoped = TRUE; QueryPerformanceCounter(&this->end); }
    
    FLOAT GetSeconds();
    FLOAT GetMilliseconds();
};

// 默认参数创建线程
HANDLE DefCreateThread(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter);

// 默认参数创建文件
HANDLE DefCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwCreationDisposition);

// 获取窗口 GWLP_USERDATA
LONG_PTR GetWndData(HWND hWnd);
// 设置窗口 GWLP_USERDATA
LONG_PTR SetWndData(HWND hWnd, LONG_PTR dwNewData);

// 获取对话框 DWLP_USER
LONG_PTR GetDlgData(HWND hDlg);

// 设置对话框 DWLP_USER
LONG_PTR SetDlgData(HWND hDlg, LONG_PTR dwNewData);

// 向注册表子键写入二进制数据
LSTATUS RegSetBinValue(HKEY hKey, PCWSTR lpValueName, PBYTE lpData, DWORD cbData);

// 从注册表子键查询二进制数据
LSTATUS RegQueryAnyValue(HKEY hKey, PCWSTR lpValueName, PBYTE lpData, PDWORD lpcbData);

// 打开 HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
LSTATUS RegOpenRunKey(PHKEY phkResult);

// 打开 HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize
LSTATUS RegOpenPersonalizeKey(PHKEY phkResult);

// 裁剪数字
DOUBLE ClampNum(DOUBLE numVal, DOUBLE minVal, DOUBLE maxVal);

// Alpha 和百分比转换
UINT AlphaToPercent(BYTE alpha);

// 百分比和 Alpha 转换, percent 是乘了 100 的值
BYTE PercentToAlpha(UINT percent);

// 设置应用开机自启
BOOL SetAppAutoRun(PCWSTR appname);

// 取消应用开机自启
BOOL UnsetAppAutoRun(PCWSTR appname);

// 换算字节速度成带单位的字符串
INT ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest);

// 换算字节速度, 得到单位, 并且返回换算次数
INT GetSpeedUnit(DOUBLE fSpeed, PWSTR szUnit, SIZE_T cchDest);

// 获取与整点时间差毫秒
UINT GetMillisecondsToNextHour();

// 判断现在是否是整点 (分钟数为 0)
BOOL IsOnTheHour();

// 设置菜单项状态
BOOL SetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uState);

// 获得菜单项状态
UINT GetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition);

// 设置菜单项类型
BOOL SetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uType);

// 获得菜单项类型
UINT GetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition);

// 获取当前系统标题字体信息
BOOL GetSystemCapitalFont(PLOGFONTW pLogFont);

// 判断系统是否是深色主题
BOOL IsSystemDarkTheme();

// 获取系统当前分辨率
BOOL GetScreenResolution(PSIZE psizeResolution);

// 获取任务栏方向, ABE_XXXXX
UINT GetShellTrayDirection();

// 在不同分辨率之间折算坐标相对位置, ptOld 和 ptNew 可以是同一个指针
BOOL ConvertPointForResolution(const PSIZE sizeOld, const PPOINT ptOld, const PSIZE sizeNew, PPOINT ptNew);

// Copy POINT
void CopyPoint(const POINT* ptSrc, PPOINT ptDst);

// Copy SIZE
void CopySize(const SIZE* sizeSrc, PSIZE sizeDst);

// 窄字符转宽字符
INT StrAtoW(PCSTR aStr, PWSTR wStr, INT wStrLen);

// 宽字符转窄字符
INT StrWtoA(PCWSTR wStr, PSTR aStr, INT aStrLen);

// 比较两个矩形的范围, rc1 包含于 rc2 返回负数, rc1 包含 rc2 返回正数, 其余返回 0
INT CheckRectContainment(const RECT* rc1, const RECT* rc2);

// 高精度休眠
void HighResolutionSleep(DWORD dwMilliseconds);

// 获取系统当前拖动判定范围, 自动取正
BOOL GetSysDragSize(PSIZE psizeDrag);

FLOAT Sigmoid(FLOAT x);

// Show error Line and GetLastError
void ShowLastError(PCWSTR func, INT line = -1);

#endif // !DS_UTILS_H

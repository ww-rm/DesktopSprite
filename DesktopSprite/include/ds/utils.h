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

// Ĭ�ϲ��������߳�
HANDLE DefCreateThread(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter);

// Ĭ�ϲ��������ļ�
HANDLE DefCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwCreationDisposition);

// ��ȡ���� GWLP_USERDATA
LONG_PTR GetWndData(HWND hWnd);
// ���ô��� GWLP_USERDATA
LONG_PTR SetWndData(HWND hWnd, LONG_PTR dwNewData);

// ��ȡ�Ի��� DWLP_USER
LONG_PTR GetDlgData(HWND hDlg);

// ���öԻ��� DWLP_USER
LONG_PTR SetDlgData(HWND hDlg, LONG_PTR dwNewData);

// ��ע����Ӽ�д�����������
LSTATUS RegSetBinValue(HKEY hKey, PCWSTR lpValueName, PBYTE lpData, DWORD cbData);

// ��ע����Ӽ���ѯ����������
LSTATUS RegQueryAnyValue(HKEY hKey, PCWSTR lpValueName, PBYTE lpData, PDWORD lpcbData);

// �� HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
LSTATUS RegOpenRunKey(PHKEY phkResult);

// �� HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize
LSTATUS RegOpenPersonalizeKey(PHKEY phkResult);

// �ü�����
DOUBLE ClampNum(DOUBLE numVal, DOUBLE minVal, DOUBLE maxVal);

// Alpha �Ͱٷֱ�ת��
UINT AlphaToPercent(BYTE alpha);

// �ٷֱȺ� Alpha ת��, percent �ǳ��� 100 ��ֵ
BYTE PercentToAlpha(UINT percent);

// ����Ӧ�ÿ�������
BOOL SetAppAutoRun(PCWSTR appname);

// ȡ��Ӧ�ÿ�������
BOOL UnsetAppAutoRun(PCWSTR appname);

// �����ֽ��ٶȳɴ���λ���ַ���
INT ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest);

// �����ֽ��ٶ�, �õ���λ, ���ҷ��ػ������
INT GetSpeedUnit(DOUBLE fSpeed, PWSTR szUnit, SIZE_T cchDest);

// ��ȡ������ʱ������
UINT GetMillisecondsToNextHour();

// �ж������Ƿ������� (������Ϊ 0)
BOOL IsOnTheHour();

// ���ò˵���״̬
BOOL SetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uState);

// ��ò˵���״̬
UINT GetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition);

// ���ò˵�������
BOOL SetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uType);

// ��ò˵�������
UINT GetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition);

// ��ȡ��ǰϵͳ����������Ϣ
BOOL GetSystemCapitalFont(PLOGFONTW pLogFont);

// �ж�ϵͳ�Ƿ�����ɫ����
BOOL IsSystemDarkTheme();

// ��ȡϵͳ��ǰ�ֱ���
BOOL GetScreenResolution(PSIZE psizeResolution);

// ��ȡ����������, ABE_XXXXX
UINT GetShellTrayDirection();

// �ڲ�ͬ�ֱ���֮�������������λ��, ptOld �� ptNew ������ͬһ��ָ��
BOOL ConvertPointForResolution(const PSIZE sizeOld, const PPOINT ptOld, const PSIZE sizeNew, PPOINT ptNew);

// Copy POINT
void CopyPoint(const POINT* ptSrc, PPOINT ptDst);

// Copy SIZE
void CopySize(const SIZE* sizeSrc, PSIZE sizeDst);

// խ�ַ�ת���ַ�
INT StrAtoW(PCSTR aStr, PWSTR wStr, INT wStrLen);

// ���ַ�תխ�ַ�
INT StrWtoA(PCWSTR wStr, PSTR aStr, INT aStrLen);

// �Ƚ��������εķ�Χ, rc1 ������ rc2 ���ظ���, rc1 ���� rc2 ��������, ���෵�� 0
INT CheckRectContainment(const RECT* rc1, const RECT* rc2);

// �߾�������
void HighResolutionSleep(DWORD dwMilliseconds);

// ��ȡϵͳ��ǰ�϶��ж���Χ, �Զ�ȡ��
BOOL GetSysDragSize(PSIZE psizeDrag);

FLOAT Sigmoid(FLOAT x);

// Show error Line and GetLastError
void ShowLastError(PCWSTR func, INT line = -1);

#endif // !DS_UTILS_H

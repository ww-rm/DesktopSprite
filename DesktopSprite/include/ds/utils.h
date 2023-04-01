#pragma once
#ifndef DS_UTILS_H
#define DS_UTILS_H

#include <ds/framework.h>

// Ĭ�ϲ��������߳�
HANDLE DefCreateThread(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter);

// Ĭ�ϲ��������ļ�
HANDLE DefCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwCreationDisposition);

//// ���ļ��ж�ȡͼ���ļ�(��Ҫ��DestroyIcon�ͷ�)
//#define LoadIconFromFile(name) \
//        ((HICON)LoadImageW(NULL, (name), IMAGE_ICON, 0, 0, LR_LOADFROMFILE))
//
// ��ȡԭʼ��Сͼ����Դ(��Ҫ��DestroyIcon�ͷ�)
#define LoadIconRawSize(hInst, name) \
        ((HICON)LoadImageW((hInst), (name), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR))

// ��ȡ���� USERDATA
LONG_PTR GetWndData(HWND hWnd);
// ���ô��� USERDATA
LONG_PTR SetWndData(HWND hWnd, LONG_PTR dwNewData);

// ��ȡ�Ի��� USER
LONG_PTR GetDlgData(HWND hDlg);

// ���öԻ��� USER
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
DOUBLE AlphaToPercent(BYTE alpha);

// �ٷֱȺ� Alpha ת��, percent �ǳ��� 100 ��ֵ
BYTE PercentToAlpha(DOUBLE percent);

// ����Ӧ�ÿ�������
DWORD SetAppAutoRun(PCWSTR appname);

// ȡ��Ӧ�ÿ�������
DWORD UnsetAppAutoRun(PCWSTR appname);

// �����ֽ��ٶȳɴ���λ���ַ���
INT ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest);

// �����ֽ��ٶ�, �õ���λ, ���ҷ��ػ������
INT GetSpeedUnit(DOUBLE fSpeed, PWSTR szUnit, SIZE_T cchDest);

// ��ȡ������ʱ������
UINT GetMillisecondsToNextHour();

// �ж������Ƿ������� (������Ϊ 0)
BOOL IsOnTheHour();

// ���ò˵���״̬
DWORD SetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uState);

// ��ò˵���״̬
UINT GetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition);

// ���ò˵�������
DWORD SetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uType);

// ��ò˵�������
UINT GetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition);

// ��ȡ��ǰϵͳ����������Ϣ
DWORD GetSystemCapitalFont(PLOGFONTW pLogFont);

// �ж�ϵͳ�Ƿ�����ɫ����
BOOL IsSystemDarkTheme();

// ��ȡϵͳ��ǰ�ֱ���
DWORD GetScreenResolution(PSIZE psizeResolution);

// ��ȡ����������, ABE_XXXXX
UINT GetShellTrayDirection();

//// �� FilePath ��ѹ��Դ
DWORD ExtractRes(UINT uResID, PCWSTR szResType, PCWSTR szFilePath);

// ��ȡ TTF ��Դ�ڴ�
PBYTE GetResPointer(UINT uResID, PCWSTR szResType, DWORD* cbData);

// �ڲ�ͬ�ֱ���֮�������������λ��
DWORD ConvertPointForResolution(PPOINT ptOld, PSIZE sizeOld, PSIZE sizeNew, PPOINT ptNew);

// Copy POINT
DWORD CopyPoint(PPOINT ptSrc, PPOINT ptDst);

// Copy SIZE
DWORD CopySize(PSIZE sizeSrc, PSIZE sizeDst);

// Show error Line and GetLastError
void ShowLastError(PCWSTR func, INT line = -1);

#endif // !DS_UTILS_H

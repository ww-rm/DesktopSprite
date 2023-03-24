#pragma once
#include <ds/framework.h>

// Ĭ�ϲ��������߳�
#define DefCreateThread(lpStartAddress, lpParameter) \
        (CreateThread(NULL, 0, (lpStartAddress), (lpParameter), 0, NULL))

// �ӽ���Ĭ�϶ѷ����ѳ�ʼ������ڴ�
#define DefAllocMem(dwBytes) \
        (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dwBytes)))

#define DefCreateFile(lpFileName, dwDesiredAccess, dwCreationDisposition) \
        (CreateFileW((lpFileName), (dwDesiredAccess), 0, NULL, (dwCreationDisposition), FILE_ATTRIBUTE_NORMAL, NULL))

// �ӽ���Ĭ�϶��ͷ��ڴ�
#define DefFreeMem(lpMem) \
        (HeapFree(GetProcessHeap(), 0, (lpMem)))

// ���ļ��ж�ȡͼ���ļ�(��Ҫ��DestroyIcon�ͷ�)
#define LoadIconFromFile(name) \
        ((HICON)LoadImageW(NULL, (name), IMAGE_ICON, 0, 0, LR_LOADFROMFILE))

// ��ȡԭʼ��Сͼ����Դ(��Ҫ��DestroyIcon�ͷ�)
#define LoadIconRawSize(hInst, name) \
        ((HICON)LoadImageW((hInst), (name), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR))

// ��ȡ���� USERDATA
#define GetWndData(hWnd) \
        (GetWindowLongPtrW((hWnd), GWLP_USERDATA))

// ���ô��� USERDATA
#define SetWndData(hWnd, dwNewData) \
        (SetWindowLongPtrW((hWnd), GWLP_USERDATA, (LONG_PTR)(dwNewData)))

// ��ȡ�Ի��� USER
#define GetDlgData(hDlg) \
        (GetWindowLongPtrW((hDlg), DWLP_USER))

// ���öԻ��� USER
#define SetDlgData(hDlg, dwNewData) \
        (SetWindowLongPtrW((hDlg), DWLP_USER, (LONG_PTR)(dwNewData)))

// ��ע����Ӽ�д�����������
#define RegSetBinValue(hKey, lpValueName, lpData, cbData) \
        (RegSetValueExW((hKey), (lpValueName), 0, REG_BINARY, (PBYTE)(lpData), (cbData)))

// ��ע����Ӽ���ѯ����������
#define RegQueryAnyValue(hKey, lpValueName, lpData, lpcbData) \
        (RegQueryValueExW((hKey), (lpValueName), NULL, NULL, (PBYTE)(lpData), (lpcbData)))

// �� HEKY_CURRENT_USER\SOFTWARE �Ӽ�
#define RegOpenSoftwareKey(phkResult) \
        (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE", 0, KEY_ALL_ACCESS, (phkResult)))

// �� HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
#define RegOpenRunKey(phkResult) \
        (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, (phkResult)))

// �� HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize
#define RegOpenPersonalizeKey(phkResult) \
        (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, (phkResult)))

// �ü�����
#define ClampNum(numVal, minVal, maxVal) \
        (min(max((minVal), (numVal)), (maxVal)))

// Alpha �Ͱٷֱ�ת��
#define AlphaToPercent(alpha) \
        ((BYTE)((INT)(alpha) * 100 / 255))

// �ٷֱȺ� Alpha ת��
#define PercentToAlpha(percent) \
        ((BYTE)((INT)(percent) * 255 / 100))

// ����Ӧ�ÿ�������
DWORD SetAppAutoRun();

// ȡ��Ӧ�ÿ�������
DWORD UnsetAppAutoRun();

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

// ��ȡ����������
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

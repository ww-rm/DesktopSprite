#pragma once
#include "framework.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
        ((HICON)LoadImageW(NULL, (name), IMAGE_ICON, 0, 0, LR_LOADFROMFILE))

// ��ȡԭʼ��Сͼ����Դ(��Ҫ��DestroyIcon�ͷ�)
#define LoadIconRawSize(hInst, name) \
        ((HICON)LoadImageW(hInst, name, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR))

#define GetWndData(hWnd) \
        GetWindowLongPtrW((hWnd), GWLP_USERDATA)

#define SetWndData(hWnd, dwNewData) \
        SetWindowLongPtrW((hWnd), GWLP_USERDATA, (LONG_PTR)(dwNewData))

// ��ע����Ӽ�д�����������
#define RegSetBinValue(hKey, lpValueName, lpData, cbData) \
        RegSetValueExW((hKey), (lpValueName), 0, REG_BINARY, (PBYTE)(lpData), (cbData))

// ��ע����Ӽ���ѯ����������
#define RegQueryAnyValue(hKey, lpValueName, lpData, lpcbData) \
        RegQueryValueExW((hKey), (lpValueName), NULL, NULL, (PBYTE)(lpData), (lpcbData))

// �� HEKY_CURRENT_USER\SOFTWARE �Ӽ�
#define RegOpenSoftwareKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE", 0, KEY_ALL_ACCESS, (phkResult))

// �� HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
#define RegOpenRunKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, (phkResult))

// �� HEKY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize
#define RegOpenPersonalizeKey(phkResult) \
        RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, (phkResult))

// ����Ӧ�ÿ�������
    DWORD SetAppAutoRun();

    // ȡ��Ӧ�ÿ�������
    DWORD UnsetAppAutoRun();

    // �����ֽ��ٶȳɴ���λ���ַ���
    DWORD ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest);

    // ��ȡ������ʱ������
    UINT GetHourTimeDiff();

    // ���ò˵���״̬
    DWORD SetMenuItemState(HMENU hMenu, UINT uIdentifier, UINT uState);

    // ��ò˵���״̬
    UINT GetMenuItemState(HMENU hMenu, UINT uIdentifier);

    // ��ȡ��ǰϵͳ����������Ϣ
    DWORD GetSystemCapitalFont(PLOGFONTW pLogFont);

    // �ж�ϵͳ�Ƿ�����ɫ����
    BOOL IsSystemDarkTheme();

#ifdef __cplusplus
}
#endif // __cplusplus
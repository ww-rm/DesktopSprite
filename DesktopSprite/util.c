#include "framework.h"

#include "util.h"

DWORD SetAppAutoRun()
{
    DWORD dwErrorCode = ERROR_SUCCESS;
    HKEY hkRun = NULL;

    // ��������  
    dwErrorCode = RegOpenRunKey(&hkRun);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        // �õ������������ȫ·��
        WCHAR szExeFullPath[MAX_PATH];
        GetModuleFileNameW(NULL, szExeFullPath, MAX_PATH);
        PathQuoteSpacesW(szExeFullPath); // ����˫����

        // �õ�Ҫд������ݴ�С
        size_t cchPath = 0;
        StringCchLengthW(szExeFullPath, MAX_PATH, &cchPath);

        dwErrorCode = RegSetValueExW(
            hkRun, APPNAME, 0, REG_SZ,
            (LPBYTE)szExeFullPath, (DWORD)(sizeof(WCHAR) * (cchPath + 1))
        );
    }

    if (hkRun != NULL)
    {
        RegCloseKey(hkRun);
    }
    return dwErrorCode;
}

DWORD UnsetAppAutoRun()
{
    DWORD dwErrorCode = ERROR_SUCCESS;
    HKEY hkRun = NULL;

    // ��������  
    dwErrorCode = RegOpenRunKey(&hkRun);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        dwErrorCode = RegDeleteValueW(hkRun, APPNAME);

    }
    if (hkRun != NULL)
    {
        RegCloseKey(hkRun);
    }
    return dwErrorCode;
}


DWORD ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest)
{
    PCWSTR dataUnits[5] = { L"B/s", L"KB/s", L"MB/s", L"GB/s", L"TB/s" };
    UINT uID = 0;
    while (fSpeed > 100)
    {
        uID++;
        fSpeed /= 1024;
    }
    StringCchPrintfW(szFormatted, cchDest, L"%.2f%s", fSpeed, dataUnits[uID]);
    return 0;
}

DWORD GetSystemCapitalFont(PLOGFONTW pLogFont)
{
    NONCLIENTMETRICSW ncMetrics = { 0 };
    ncMetrics.cbSize = sizeof(NONCLIENTMETRICSW);

    // ��ȡ��ǰϵͳ������������
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncMetrics, 0);
    CopyMemory(pLogFont, &ncMetrics.lfCaptionFont, sizeof(LOGFONTW));
    return 0;
}

BOOL IsSystemDarkTheme()
{
    BOOL bRet = TRUE;
    HKEY hkPersonalize = NULL;
    if (RegOpenPersonalizeKey(&hkPersonalize) == ERROR_SUCCESS)
    {
        DWORD dwSystemUsesLightTheme = 0;
        DWORD cbData = sizeof(DWORD);
        RegQueryAnyValue(hkPersonalize, L"SystemUsesLightTheme", &dwSystemUsesLightTheme, &cbData);
        bRet = !dwSystemUsesLightTheme;
    }
    else
    {
        // TODO: ������
    }

    if (hkPersonalize != NULL)
    {
        RegCloseKey(hkPersonalize);
    }
    return bRet;
}
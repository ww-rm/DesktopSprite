#include "framework.h"

#include "util.h"

DWORD SetAppAutoRun()
{
    DWORD dwErrorCode = ERROR_SUCCESS;
    HKEY hkRun = NULL;

    // 打开启动项  
    dwErrorCode = RegOpenRunKey(&hkRun);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        // 得到本程序自身的全路径
        WCHAR szExeFullPath[MAX_PATH];
        GetModuleFileNameW(NULL, szExeFullPath, MAX_PATH);
        PathQuoteSpacesW(szExeFullPath); // 补上双引号

        // 得到要写入的数据大小
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

    // 打开启动项  
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

INT ConvertSpeed(DOUBLE fSpeed, PWSTR szFormatted, SIZE_T cchDest)
{
    PCWSTR dataUnits[5] = { L"B/s", L"KB/s", L"MB/s", L"GB/s", L"TB/s" };
    INT nID = 0;
    while (fSpeed >= 1000)
    {
        nID++;
        fSpeed /= 1024;
    }
    StringCchPrintfW(szFormatted, cchDest, L"%.1f %s", fSpeed, dataUnits[nID]);
    nID = 1 + nID * 2 + (fSpeed >= 500 ? 1 : 0);
    return nID;
}

INT GetSpeedUnit(DOUBLE fSpeed, PWSTR szUnit, SIZE_T cchDest)
{
    PCWSTR dataUnits[5] = { L"B/s", L"KB/s", L"MB/s", L"GB/s", L"TB/s" };
    INT nID = 0;
    while (fSpeed >= 1000)
    {
        nID++;
        fSpeed /= 1024;
    }
    StringCchCopyW(szUnit, cchDest, dataUnits[nID]);
    nID = 1 + nID * 2 + (fSpeed >= 500 ? 1 : 0);
    return nID;
}

UINT GetHourTimeDiff()
{
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    UINT uTimeDiff = (59 - st.wMinute) * 60 * 1000 + (60 - st.wSecond) * 1000 + 1000;
    return uTimeDiff;
}

DWORD SetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uState)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STATE;
    mii.fState = uState;
    SetMenuItemInfoW(hMenu, uItem, bByPosition, &mii); // ERROR_MENU_ITEM_NOT_FOUND
    return 0;
}

UINT GetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STATE;
    GetMenuItemInfoW(hMenu, uItem, bByPosition, &mii);
    return mii.fState;
}

DWORD SetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uType)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_FTYPE;
    mii.fType = uType;
    SetMenuItemInfoW(hMenu, uItem, bByPosition, &mii); // ERROR_MENU_ITEM_NOT_FOUND
    return 0;
}

UINT GetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_FTYPE;
    GetMenuItemInfoW(hMenu, uItem, bByPosition, &mii); // ERROR_MENU_ITEM_NOT_FOUND
    return mii.fType;
}

DWORD GetSystemCapitalFont(PLOGFONTW pLogFont)
{
    NONCLIENTMETRICSW ncMetrics = { 0 };
    ncMetrics.cbSize = sizeof(NONCLIENTMETRICSW);

    // 获取当前系统标题栏的字体
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
        // TODO: 错误处理
    }

    if (hkPersonalize != NULL)
    {
        RegCloseKey(hkPersonalize);
    }
    return bRet;
}

DWORD ExtractRes(UINT uResID, PCWSTR szResType, PCWSTR szFilePath)
{
    DWORD dwErrorCode = ERROR_SUCCESS;

    // 获取 TTF 资源
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    HRSRC hrsrc = FindResourceW(hInstance, MAKEINTRESOURCEW(uResID), szResType);

    if (hrsrc != NULL)
    {
        HGLOBAL hglobal = LoadResource(hInstance, hrsrc);
        PBYTE pData = (PBYTE)LockResource(hglobal);
        DWORD cbData = SizeofResource(hInstance, hrsrc);

        // 写入临时文件
        HANDLE hFile = DefCreateFile(szFilePath, GENERIC_WRITE, CREATE_ALWAYS);
        dwErrorCode = GetLastError();
        if (dwErrorCode == ERROR_SUCCESS || dwErrorCode == ERROR_ALREADY_EXISTS)
        {
            DWORD dwWrittenNum = 0;
            WriteFile(hFile, pData, cbData, &dwWrittenNum, NULL);
        }

        if (hFile != NULL)
        {
            CloseHandle(hFile);
        }
    }
    else
    {
        dwErrorCode = GetLastError();
    }

    return dwErrorCode;
}

PBYTE GetResPointer(UINT uResID, PCWSTR szResType, DWORD* cbData)
{
    // 获取 TTF 资源
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    HRSRC hrsrc = FindResourceW(hInstance, MAKEINTRESOURCEW(uResID), szResType);

    if (hrsrc != NULL)
    {
        HGLOBAL hglobal = LoadResource(hInstance, hrsrc);
        *cbData = SizeofResource(hInstance, hrsrc);
        return (PBYTE)LockResource(hglobal);
    }

    *cbData = 0;
    return NULL;
}
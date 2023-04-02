#include <ds/framework.h>
#include <ds/utils.h>

HANDLE DefCreateThread(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter)
{
    return CreateThread(NULL, 0, lpStartAddress, lpParameter, 0, NULL);
}

HANDLE DefCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwCreationDisposition)
{
    return CreateFileW(lpFileName, dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
}

LONG_PTR GetWndData(HWND hWnd)
{
    return GetWindowLongPtrW(hWnd, GWLP_USERDATA);
}

LONG_PTR SetWndData(HWND hWnd, LONG_PTR dwNewData)
{
    return SetWindowLongPtrW(hWnd, GWLP_USERDATA, dwNewData);
}

LONG_PTR GetDlgData(HWND hDlg)
{
    return GetWindowLongPtrW(hDlg, DWLP_USER);
}

LONG_PTR SetDlgData(HWND hDlg, LONG_PTR dwNewData)
{
    return SetWindowLongPtrW(hDlg, DWLP_USER, dwNewData);
}

LSTATUS RegSetBinValue(HKEY hKey, PCWSTR lpValueName, PBYTE lpData, DWORD cbData)
{
    return RegSetValueExW(hKey, lpValueName, 0, REG_BINARY, lpData, cbData);
}

LSTATUS RegQueryAnyValue(HKEY hKey, PCWSTR lpValueName, PBYTE lpData, PDWORD lpcbData)
{
    return RegQueryValueExW(hKey, lpValueName, NULL, NULL, lpData, lpcbData);
}

LSTATUS RegOpenRunKey(PHKEY phkResult)
{
    return RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, phkResult);
}

LSTATUS RegOpenPersonalizeKey(PHKEY phkResult)
{
    return RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_ALL_ACCESS, phkResult);
}

DOUBLE ClampNum(DOUBLE numVal, DOUBLE minVal, DOUBLE maxVal)
{
    numVal = (numVal >= minVal) ? numVal : minVal;
    numVal = (numVal <= maxVal) ? numVal : maxVal;
    return numVal;
}

DOUBLE AlphaToPercent(BYTE alpha)
{
    return (DOUBLE)((DOUBLE)alpha / 255.0 * 100.0);
}

BYTE PercentToAlpha(DOUBLE percent)
{
    return (BYTE)(percent / 100.0 * 255.0);
}

DWORD SetAppAutoRun(PCWSTR appname)
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
        dwErrorCode = RegSetValueExW(hkRun, appname, 0, REG_SZ, (LPBYTE)szExeFullPath, (DWORD)(sizeof(WCHAR) * (cchPath + 1)));
    }

    if (hkRun != NULL)
    {
        RegCloseKey(hkRun);
    }
    return dwErrorCode;
}

DWORD UnsetAppAutoRun(PCWSTR appname)
{
    DWORD dwErrorCode = ERROR_SUCCESS;
    HKEY hkRun = NULL;

    // 打开启动项  
    dwErrorCode = RegOpenRunKey(&hkRun);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        dwErrorCode = RegDeleteValueW(hkRun, appname);

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

UINT GetMillisecondsToNextHour()
{
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    UINT uTimeDiff = (59 - st.wMinute) * 60 * 1000 + (59 - st.wSecond) * 1000 + (1000 - st.wMilliseconds) + 1000;
    return uTimeDiff;
}

BOOL IsOnTheHour()
{
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    return (st.wMinute == 0);
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

DWORD GetScreenResolution(PSIZE psizeResolution) 
{
    INT cx = GetSystemMetrics(SM_CXSCREEN);
    INT cy = GetSystemMetrics(SM_CYSCREEN);
    if (cx && cy)
    {
        psizeResolution->cx = cx;
        psizeResolution->cy = cy;
        return 0;
    }
    else
    {
        return -1;
    }
}

BOOL IsSystemDarkTheme()
{
    BOOL bRet = TRUE;
    HKEY hkPersonalize = NULL;
    if (RegOpenPersonalizeKey(&hkPersonalize) == ERROR_SUCCESS)
    {
        DWORD dwSystemUsesLightTheme = 0;
        DWORD cbData = sizeof(DWORD);
        RegQueryAnyValue(hkPersonalize, L"SystemUsesLightTheme", (PBYTE)&dwSystemUsesLightTheme, &cbData);
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

UINT GetShellTrayDirection()
{
    UINT uDirection = ABE_BOTTOM;
    APPBARDATA abData = { 0 };
    abData.cbSize = sizeof(APPBARDATA);
    if (SHAppBarMessage(ABM_GETTASKBARPOS, &abData))
    {
        uDirection = abData.uEdge;
    }
    return uDirection;
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

DWORD ConvertPointForResolution(const PSIZE sizeOld, const PPOINT ptOld, const PSIZE sizeNew, PPOINT ptNew)
{
    ptNew->x = ptOld->x * sizeNew->cx / sizeOld->cx;
    ptNew->y = ptOld->y * sizeNew->cy / sizeOld->cy;
    return 0;
}

DWORD CopyPoint(PPOINT ptSrc, PPOINT ptDst)
{
    ptDst->x = ptSrc->x;
    ptDst->y = ptSrc->y;
    return 0;
}

DWORD CopySize(PSIZE sizeSrc, PSIZE sizeDst)
{
    sizeDst->cx = sizeSrc->cx;
    sizeDst->cy = sizeSrc->cy;
    return 0;
}

INT StrAtoW(PCSTR aStr, PWSTR wStr, INT wStrLen)
{
    return MultiByteToWideChar(CP_UTF8, 0, aStr, -1, wStr, wStrLen);
}

INT StrWtoA(PCWSTR wStr, PSTR aStr, INT aStrLen)
{
    return WideCharToMultiByte(CP_UTF8, 0, wStr, -1, aStr, aStrLen, NULL, NULL);
}

// Show error Line and GetLastError
void ShowLastError(PCWSTR func, INT line)
{
    LPWSTR lastErrMsg = NULL;
    DWORD length = 0;
    length = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
        GetLastError(), 0,
        (LPWSTR)&lastErrMsg, 0, NULL
    );

    PWSTR errMsg = new WCHAR[length + 1024]{ 0 };
    StringCchPrintfW(errMsg, length + 1024, L"Func: %s\nLine: %d\nCode: 0x%X\nMsg: %s\n", func, line, GetLastError(), lastErrMsg);
    MessageBoxW(NULL, errMsg, L"GetLastError", MB_ICONERROR);

    LocalFree(lastErrMsg);
    delete[] errMsg;
}

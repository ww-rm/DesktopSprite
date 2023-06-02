#include <ds/framework.h>
#include <ds/utils.h>

FLOAT HighResolutionTimer::GetSeconds()
{
    if (!this->stoped)
    {
        QueryPerformanceCounter(&this->end);
    }
    return (float)(this->end.QuadPart - this->start.QuadPart) / (float)this->freq.QuadPart;
}

FLOAT HighResolutionTimer::GetMilliseconds()
{
    if (!this->stoped)
    {
        QueryPerformanceCounter(&this->end);
    }
    return (float)(1000 * (this->end.QuadPart - this->start.QuadPart)) / (float)this->freq.QuadPart;
}

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

UINT AlphaToPercent(BYTE alpha)
{
    return (UINT)((DOUBLE)alpha / 255.0 * 100.0);
}

BYTE PercentToAlpha(UINT percent)
{
    return (BYTE)((DOUBLE)percent / 100.0 * 255.0);
}

BOOL SetAppAutoRun(PCWSTR appname)
{
    HKEY hkRun = NULL;

    // 打开启动项  
    if (RegOpenRunKey(&hkRun))
    {
        return FALSE;
    }

    // 得到本程序自身的全路径
    WCHAR szExeFullPath[MAX_PATH];
    GetModuleFileNameW(NULL, szExeFullPath, MAX_PATH);
    PathQuoteSpacesW(szExeFullPath); // 补上双引号

    // 得到要写入的数据大小
    size_t cchPath = 0;
    if (FAILED(StringCchLengthW(szExeFullPath, MAX_PATH, &cchPath)) ||
        RegSetValueExW(hkRun, appname, 0, REG_SZ, (LPBYTE)szExeFullPath, (DWORD)(sizeof(WCHAR) * (cchPath + 1))))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
    }

    RegCloseKey(hkRun);
    return TRUE;
}

BOOL UnsetAppAutoRun(PCWSTR appname)
{
    HKEY hkRun = NULL;

    // 打开启动项
    RegOpenRunKey(&hkRun);
    RegDeleteValueW(hkRun, appname);
    RegCloseKey(hkRun);
    return TRUE;
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

BOOL SetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uState)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fState = uState;
    return SetMenuItemInfoW(hMenu, uItem, bByPosition, &mii); // ERROR_MENU_ITEM_NOT_FOUND
}

UINT GetMenuItemState(HMENU hMenu, UINT uItem, BOOL bByPosition)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    GetMenuItemInfoW(hMenu, uItem, bByPosition, &mii);
    return mii.fState;
}

BOOL SetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition, UINT uType)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_FTYPE;
    mii.fType = uType;
    return SetMenuItemInfoW(hMenu, uItem, bByPosition, &mii); // ERROR_MENU_ITEM_NOT_FOUND
}

UINT GetMenuItemType(HMENU hMenu, UINT uItem, BOOL bByPosition)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_FTYPE;
    GetMenuItemInfoW(hMenu, uItem, bByPosition, &mii); // ERROR_MENU_ITEM_NOT_FOUND
    return mii.fType;
}

BOOL GetSystemCapitalFont(PLOGFONTW pLogFont)
{
    NONCLIENTMETRICSW ncMetrics = { 0 };
    ncMetrics.cbSize = sizeof(ncMetrics);

    // 获取当前系统标题栏的字体
    return (BOOL)(SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncMetrics, 0) &&
        CopyMemory(pLogFont, &ncMetrics.lfCaptionFont, sizeof(LOGFONTW)));
}

BOOL GetScreenResolution(PSIZE psizeResolution) 
{
    INT cx = GetSystemMetrics(SM_CXSCREEN);
    INT cy = GetSystemMetrics(SM_CYSCREEN);
    if (cx && cy)
    {
        psizeResolution->cx = cx;
        psizeResolution->cy = cy;
        return TRUE;
    }

    return FALSE;
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
        ShowLastError(__FUNCTIONW__, __LINE__);
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
    abData.cbSize = sizeof(abData);
    if (SHAppBarMessage(ABM_GETTASKBARPOS, &abData))
    {
        uDirection = abData.uEdge;
    }
    return uDirection;
}

BOOL ConvertPointForResolution(const PSIZE sizeOld, const PPOINT ptOld, const PSIZE sizeNew, PPOINT ptNew)
{
    ptNew->x = ptOld->x * sizeNew->cx / sizeOld->cx;
    ptNew->y = ptOld->y * sizeNew->cy / sizeOld->cy;
    return TRUE;
}

void CopyPoint(const POINT* ptSrc, PPOINT ptDst)
{
    ptDst->x = ptSrc->x;
    ptDst->y = ptSrc->y;
}

void CopySize(const SIZE* sizeSrc, PSIZE sizeDst)
{
    sizeDst->cx = sizeSrc->cx;
    sizeDst->cy = sizeSrc->cy;
}

INT StrAtoW(PCSTR aStr, PWSTR wStr, INT wStrLen)
{
    return MultiByteToWideChar(CP_UTF8, 0, aStr, -1, wStr, wStrLen);
}

INT StrWtoA(PCWSTR wStr, PSTR aStr, INT aStrLen)
{
    return WideCharToMultiByte(CP_UTF8, 0, wStr, -1, aStr, aStrLen, NULL, NULL);
}

INT CheckRectContainment(const RECT* rc1, const RECT* rc2)
{
    if (rc1->left >= rc2->left &&
        rc1->top >= rc2->top &&
        rc1->right <= rc2->right &&
        rc1->bottom <= rc2->bottom)
    {
        return -1;
    }
    else if (rc1->left <= rc2->left &&
        rc1->top <= rc2->top &&
        rc1->right >= rc2->right &&
        rc1->bottom >= rc2->bottom)
    {
        return 1;
    }
    return 0;
}

void HighResolutionSleep(DWORD dwMilliseconds)
{
    timeBeginPeriod(1);
    Sleep(dwMilliseconds);
    timeEndPeriod(1);
}

BOOL GetSysDragSize(PSIZE psizeDrag)
{
    INT cx = GetSystemMetrics(SM_CXDRAG);
    INT cy = GetSystemMetrics(SM_CYDRAG);
    if (cx && cy)
    {
        cx = (cx < 0) ? -cx : cx;
        cy = (cy < 0) ? -cy : cy;
        psizeDrag->cx = cx;
        psizeDrag->cy = cy;
        return TRUE;
    }

    return FALSE;
}

INT GetLastInputInterval()
{
    LASTINPUTINFO info = { sizeof(info), 0 };
    GetLastInputInfo(&info);
    return (INT)GetTickCount64() - (INT)info.dwTime;
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
    StringCchPrintfW(errMsg, (size_t)length + 1024, L"Func: %s\nLine: %d\nCode: 0x%X\nMsg: %s\n", func, line, GetLastError(), lastErrMsg);
    MessageBoxW(NULL, errMsg, L"GetLastError", MB_ICONERROR);

    LocalFree(lastErrMsg);
    delete[] errMsg;
}

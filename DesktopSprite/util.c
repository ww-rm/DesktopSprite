#include "framework.h"

#include "util.h"

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

BOOL IsDarkTheme()
{
    return TRUE;
}
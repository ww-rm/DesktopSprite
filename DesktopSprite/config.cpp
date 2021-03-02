#include "framework.h"
#include "util.h"

#include "config.h"

// TODO: ������ע�������
static PCWSTR const REGVAL_SHOWMAINWND              = L"IsFloatWnd";
static PCWSTR const REGVAL_AUTORUN                  = L"IsAutoRun";
static PCWSTR const REGVAL_TIMEAlARM                = L"IsTimeAlarm";
//static PCWSTR const REGVAL_BALLOONICONPATH          = L"BalloonIconPath";
static PCWSTR const REGVAL_INFOSOUND                = L"IsInfoSound";
//static PCWSTR const REGVAL_DARKTHEME                = L"IsDarkTheme";
//static PCWSTR const REGVAL_TRANSPARENCY             = L"Transparency";

static PCWSTR const REGVAL_LASTFLOATPOS             = L"LastFloatPos";


DWORD LoadDefaultConfig(PCFGDATA pCfgData)
{
    // TODO: Ӧ��Ĭ������
    pCfgData->bFloatWnd = FALSE;                            // ���ظ���
    pCfgData->bAutoRun = FALSE;                             // ��ֹ��������
    pCfgData->bTimeAlarm = TRUE;                            // �������㱨ʱ

    //WCHAR szExeFullDir[MAX_PATH] = { 0 };
    //GetModuleFileNameW(NULL, szExeFullDir, MAX_PATH);
    //PathCchRemoveFileSpec(szExeFullDir, MAX_PATH);
    //PathCchCombine(pCfgData->szBalloonIconPath, MAX_PATH, szExeFullDir, L"data\\default_balloonicon.ico");

    pCfgData->bInfoSound = TRUE;                            // ������ʾ����

    // Ĭ��λ������Ļ�� 1/6 ��
    RECT rcScreen = { 0 };
    GetWindowRect(GetDesktopWindow(), &rcScreen);
    pCfgData->ptLastFloatPos.x = rcScreen.right * 5 / 6;
    pCfgData->ptLastFloatPos.y = rcScreen.bottom * 1 / 6;
    return 0;
}

DWORD LoadConfigFromReg(PCFGDATA pCfgData)
{
    LoadDefaultConfig(pCfgData); // ��֤��Ĭ��ֵ

    DWORD dwErrorCode = ERROR_SUCCESS;

    // ��ע�����
    HKEY hkSoftware = NULL;
    HKEY hkApp = NULL;
    DWORD cbData = 0;

    dwErrorCode = RegOpenSoftwareKey(&hkSoftware);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        dwErrorCode = RegOpenKeyExW(hkSoftware, APPNAME, 0, KEY_ALL_ACCESS, &hkApp);
        if (dwErrorCode == ERROR_SUCCESS)
        {
            // TODO: ��ȡ����
            cbData = sizeof(BOOL);
            RegQueryAnyValue(hkApp, REGVAL_SHOWMAINWND, &pCfgData->bFloatWnd, &cbData);
            cbData = sizeof(BOOL);
            RegQueryAnyValue(hkApp, REGVAL_AUTORUN, &pCfgData->bAutoRun, &cbData);
            cbData = sizeof(BOOL);
            RegQueryAnyValue(hkApp, REGVAL_TIMEAlARM, &pCfgData->bTimeAlarm, &cbData);
            //cbData = MAX_PATH;
            //RegQueryValueExW(hkApp, REGVAL_BALLOONICONPATH, 0, NULL, (PBYTE)pCfgData->szBalloonIconPath, &cbData);
            cbData = sizeof(BOOL);
            RegQueryAnyValue(hkApp, REGVAL_INFOSOUND, &pCfgData->bInfoSound, &cbData);

            cbData = sizeof(POINT);
            RegQueryAnyValue(hkApp, REGVAL_LASTFLOATPOS, &pCfgData->ptLastFloatPos, &cbData);
        }
        else if (dwErrorCode != ERROR_FILE_NOT_FOUND)
        {
            // TODO: ������
        }
    }

    // �ر�ע���
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    if (hkSoftware != NULL)
    {
        RegCloseKey(hkSoftware);
    }
    return dwErrorCode;
}

DWORD SaveConfigToReg(PCFGDATA pCfgData)
{
    DWORD dwErrorCode = ERROR_SUCCESS;

    // ��ע�����
    HKEY hkSoftware = NULL;
    HKEY hkApp = NULL;
    DWORD cbData = 0;

    dwErrorCode = RegOpenSoftwareKey(&hkSoftware);
    if (dwErrorCode == ERROR_SUCCESS)
    {
        dwErrorCode = RegCreateKeyExW(hkSoftware, APPNAME, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkApp, NULL);
        if (dwErrorCode == ERROR_SUCCESS)
        {
            // TODO: ����Ӧ������
            RegSetBinValue(hkApp, REGVAL_SHOWMAINWND, &pCfgData->bFloatWnd, sizeof(BOOL));
            RegSetBinValue(hkApp, REGVAL_AUTORUN, &pCfgData->bAutoRun, sizeof(BOOL));
            RegSetBinValue(hkApp, REGVAL_TIMEAlARM, &pCfgData->bTimeAlarm, sizeof(BOOL));
            //SIZE_T cbData = 0;
            //StringCchLengthW(pCfgData->szBalloonIconPath, MAX_PATH, &cbData);
            //RegSetValueExW(hkApp, REGVAL_BALLOONICONPATH, 0, REG_SZ, (PBYTE)pCfgData->szBalloonIconPath, (DWORD)(cbData+1)*sizeof(WCHAR));
            RegSetBinValue(hkApp, REGVAL_INFOSOUND, &pCfgData->bInfoSound, sizeof(BOOL));

            RegSetBinValue(hkApp, REGVAL_LASTFLOATPOS, &pCfgData->ptLastFloatPos, sizeof(POINT));
        }
        else
        {
            // TODO: ������
        }
    }

    // �ر�ע���
    if (hkApp != NULL)
    {
        RegCloseKey(hkApp);
    }
    if (hkSoftware != NULL)
    {
        RegCloseKey(hkSoftware);
    }
    return dwErrorCode;
}

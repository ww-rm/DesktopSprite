#include "framework.h"
#include "perfdata.h"

// �˴���ʼ����ģ���ڲ�ʹ�õı���

// handles and flags
static DWORD  const QUERYINTEREVAL          = 1000;                     // ��ѯʱ����
static HANDLE       hQueryThread            = NULL;                     // �Զ���ѯ�߳̾��
static BOOL         isThreadRun             = FALSE;                    // �����̵߳�����

// ��PDH�йص�����
static PDH_HQUERY   hPdhQuery                               = NULL;                     // PDH��ѯ���
static HCOUNTER     hCnterCpu                               = NULL;                     // cpu������
static HCOUNTER     hCnterMem                               = NULL;                     // �ڴ������
static INT          nNetworkCnter                           = 0;                        // ������
static HCOUNTER     hCnterUploadSpeedList[MAX_COUNTER]      = { 0 };                    // �ϴ��������б�
static HCOUNTER     hCnterDownloadSpeedList[MAX_COUNTER]    = { 0 };                    // ���ؼ������б�

// ��������
static PERFDATA     perfData                = { 0 };                    // ��������
static HANDLE       hDataMutex              = NULL;                     // ���ڷ������ݵĻ�����  


// �Զ����º���, ʹ�����̵߳���
static DWORD QueryPerfData();

// �̺߳���
static DWORD CALLBACK QueryThreadProc(_In_ LPVOID lpParameter);


// ��ʼ�����ܼ���ģ��
DWORD OpenPerfMonitor()
{
    // ��ʼ������, �򿪲�ѯ, ����ȡ�ö�Ӧ�ļ�����

    DWORD dwErrorCode = ERROR_SUCCESS;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;

    // cache buffer
    PWSTR pwsCounterListBuffer = NULL;
    DWORD dwCounterListSize = 0;
    PWSTR pwsInstanceListBuffer = NULL;
    DWORD dwInstanceListSize = 0;
    WCHAR pwsCnterPath[PDH_MAX_COUNTER_PATH];

    // temp value
    DWORD dwTmp = 0;
    PWSTR pwsTmp = NULL;
    size_t pwsTmpLen = 0;

    // �������ݷ��ʻ�����
    hDataMutex = CreateMutexW(NULL, FALSE, NULL);
    if (hDataMutex != NULL)
    {
        // ˢ�»���
        pdhStatus = PdhEnumObjectsW(NULL, NULL, NULL, &dwTmp, PERF_DETAIL_WIZARD, TRUE);
        if (pdhStatus == PDH_MORE_DATA)
        {
            // ��һ����ѯ
            pdhStatus = PdhOpenQueryW(NULL, (DWORD_PTR)NULL, &hPdhQuery);
            if (pdhStatus == ERROR_SUCCESS)
            {
                // ���CPU������
                pdhStatus = PdhAddCounterW(
                    hPdhQuery, L"\\Processor Information(_Total)\\% Processor Utility",
                    (DWORD_PTR)NULL, &hCnterCpu
                );

                // ����ڴ������
                pdhStatus = PdhAddCounterW(
                    hPdhQuery, L"\\Memory\\% Committed Bytes In Use",
                    (DWORD_PTR)NULL, &hCnterMem
                );

                // ��ȡ��������С
                pdhStatus = PdhEnumObjectItemsW(
                    NULL, NULL, L"Network Interface",
                    pwsCounterListBuffer, &dwCounterListSize,
                    pwsInstanceListBuffer, &dwInstanceListSize,
                    PERF_DETAIL_WIZARD, 0
                );
                if (pdhStatus == PDH_MORE_DATA)
                {
                    // allocate buffer
                    pwsCounterListBuffer = (PWSTR)HeapAlloc(
                        GetProcessHeap(), HEAP_ZERO_MEMORY, dwCounterListSize * sizeof(WCHAR)
                    );
                    pwsInstanceListBuffer = (PWSTR)HeapAlloc(
                        GetProcessHeap(), HEAP_ZERO_MEMORY, dwInstanceListSize * sizeof(WCHAR)
                    );
                    // ������е�ʵ������
                    if (pwsCounterListBuffer != NULL && pwsInstanceListBuffer != NULL)
                    {
                        pdhStatus = PdhEnumObjectItemsW(
                            NULL, NULL, L"Network Interface",
                            pwsCounterListBuffer, &dwCounterListSize,
                            pwsInstanceListBuffer, &dwInstanceListSize,
                            PERF_DETAIL_WIZARD, 0
                        );
                        // ���ÿһ��ʵ�����ϴ�/���ؼ�����
                        if (pdhStatus == ERROR_SUCCESS)
                        {
                            pwsTmp = pwsInstanceListBuffer;
                            while (*pwsTmp != (WCHAR)NULL)
                            {
                                OutputDebugStringW(pwsTmp);
                                OutputDebugStringW(L"\n");
                                StringCchPrintfW(
                                    pwsCnterPath, PDH_MAX_COUNTER_PATH,
                                    L"\\Network Interface(%s)\\Bytes Sent/sec", pwsTmp
                                );
                                pdhStatus = PdhAddCounterW(
                                    hPdhQuery, pwsCnterPath, (DWORD_PTR)NULL,
                                    hCnterUploadSpeedList + nNetworkCnter
                                );
                                StringCchPrintfW(
                                    pwsCnterPath, PDH_MAX_COUNTER_PATH,
                                    L"\\Network Interface(%s)\\Bytes Received/sec", pwsTmp
                                );
                                pdhStatus = PdhAddCounterW(
                                    hPdhQuery, pwsCnterPath, (DWORD_PTR)NULL,
                                    hCnterDownloadSpeedList + nNetworkCnter
                                );
                                nNetworkCnter++;
                                StringCchLengthW(pwsTmp, PDH_MAX_INSTANCE_NAME, &pwsTmpLen);
                                pwsTmp += (pwsTmpLen + 1);

                                // ��ֹ���������
                                if (nNetworkCnter >= MAX_COUNTER)
                                {
                                    dwErrorCode = -2;
                                    break;
                                }
                            }

                            // �����߳��Զ���ȡ����
                            isThreadRun = TRUE;
                            hQueryThread = CreateThread(NULL, 0, QueryThreadProc, NULL, 0, NULL);
                            if (hQueryThread == NULL)
                            {
                                isThreadRun = FALSE;
                                dwErrorCode = -1;
                            }
                        }
                    }
                    else
                    {
                        // ����������ʧ��
                        dwErrorCode = ERROR_OUTOFMEMORY;
                    }
                }
            }
            else
            {
                // �򿪲�ѯʧ��
                dwErrorCode = pdhStatus;
            }
        }

        // recycle memory
        if (pwsCounterListBuffer != NULL)
        {
            HeapFree(GetProcessHeap(), 0, pwsCounterListBuffer);
        }
        if (pwsInstanceListBuffer != NULL)
        {
            HeapFree(GetProcessHeap(), 0, pwsInstanceListBuffer);
        }

    }
    else
    {
        dwErrorCode = -1;
    }
    return dwErrorCode;
}

// �������ܼ���ģ��
DWORD ClosePerfMonitor()
{
    if (hQueryThread != NULL)
    {
        isThreadRun = FALSE;
        WaitForSingleObject(hQueryThread, INFINITE);
        CloseHandle(hQueryThread);
    }
    if (hPdhQuery != NULL)
    {
        PdhCloseQuery(hPdhQuery);
    }
    if (hDataMutex != NULL)
    {
        CloseHandle(hDataMutex);
    }
    return ERROR_SUCCESS;
}

DWORD GetPerfData(PPERFDATA pPerfData)
{
    // ������ݷ�����
    WaitForSingleObject(hDataMutex, INFINITE);

    // ��������
    pPerfData->cpuPercent = perfData.cpuPercent;
    pPerfData->memPercent = perfData.memPercent;
    pPerfData->uploadSpeed = perfData.uploadSpeed;
    pPerfData->downloadSpeed = perfData.downloadSpeed;

    // �ͷŷ�����
    ReleaseMutex(hDataMutex);

    return 0;
}

// ���º���, ʹ�����̵߳���
static DWORD QueryPerfData()
{
    DWORD dwErrorCode = ERROR_SUCCESS;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PDH_FMT_COUNTERVALUE pValue = { 0 };

    // ��ѯ
    pdhStatus = PdhCollectQueryData(hPdhQuery);
    if (pdhStatus == ERROR_SUCCESS)
    {
        // ���һС��ʱ��
        Sleep(QUERYINTEREVAL);
        pdhStatus = PdhCollectQueryData(hPdhQuery);
        if (pdhStatus == ERROR_SUCCESS)
        {
            // ������ݷ�����
            WaitForSingleObject(hDataMutex, INFINITE);

            // ���CPU����
            pdhStatus = PdhGetFormattedCounterValue(
                hCnterCpu,
                PDH_FMT_DOUBLE,
                NULL, &pValue
            );
            perfData.cpuPercent = (pValue.doubleValue > 100 ? 100 : pValue.doubleValue);    // ������100%

            // ����ڴ�����
            pdhStatus = PdhGetFormattedCounterValue(
                hCnterMem,
                PDH_FMT_DOUBLE,
                NULL, &pValue
            );
            perfData.memPercent = pValue.doubleValue;

            // ������ϴ��ٶ�
            INT i = 0;
            perfData.uploadSpeed = 0;
            for (i = 0; i <= nNetworkCnter - 1; i++)
            {
                pdhStatus = PdhGetFormattedCounterValue(
                    hCnterUploadSpeedList[i],
                    PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                    NULL, &pValue
                );
                perfData.uploadSpeed += pValue.doubleValue;
            }

            // ����������ٶ�
            perfData.downloadSpeed = 0;
            for (i = 0; i <= nNetworkCnter - 1; i++)
            {
                pdhStatus = PdhGetFormattedCounterValue(
                    hCnterDownloadSpeedList[i],
                    PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                    NULL, &pValue
                );
                perfData.downloadSpeed += pValue.doubleValue;
            }

            // �ͷŷ�����
            ReleaseMutex(hDataMutex);
        }
        else
        {
            dwErrorCode = pdhStatus;
        }
    }
    else
    {
        dwErrorCode = pdhStatus;
    }
    return dwErrorCode;
}

static DWORD CALLBACK QueryThreadProc(_In_ LPVOID lpParameter)
{
    DWORD dwErrorCode = EXIT_SUCCESS;
    while (isThreadRun)
    {
        QueryPerfData();
        Sleep(1);
    }
    return dwErrorCode;
}
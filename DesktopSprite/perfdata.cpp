#include "framework.h"
#include "perfdata.h"

// 此处开始的是模块内部使用的变量

// handles and flags
static DWORD  const QUERYINTEREVAL          = 1000;                     // 查询时间间隔
static HANDLE       hQueryThread            = NULL;                     // 自动查询线程句柄
static BOOL         isThreadRun             = FALSE;                    // 控制线程的运行

// 与PDH有关的数据
static PDH_HQUERY   hPdhQuery                               = NULL;                     // PDH查询句柄
static HCOUNTER     hCnterCpu                               = NULL;                     // cpu计数器
static HCOUNTER     hCnterMem                               = NULL;                     // 内存计数器
static INT          nNetworkCnter                           = 0;                        // 网卡数
static HCOUNTER     hCnterUploadSpeedList[MAX_COUNTER]      = { 0 };                    // 上传计数器列表
static HCOUNTER     hCnterDownloadSpeedList[MAX_COUNTER]    = { 0 };                    // 下载计数器列表

// 性能数据
static PERFDATA     perfData                = { 0 };                    // 性能数据
static HANDLE       hDataMutex              = NULL;                     // 用于访问数据的互斥锁  


// 自动更新函数, 使用子线程调用
static DWORD QueryPerfData();

// 线程函数
static DWORD CALLBACK QueryThreadProc(_In_ LPVOID lpParameter);


// 初始化性能监视模块
DWORD OpenPerfMonitor()
{
    // 初始化数据, 打开查询, 并且取得对应的计数器

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

    // 创建数据访问互斥锁
    hDataMutex = CreateMutexW(NULL, FALSE, NULL);
    if (hDataMutex != NULL)
    {
        // 刷新缓存
        pdhStatus = PdhEnumObjectsW(NULL, NULL, NULL, &dwTmp, PERF_DETAIL_WIZARD, TRUE);
        if (pdhStatus == PDH_MORE_DATA)
        {
            // 打开一个查询
            pdhStatus = PdhOpenQueryW(NULL, (DWORD_PTR)NULL, &hPdhQuery);
            if (pdhStatus == ERROR_SUCCESS)
            {
                // 添加CPU计数器
                pdhStatus = PdhAddCounterW(
                    hPdhQuery, L"\\Processor Information(_Total)\\% Processor Utility",
                    (DWORD_PTR)NULL, &hCnterCpu
                );

                // 添加内存计数器
                pdhStatus = PdhAddCounterW(
                    hPdhQuery, L"\\Memory\\% Committed Bytes In Use",
                    (DWORD_PTR)NULL, &hCnterMem
                );

                // 获取缓冲区大小
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
                    // 获得所有的实例名称
                    if (pwsCounterListBuffer != NULL && pwsInstanceListBuffer != NULL)
                    {
                        pdhStatus = PdhEnumObjectItemsW(
                            NULL, NULL, L"Network Interface",
                            pwsCounterListBuffer, &dwCounterListSize,
                            pwsInstanceListBuffer, &dwInstanceListSize,
                            PERF_DETAIL_WIZARD, 0
                        );
                        // 添加每一个实例的上传/下载计数器
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

                                // 防止缓冲区溢出
                                if (nNetworkCnter >= MAX_COUNTER)
                                {
                                    dwErrorCode = -2;
                                    break;
                                }
                            }

                            // 打开子线程自动获取数据
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
                        // 缓冲区分配失败
                        dwErrorCode = ERROR_OUTOFMEMORY;
                    }
                }
            }
            else
            {
                // 打开查询失败
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

// 清理性能监视模块
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
    // 获得数据访问锁
    WaitForSingleObject(hDataMutex, INFINITE);

    // 拷贝数据
    pPerfData->cpuPercent = perfData.cpuPercent;
    pPerfData->memPercent = perfData.memPercent;
    pPerfData->uploadSpeed = perfData.uploadSpeed;
    pPerfData->downloadSpeed = perfData.downloadSpeed;

    // 释放访问锁
    ReleaseMutex(hDataMutex);

    return 0;
}

// 更新函数, 使用子线程调用
static DWORD QueryPerfData()
{
    DWORD dwErrorCode = ERROR_SUCCESS;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PDH_FMT_COUNTERVALUE pValue = { 0 };

    // 查询
    pdhStatus = PdhCollectQueryData(hPdhQuery);
    if (pdhStatus == ERROR_SUCCESS)
    {
        // 间隔一小段时间
        Sleep(QUERYINTEREVAL);
        pdhStatus = PdhCollectQueryData(hPdhQuery);
        if (pdhStatus == ERROR_SUCCESS)
        {
            // 获得数据访问锁
            WaitForSingleObject(hDataMutex, INFINITE);

            // 获得CPU数据
            pdhStatus = PdhGetFormattedCounterValue(
                hCnterCpu,
                PDH_FMT_DOUBLE,
                NULL, &pValue
            );
            perfData.cpuPercent = (pValue.doubleValue > 100 ? 100 : pValue.doubleValue);    // 不高于100%

            // 获得内存数据
            pdhStatus = PdhGetFormattedCounterValue(
                hCnterMem,
                PDH_FMT_DOUBLE,
                NULL, &pValue
            );
            perfData.memPercent = pValue.doubleValue;

            // 获得总上传速度
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

            // 获得总下载速度
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

            // 释放访问锁
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
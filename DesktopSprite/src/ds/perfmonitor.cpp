#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/perfmonitor.h>

PerfMonitor::PerfMonitor(DWORD queryInterval): queryInterval(queryInterval)
{
    // 创建数据访问互斥锁
    this->hDataMutex = CreateMutexW(NULL, FALSE, NULL);
    if (!this->hDataMutex)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }
}

PerfMonitor::~PerfMonitor()
{
    if (!CloseHandle(hDataMutex))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
    }
}

BOOL PerfMonitor::Start()
{
    return (BOOL)(this->OpenQuery() && this->AddCounter() && this->StartThread());
}

BOOL PerfMonitor::Stop()
{
    this->isThreadRun = FALSE;
    if (WaitForSingleObject(this->hQueryThread, INFINITE) ||
        !CloseHandle(this->hQueryThread) ||
        PdhCloseQuery(hPdhQuery))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }
    return TRUE;
}

BOOL PerfMonitor::GetPerfData(PERFDATA* pPerfData)
{
    // 获得数据访问锁
    if (WaitForSingleObject(this->hDataMutex, INFINITE))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    // 拷贝数据
    pPerfData->cpuPercent = this->perfData.cpuPercent;
    pPerfData->memPercent = this->perfData.memPercent;
    pPerfData->uploadSpeed = this->perfData.uploadSpeed;
    pPerfData->downloadSpeed = this->perfData.downloadSpeed;

    // 释放访问锁
    if (!ReleaseMutex(this->hDataMutex))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    return TRUE;
}

BOOL PerfMonitor::OpenQuery()
{
    // 刷新缓存
    DWORD dwTmp = 0;
    if (PdhEnumObjectsW(NULL, NULL, NULL, &dwTmp, PERF_DETAIL_WIZARD, TRUE) != PDH_MORE_DATA)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    // 打开一个查询
    if (PdhOpenQueryW(NULL, (DWORD_PTR)NULL, &this->hPdhQuery))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    return TRUE;
}

BOOL PerfMonitor::AddCounter()
{
    // 添加CPU计数器和内存计数器
    if (PdhAddCounterW(this->hPdhQuery, L"\\Processor Information(_Total)\\% Processor Utility", (DWORD_PTR)NULL, &this->hCnterCpu) ||
        PdhAddCounterW(this->hPdhQuery, L"\\Memory\\% Committed Bytes In Use", (DWORD_PTR)NULL, &this->hCnterMem))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    // cache buffer
    PDH_HCOUNTER hCounter = NULL;
    PWSTR pwsCounterListBuffer = NULL;
    DWORD dwCounterListSize = 0;
    PWSTR pwsInstanceListBuffer = NULL;
    DWORD dwInstanceListSize = 0;

    PWSTR pwsTmp = NULL;
    size_t pwsTmpLen = 0;
    WCHAR pwsCnterPath[PDH_MAX_COUNTER_PATH];

    BOOL ret = FALSE; // 用作返回值

    // 获取缓冲区大小
    if (PdhEnumObjectItemsW(
        NULL, NULL, L"Network Interface",
        NULL, &dwCounterListSize,
        NULL, &dwInstanceListSize,
        PERF_DETAIL_WIZARD, 0
    ) != PDH_MORE_DATA)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }

    // allocate buffer
    pwsCounterListBuffer = new WCHAR[dwCounterListSize];
    pwsInstanceListBuffer = new WCHAR[dwInstanceListSize];

    // 获得所有的实例名称
    if (PdhEnumObjectItemsW(
        NULL, NULL, L"Network Interface",
        pwsCounterListBuffer, &dwCounterListSize,
        pwsInstanceListBuffer, &dwInstanceListSize,
        PERF_DETAIL_WIZARD, 0
    ))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        goto RETURN_FALSE;
    }

    // 添加每一个实例的上传/下载计数器
    pwsTmp = pwsInstanceListBuffer;
    while (*pwsTmp != (WCHAR)NULL)
    {
        OutputDebugStringW(pwsTmp);
        OutputDebugStringW(L"\n");

        if (FAILED(StringCchPrintfW(pwsCnterPath, PDH_MAX_COUNTER_PATH, L"\\Network Interface(%s)\\Bytes Sent/sec", pwsTmp)) ||
            PdhAddCounterW(this->hPdhQuery, pwsCnterPath, (DWORD_PTR)NULL, &hCounter))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            goto RETURN_FALSE;
        }
        this->hCounterUpSpeedList.push_back(hCounter);

        if (FAILED(StringCchPrintfW(pwsCnterPath, PDH_MAX_COUNTER_PATH, L"\\Network Interface(%s)\\Bytes Received/sec", pwsTmp)) ||
            PdhAddCounterW(this->hPdhQuery, pwsCnterPath, (DWORD_PTR)NULL, &hCounter))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            goto RETURN_FALSE;
        }
        this->hCounterDownSpeedList.push_back(hCounter);

        if (FAILED(StringCchLengthW(pwsTmp, PDH_MAX_INSTANCE_NAME, &pwsTmpLen)))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            goto RETURN_FALSE;
        }
        pwsTmp += (pwsTmpLen + 1);
    }


    ret = TRUE; // 如果顺序执行到这里则是成功, 返回 TRUE
RETURN_FALSE:
    delete[] pwsCounterListBuffer;
    delete[] pwsInstanceListBuffer;
    return ret;
}

BOOL PerfMonitor::StartThread()
{
    // 打开子线程自动获取数据
    this->isThreadRun = TRUE;
    this->hQueryThread = CreateThread(NULL, 0, PerfMonitor::QueryThreadProc, (LPVOID)this, 0, NULL);
    if (!this->hQueryThread)
    {
        this->isThreadRun = FALSE;
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }
    return TRUE;
}

DWORD CALLBACK PerfMonitor::QueryThreadProc(_In_ LPVOID lpParameter)
{
    PerfMonitor* this_ = (PerfMonitor*)lpParameter;
    while (this_->isThreadRun)
    {
        this_->QueryPerfData();
        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL PerfMonitor::QueryPerfData()
{
    PDH_FMT_COUNTERVALUE pValue = { 0 };

    // 查询
    if (PdhCollectQueryData(this->hPdhQuery))
    {
        return FALSE;
    }

    // 间隔一小段时间
    Sleep(this->queryInterval);
    if (PdhCollectQueryData(this->hPdhQuery))
    {
        return FALSE;
    }

    // 获得数据访问锁
    if (WaitForSingleObject(this->hDataMutex, INFINITE))
    {
        return FALSE;
    }

    // 获得CPU数据
    if (!PdhGetFormattedCounterValue(this->hCnterCpu, PDH_FMT_DOUBLE, NULL, &pValue))
    {
        this->perfData.cpuPercent = min(pValue.doubleValue, 100); // 不高于100%
    }

    // 获得内存数据
    if (!PdhGetFormattedCounterValue(this->hCnterMem, PDH_FMT_DOUBLE, NULL, &pValue))
    {
        this->perfData.memPercent = pValue.doubleValue;
    }

    // 获得总上传速度
    this->perfData.uploadSpeed = 0;
    for (auto item = this->hCounterUpSpeedList.begin(); item != this->hCounterUpSpeedList.end(); item++)
    {
        if (!PdhGetFormattedCounterValue(*item, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, NULL, &pValue))
        {
            perfData.uploadSpeed += pValue.doubleValue;
        }
    }

    // 获得总下载速度
    this->perfData.downloadSpeed = 0;
    for (auto item = this->hCounterDownSpeedList.begin(); item != this->hCounterDownSpeedList.end(); item++)
    {
        if (!PdhGetFormattedCounterValue(*item, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, NULL, &pValue))
        {
            perfData.downloadSpeed += pValue.doubleValue;
        }
    }

    // 释放访问锁
    if (!ReleaseMutex(this->hDataMutex))
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
    }

    return TRUE;
}

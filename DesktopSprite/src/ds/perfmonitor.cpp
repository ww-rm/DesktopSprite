#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/perfmonitor.h>

PerfMonitor::PerfMonitor(DWORD queryInterval)
{
    // 初始化数据, 打开查询, 并且取得对应的计数器
    this->queryInterval = queryInterval;

    DWORD dwErrorCode = ERROR_SUCCESS;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;

    // 创建数据访问互斥锁
    this->hDataMutex = CreateMutexW(NULL, FALSE, NULL);
    if (!this->hDataMutex)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // 刷新缓存
    DWORD dwTmp = 0;
    pdhStatus = PdhEnumObjectsW(NULL, NULL, NULL, &dwTmp, PERF_DETAIL_WIZARD, TRUE);
    if (pdhStatus != PDH_MORE_DATA)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // 打开一个查询
    pdhStatus = PdhOpenQueryW(NULL, (DWORD_PTR)NULL, &this->hPdhQuery);
    if (pdhStatus != ERROR_SUCCESS)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // 添加CPU计数器
    pdhStatus = PdhAddCounterW(this->hPdhQuery, L"\\Processor Information(_Total)\\% Processor Utility", (DWORD_PTR)NULL, &this->hCnterCpu);

    // 添加内存计数器
    pdhStatus = PdhAddCounterW(this->hPdhQuery, L"\\Memory\\% Committed Bytes In Use", (DWORD_PTR)NULL, &this->hCnterMem);

    // cache buffer
    PDH_HCOUNTER hCounter = NULL;
    PWSTR pwsCounterListBuffer = NULL;
    DWORD dwCounterListSize = 0;
    PWSTR pwsInstanceListBuffer = NULL;
    DWORD dwInstanceListSize = 0;

    PWSTR pwsTmp = NULL;
    size_t pwsTmpLen = 0;
    WCHAR pwsCnterPath[PDH_MAX_COUNTER_PATH];

    // 获取缓冲区大小
    pdhStatus = PdhEnumObjectItemsW(
        NULL, NULL, L"Network Interface",
        NULL, &dwCounterListSize,
        NULL, &dwInstanceListSize,
        PERF_DETAIL_WIZARD, 0
    );
    if (pdhStatus != PDH_MORE_DATA)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // allocate buffer
    pwsCounterListBuffer = new WCHAR[dwCounterListSize];
    pwsInstanceListBuffer = new WCHAR[dwInstanceListSize];

    // 获得所有的实例名称
    pdhStatus = PdhEnumObjectItemsW(
        NULL, NULL, L"Network Interface",
        pwsCounterListBuffer, &dwCounterListSize,
        pwsInstanceListBuffer, &dwInstanceListSize,
        PERF_DETAIL_WIZARD, 0
    );
    if (pdhStatus != ERROR_SUCCESS)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // 添加每一个实例的上传/下载计数器
    pwsTmp = pwsInstanceListBuffer;
    while (*pwsTmp != (WCHAR)NULL)
    {
        OutputDebugStringW(pwsTmp);
        OutputDebugStringW(L"\n");

        StringCchPrintfW(pwsCnterPath, PDH_MAX_COUNTER_PATH, L"\\Network Interface(%s)\\Bytes Sent/sec", pwsTmp);
        pdhStatus = PdhAddCounterW(this->hPdhQuery, pwsCnterPath, (DWORD_PTR)NULL, &hCounter);
        this->hCounterUpSpeedList.push_back(hCounter);

        StringCchPrintfW(pwsCnterPath, PDH_MAX_COUNTER_PATH, L"\\Network Interface(%s)\\Bytes Received/sec", pwsTmp);
        pdhStatus = PdhAddCounterW(this->hPdhQuery, pwsCnterPath, (DWORD_PTR)NULL, &hCounter);
        this->hCounterDownSpeedList.push_back(hCounter);

        StringCchLengthW(pwsTmp, PDH_MAX_INSTANCE_NAME, &pwsTmpLen);
        pwsTmp += (pwsTmpLen + 1);
    }

    // 打开子线程自动获取数据
    this->isThreadRun = TRUE;
    this->hQueryThread = CreateThread(NULL, 0, PerfMonitor::QueryThreadProc, (LPVOID)this, 0, NULL);
    if (!this->hQueryThread)
    {
        this->isThreadRun = FALSE;
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    delete[] pwsCounterListBuffer;
    delete[] pwsInstanceListBuffer;
}

PerfMonitor::~PerfMonitor()
{
    this->isThreadRun = FALSE;
    WaitForSingleObject(this->hQueryThread, INFINITE);
    CloseHandle(this->hQueryThread);

    PdhCloseQuery(hPdhQuery);

    CloseHandle(hDataMutex);
}

DWORD PerfMonitor::GetPerfData(PPERFDATA pPerfData)
{
    // 获得数据访问锁
    WaitForSingleObject(this->hDataMutex, INFINITE);

    // 拷贝数据
    pPerfData->cpuPercent = this->perfData.cpuPercent;
    pPerfData->memPercent = this->perfData.memPercent;
    pPerfData->uploadSpeed = this->perfData.uploadSpeed;
    pPerfData->downloadSpeed = this->perfData.downloadSpeed;

    // 释放访问锁
    ReleaseMutex(this->hDataMutex);

    return 0;
}

DWORD CALLBACK PerfMonitor::QueryThreadProc(_In_ LPVOID lpParameter)
{
    PerfMonitor* this_ = (PerfMonitor*)lpParameter;
    DWORD dwErrorCode = EXIT_SUCCESS;
    while (this_->IsThreadRun())
    {
        this_->QueryPerfData();
        Sleep(1);
    }
    return dwErrorCode;
}

PDH_STATUS PerfMonitor::QueryPerfData()
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PDH_FMT_COUNTERVALUE pValue = { 0 };

    // 查询
    pdhStatus = PdhCollectQueryData(this->hPdhQuery);
    if (pdhStatus != ERROR_SUCCESS)
    {
        return pdhStatus;
    }

    // 间隔一小段时间
    Sleep(this->queryInterval);
    pdhStatus = PdhCollectQueryData(this->hPdhQuery);
    if (pdhStatus != ERROR_SUCCESS)
    {
        return pdhStatus;
    }

    // 获得数据访问锁
    WaitForSingleObject(this->hDataMutex, INFINITE);

    // 获得CPU数据
    pdhStatus = PdhGetFormattedCounterValue(this->hCnterCpu, PDH_FMT_DOUBLE, NULL, &pValue);
    this->perfData.cpuPercent = min(pValue.doubleValue, 100);    // 不高于100%

    // 获得内存数据
    pdhStatus = PdhGetFormattedCounterValue(this->hCnterMem, PDH_FMT_DOUBLE, NULL, &pValue);
    this->perfData.memPercent = pValue.doubleValue;

    // 获得总上传速度
    this->perfData.uploadSpeed = 0;
    for (auto item = this->hCounterUpSpeedList.begin(); item != this->hCounterUpSpeedList.end(); item++)
    {
        pdhStatus = PdhGetFormattedCounterValue(*item, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, NULL, &pValue);
        perfData.uploadSpeed += pValue.doubleValue;
    }

    // 获得总下载速度
    this->perfData.downloadSpeed = 0;
    for (auto item = this->hCounterDownSpeedList.begin(); item != this->hCounterDownSpeedList.end(); item++)
    {
        pdhStatus = PdhGetFormattedCounterValue(*item, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, NULL, &pValue);
        perfData.downloadSpeed += pValue.doubleValue;
    }

    // 释放访问锁
    ReleaseMutex(this->hDataMutex);

    return pdhStatus;
}

BOOL PerfMonitor::IsThreadRun() const
{
    return this->isThreadRun;
}

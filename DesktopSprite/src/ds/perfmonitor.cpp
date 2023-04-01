#include <ds/framework.h>
#include <ds/utils.h>
#include <ds/perfmonitor.h>

PerfMonitor::PerfMonitor(DWORD queryInterval)
{
    // ��ʼ������, �򿪲�ѯ, ����ȡ�ö�Ӧ�ļ�����
    this->queryInterval = queryInterval;

    DWORD dwErrorCode = ERROR_SUCCESS;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;

    // �������ݷ��ʻ�����
    this->hDataMutex = CreateMutexW(NULL, FALSE, NULL);
    if (!this->hDataMutex)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // ˢ�»���
    DWORD dwTmp = 0;
    pdhStatus = PdhEnumObjectsW(NULL, NULL, NULL, &dwTmp, PERF_DETAIL_WIZARD, TRUE);
    if (pdhStatus != PDH_MORE_DATA)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // ��һ����ѯ
    pdhStatus = PdhOpenQueryW(NULL, (DWORD_PTR)NULL, &this->hPdhQuery);
    if (pdhStatus != ERROR_SUCCESS)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // ���CPU������
    pdhStatus = PdhAddCounterW(this->hPdhQuery, L"\\Processor Information(_Total)\\% Processor Utility", (DWORD_PTR)NULL, &this->hCnterCpu);

    // ����ڴ������
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

    // ��ȡ��������С
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

    // ������е�ʵ������
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

    // ���ÿһ��ʵ�����ϴ�/���ؼ�����
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

    // �����߳��Զ���ȡ����
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
    // ������ݷ�����
    WaitForSingleObject(this->hDataMutex, INFINITE);

    // ��������
    pPerfData->cpuPercent = this->perfData.cpuPercent;
    pPerfData->memPercent = this->perfData.memPercent;
    pPerfData->uploadSpeed = this->perfData.uploadSpeed;
    pPerfData->downloadSpeed = this->perfData.downloadSpeed;

    // �ͷŷ�����
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

    // ��ѯ
    pdhStatus = PdhCollectQueryData(this->hPdhQuery);
    if (pdhStatus != ERROR_SUCCESS)
    {
        return pdhStatus;
    }

    // ���һС��ʱ��
    Sleep(this->queryInterval);
    pdhStatus = PdhCollectQueryData(this->hPdhQuery);
    if (pdhStatus != ERROR_SUCCESS)
    {
        return pdhStatus;
    }

    // ������ݷ�����
    WaitForSingleObject(this->hDataMutex, INFINITE);

    // ���CPU����
    pdhStatus = PdhGetFormattedCounterValue(this->hCnterCpu, PDH_FMT_DOUBLE, NULL, &pValue);
    this->perfData.cpuPercent = min(pValue.doubleValue, 100);    // ������100%

    // ����ڴ�����
    pdhStatus = PdhGetFormattedCounterValue(this->hCnterMem, PDH_FMT_DOUBLE, NULL, &pValue);
    this->perfData.memPercent = pValue.doubleValue;

    // ������ϴ��ٶ�
    this->perfData.uploadSpeed = 0;
    for (auto item = this->hCounterUpSpeedList.begin(); item != this->hCounterUpSpeedList.end(); item++)
    {
        pdhStatus = PdhGetFormattedCounterValue(*item, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, NULL, &pValue);
        perfData.uploadSpeed += pValue.doubleValue;
    }

    // ����������ٶ�
    this->perfData.downloadSpeed = 0;
    for (auto item = this->hCounterDownSpeedList.begin(); item != this->hCounterDownSpeedList.end(); item++)
    {
        pdhStatus = PdhGetFormattedCounterValue(*item, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, NULL, &pValue);
        perfData.downloadSpeed += pValue.doubleValue;
    }

    // �ͷŷ�����
    ReleaseMutex(this->hDataMutex);

    return pdhStatus;
}

BOOL PerfMonitor::IsThreadRun() const
{
    return this->isThreadRun;
}

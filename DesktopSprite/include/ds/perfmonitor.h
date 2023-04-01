#pragma once
#ifndef DS_PERFMONITOR_H
#define DS_PERFMONITOR_H

#include <ds/framework.h>

// 性能模块数据
typedef struct _PERFDATA
{
    DOUBLE  cpuPercent;                         // CPU使用百分比
    DOUBLE  memPercent;                         // 内存使用百分比
    DOUBLE  uploadSpeed;                        // 上传速度字节
    DOUBLE  downloadSpeed;                      // 下载速度字节
}PERFDATA, * PPERFDATA;

class PerfMonitor
{
private:
    DWORD                   queryInterval = 1000;                  // 查询时间间隔
    HANDLE                  hQueryThread = NULL;                   // 自动查询线程句柄
    BOOL                    isThreadRun = FALSE;                   // 控制线程的运行
    PERFDATA                perfData = { 0 };                      // 性能数据
    HANDLE                  hDataMutex = NULL;                     // 用于访问数据的互斥锁

    PDH_HQUERY              hPdhQuery = NULL;                      // PDH查询句柄
    PDH_HCOUNTER            hCnterCpu = NULL;                      // cpu计数器
    PDH_HCOUNTER            hCnterMem = NULL;                      // 内存计数器
    std::list<PDH_HCOUNTER> hCounterUpSpeedList;                   // 上传计数器列表
    std::list<PDH_HCOUNTER> hCounterDownSpeedList;                 // 下载计数器列表 

public:
    PerfMonitor(DWORD queryInterval = 1000);
    ~PerfMonitor();

    // 获得性能数据
    DWORD GetPerfData(PPERFDATA pPerfData);

private:
    // 线程函数
    static DWORD CALLBACK QueryThreadProc(_In_ LPVOID lpParameter);

public:
    // 自动更新函数, 使用子线程调用
    PDH_STATUS QueryPerfData();
    BOOL IsThreadRun() const;
};

#endif // !DS_PERFMONITOR_H

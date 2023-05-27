#pragma once
#ifndef DS_PERFMONITOR_H
#define DS_PERFMONITOR_H

#include <ds/framework.h>

namespace PerfMonitor {

    // 性能模块数据
    typedef struct _PERFDATA
    {
        DOUBLE  cpuPercent;                         // CPU使用百分比
        DOUBLE  memPercent;                         // 内存使用百分比
        DOUBLE  uploadSpeed;                        // 上传速度字节
        DOUBLE  downloadSpeed;                      // 下载速度字节
    } PERFDATA;

    class PerfMonitor
    {
    private:
        DWORD                   queryInterval = 1000;                  // 查询时间间隔
        HANDLE                  hQueryEvent = NULL;                    // 控制线程的运行
        HANDLE                  hQueryThread = NULL;                   // 自动查询线程句柄
        PERFDATA                perfData = { 0 };                      // 性能数据
        HANDLE                  hDataMutex = NULL;                     // 用于访问数据的互斥锁

        PDH_HQUERY              hPdhQuery = NULL;                      // PDH查询句柄
        PDH_HCOUNTER            hCnterCpu = NULL;                      // cpu计数器
        PDH_HCOUNTER            hCnterMem = NULL;                      // 内存计数器
        std::list<PDH_HCOUNTER> hCounterUpSpeedList;                   // 上传计数器列表
        std::list<PDH_HCOUNTER> hCounterDownSpeedList;                 // 下载计数器列表 

        std::map<HWND, INT>     registeredMsg;                         // 已注册的窗口和对应的通知消息

    public:
        PerfMonitor(DWORD queryInterval = 1000);
        ~PerfMonitor();

        // 启动监视器
        BOOL Start();

        // 停止监视器
        BOOL Stop();

        // 注册更新消息
        BOOL RegisterMessage(HWND hWnd, UINT wndMsg);

        // 获得性能数据
        BOOL GetPerfData(PERFDATA* pPerfData);

    private:
        // 线程函数
        static DWORD CALLBACK QueryThreadProc(_In_ LPVOID lpParameter);

        DWORD QueryThread();

        // 打开查询
        BOOL OpenQuery();

        // 添加计数器
        BOOL AddCounter();

        // 开启线程
        BOOL StartThread();

        // 自动更新函数, 使用子线程调用
        BOOL QueryPerfData();
    };

    // global functions

    BOOL InitializePerfMonitor(DWORD queryInterval = 1000);

    BOOL UninitializePerfMonitor();

    BOOL RegisterMessage(HWND hWnd, UINT wndMsg);

    BOOL GetPerfData(PERFDATA* pPerfData);
}

#endif // !DS_PERFMONITOR_H

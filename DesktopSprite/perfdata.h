#pragma once
#include "framework.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAX_COUNTER         32          // 计数器缓冲区列表最大值

    typedef struct _PERFDATA
    {
        // 性能模块数据
        DOUBLE  cpuPercent;                         // CPU使用百分比
        DOUBLE  memPercent;                         // 内存使用百分比
        DOUBLE  uploadSpeed;                        // 上传速度字节
        DOUBLE  downloadSpeed;                      // 下载速度字节
    }PERFDATA, * PPERFDATA;

    // 初始化性能监视模块
    DWORD OpenPerfMonitor();

    // 清理性能监视模块
    DWORD ClosePerfMonitor();

    // 获得性能数据
    DWORD GetPerfData(PPERFDATA pPerfData);

#ifdef __cplusplus
}
#endif // __cplusplus
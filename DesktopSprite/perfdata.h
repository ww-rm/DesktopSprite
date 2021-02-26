#pragma once
#include "framework.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MAX_COUNTER         32          // �������������б����ֵ

    typedef struct _PERFDATA
    {
        // ����ģ������
        DOUBLE  cpuPercent;                         // CPUʹ�ðٷֱ�
        DOUBLE  memPercent;                         // �ڴ�ʹ�ðٷֱ�
        DOUBLE  uploadSpeed;                        // �ϴ��ٶ��ֽ�
        DOUBLE  downloadSpeed;                      // �����ٶ��ֽ�
    }PERFDATA, * PPERFDATA;

    // ��ʼ�����ܼ���ģ��
    DWORD OpenPerfMonitor();

    // �������ܼ���ģ��
    DWORD ClosePerfMonitor();

    // �����������
    DWORD GetPerfData(PPERFDATA pPerfData);

#ifdef __cplusplus
}
#endif // __cplusplus
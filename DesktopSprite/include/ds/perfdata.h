#pragma once
#include <ds/framework.h>

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

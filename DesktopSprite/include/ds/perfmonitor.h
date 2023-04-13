#pragma once
#ifndef DS_PERFMONITOR_H
#define DS_PERFMONITOR_H

#include <ds/framework.h>

// ����ģ������
typedef struct _PERFDATA
{
    DOUBLE  cpuPercent;                         // CPUʹ�ðٷֱ�
    DOUBLE  memPercent;                         // �ڴ�ʹ�ðٷֱ�
    DOUBLE  uploadSpeed;                        // �ϴ��ٶ��ֽ�
    DOUBLE  downloadSpeed;                      // �����ٶ��ֽ�
}PERFDATA;

class PerfMonitor
{
private:
    DWORD                   queryInterval = 1000;                  // ��ѯʱ����
    HANDLE                  hQueryThread = NULL;                   // �Զ���ѯ�߳̾��
    BOOL                    isThreadRun = FALSE;                   // �����̵߳�����
    PERFDATA                perfData = { 0 };                      // ��������
    HANDLE                  hDataMutex = NULL;                     // ���ڷ������ݵĻ�����

    PDH_HQUERY              hPdhQuery = NULL;                      // PDH��ѯ���
    PDH_HCOUNTER            hCnterCpu = NULL;                      // cpu������
    PDH_HCOUNTER            hCnterMem = NULL;                      // �ڴ������
    std::list<PDH_HCOUNTER> hCounterUpSpeedList;                   // �ϴ��������б�
    std::list<PDH_HCOUNTER> hCounterDownSpeedList;                 // ���ؼ������б� 

public:
    PerfMonitor(DWORD queryInterval = 1000);
    ~PerfMonitor();

    // ����������
    BOOL Start();

    // ֹͣ������
    BOOL Stop();

    // �����������
    BOOL GetPerfData(PERFDATA* pPerfData);

private:
    // �̺߳���
    static DWORD CALLBACK QueryThreadProc(_In_ LPVOID lpParameter);

    // �򿪲�ѯ
    BOOL OpenQuery();

    // ��Ӽ�����
    BOOL AddCounter();

    // �����߳�
    BOOL StartThread();

    // �Զ����º���, ʹ�����̵߳���
    BOOL QueryPerfData();
};

#endif // !DS_PERFMONITOR_H

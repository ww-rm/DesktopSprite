#pragma once
#ifndef DS_PERFMONITOR_H
#define DS_PERFMONITOR_H

#include <ds/framework.h>

namespace PerfMonitor {

    // ����ģ������
    typedef struct _PERFDATA
    {
        DOUBLE  cpuPercent;                         // CPUʹ�ðٷֱ�
        DOUBLE  memPercent;                         // �ڴ�ʹ�ðٷֱ�
        DOUBLE  uploadSpeed;                        // �ϴ��ٶ��ֽ�
        DOUBLE  downloadSpeed;                      // �����ٶ��ֽ�
    } PERFDATA;

    class PerfMonitor
    {
    private:
        DWORD                   queryInterval = 1000;                  // ��ѯʱ����
        HANDLE                  hQueryEvent = NULL;                    // �����̵߳�����
        HANDLE                  hQueryThread = NULL;                   // �Զ���ѯ�߳̾��
        PERFDATA                perfData = { 0 };                      // ��������
        HANDLE                  hDataMutex = NULL;                     // ���ڷ������ݵĻ�����

        PDH_HQUERY              hPdhQuery = NULL;                      // PDH��ѯ���
        PDH_HCOUNTER            hCnterCpu = NULL;                      // cpu������
        PDH_HCOUNTER            hCnterMem = NULL;                      // �ڴ������
        std::list<PDH_HCOUNTER> hCounterUpSpeedList;                   // �ϴ��������б�
        std::list<PDH_HCOUNTER> hCounterDownSpeedList;                 // ���ؼ������б� 

        std::map<HWND, INT>     registeredMsg;                         // ��ע��Ĵ��ںͶ�Ӧ��֪ͨ��Ϣ

    public:
        PerfMonitor(DWORD queryInterval = 1000);
        ~PerfMonitor();

        // ����������
        BOOL Start();

        // ֹͣ������
        BOOL Stop();

        // ע�������Ϣ
        BOOL RegisterMessage(HWND hWnd, UINT wndMsg);

        // �����������
        BOOL GetPerfData(PERFDATA* pPerfData);

    private:
        // �̺߳���
        static DWORD CALLBACK QueryThreadProc(_In_ LPVOID lpParameter);

        DWORD QueryThread();

        // �򿪲�ѯ
        BOOL OpenQuery();

        // ��Ӽ�����
        BOOL AddCounter();

        // �����߳�
        BOOL StartThread();

        // �Զ����º���, ʹ�����̵߳���
        BOOL QueryPerfData();
    };

    // global functions

    BOOL InitializePerfMonitor(DWORD queryInterval = 1000);

    BOOL UninitializePerfMonitor();

    BOOL RegisterMessage(HWND hWnd, UINT wndMsg);

    BOOL GetPerfData(PERFDATA* pPerfData);
}

#endif // !DS_PERFMONITOR_H

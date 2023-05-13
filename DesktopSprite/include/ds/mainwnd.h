#pragma once
#ifndef DS_MAINWND_H
#define DS_MAINWND_H

#include <ds/framework.h>
#include <ds/perfmonitor.h>
#include <ds/notifyicon.h>
#include <ds/config.h>
#include <ds/basewindow.h>
#include <ds/winapp.h>


// ������Ϣ��غ궨��

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

using namespace Gdiplus;

// ����������Բ
BOOL DrawCircle(
    Graphics& graphics, 
    Pen& pen, 
    const PointF& ptCenter, 
    const REAL& nOuterRadius, 
    const REAL& sweepPercent
);

// ��������������ͼ
BOOL DrawSpeedStair(
    Graphics& graphics,
    const Color& color,
    const RectF& rect,
    const BOOL& bUp,
    const INT& nLevel,
    const INT& nMaxLevel = 6
);

class MainWindow :public BaseWindow
{
private:
    const WinApp*       app = NULL;
    AppConfig           config;
    PerfMonitor         perfMonitor;
    NotifyIcon*         pNotifyIcon = NULL;

// ����ʱ����
private:
    // ����������ʱ
    INT                                 wndSizeUnit = 0;                // ���ڵ�Ԫ���С, ������ʱ�� DPI ��������
    BOOL                                bWndFixed = FALSE;              // �����Ƿ�ͨ��ͼ����������ʾ
    BOOL                                bClocked = FALSE;               // �����������Ƿ񱨹�ʱ
    POINT                               ptDragSrc = { 0 };              // �϶�����ʱ��Դ��
    POINT                               currentFloatPos = { 0 };        // ����ʾ��������ʱ��ʱ���浱ǰ��������λ��
    Gdiplus::PrivateFontCollection      fontColl;                       // �ı���������
    HICON                               balloonIcon = NULL;             // ������Ϣ��ͼ����Դ

public:
    PCWSTR GetClassName_() const { return L"DesktopSpriteMainWndClass"; }
    PCWSTR GetConfigPath() const { return L"config.json"; }

public:
    MainWindow(WinApp* app) : app(app) {}

private:
    // ��ʾ�Ҽ��˵�
    BOOL ShowContextMenu(INT x, INT y);

    // ��ȡ����Ļ�ϵ�Ĭ��λ��
    BOOL GetDefaultWindowPos(POINT* pt);

    // ����������λ�ü��㴰�ڵ�λ��
    BOOL GetPopupWindowPos(POINT* pt);

    // �Ƿ񴰿ھ������������Ե�ཻ
    BOOL IsIntersectDesktop();

    // һ�������洢����λ�õ�ע���ķ���
    BOOL LoadLastPosFromReg(POINT* pt);
    BOOL SaveCurrentPosToReg();

    BOOL ApplyConfig();
    BOOL ApplyConfig(const CFGDATA* pcfgdata); // Ӧ�ø���, ֻ�޸ķ����仯��������

    void GetWndSizeByShowContent(PSIZE psizeWnd, BYTE byShowContent);
    BOOL TimeAlarm();
    INT ShowNoConentWarningMsg();

private:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    LRESULT OnActivate(WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnSettingChange(WPARAM wParam, LPARAM lParam);
    LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
    LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
    //LRESULT OnPowerBroadcast(WPARAM wParam, LPARAM lParam);
    LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimeAlarm(WPARAM wParam, LPARAM lParam);
    LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
};

#endif // !DS_MAINWND_H

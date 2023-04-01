#pragma once
#ifndef DS_MAINWND_H
#define DS_MAINWND_H

#include <ds/framework.h>
#include <ds/perfmonitor.h>
#include <ds/config.h>
#include <ds/basewindow.h>
#include <ds/notifyicon.h>
#include <ds/winapp.h>

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

// �������ļ���������, ��Ҫʹ�� delete �ͷ�ָ��
Font* CreateFontFromFile(PCWSTR szFontFilePath, REAL emSize = 9, INT style = 0, Unit unit = Unit::UnitPoint);

class MainWindow :BaseWindow
{
private:
    UINT                uMsgTaskbarCreated = 0;
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
    Gdiplus::PrivateFontCollection      fontColl;                       // �ı���������

    POINT                               lastFloatPos = { 0 };           // ���һ�θ�������λ��
    SIZE                                lastResolution = { 0 };         // �������ʱϵͳ�ֱ���

public:
    PCWSTR GetClassName_() const;
    PCWSTR GetConfigPath() const;

public:
    MainWindow(const WinApp* app);

    DWORD LoadFloatPosDataFromReg();
    DWORD UpdateFloatPosDataToRegByCurrentResolution();

    DWORD ApplyConfig();
    DWORD ApplyConfig(PCFGDATA pcfgdata); // Ӧ�ø���, ֻ�޸ķ����仯��������

    DWORD GetWndSizeByShowContent(PSIZE psizeWnd, BYTE byShowContent);
    DWORD TimeAlarm();

public:
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

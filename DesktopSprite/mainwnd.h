#pragma once
#include "framework.h"
#include "config.h"

using namespace Gdiplus;

#define MAINWNDCLASSNAME                L"DesktopSpriteMainWndClass"

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define WM_NOTIFYICON                   (WM_USER + 1)           // ֪ͨ������Ϣ
#define WM_TIMEALARM                    (WM_USER + 2)           // ��ʱ��Ϣ

#define STATUSCOLOR_LOW                 0xff00ff00
#define STATUSCOLOR_MIDDLE              0xffff8000
#define STATUSCOLOR_HIGH                0xffff0000

// ��������
typedef struct _MAINWNDDATA
{
    HWND            hWnd;

    // ������������
    BOOL            bFloatWnd;                      // �Ƿ����渡��
    BOOL            bInfoSound;                     // ������ʾ�Ƿ�������
    //WCHAR           szBalloonIconPath[MAX_PATH];    // ����ͼ���ļ�·��
    BOOL            bDarkTheme;                     // �Ƿ�����ɫ����
    BYTE            byTransparency;                 // ����͸����    // TODO: ������͸����
    BYTE            byShowContent;                  // Ҫ��ʾ������(���������С)

    // ����������ʱ
    INT             wndSizeUnit;                    // ���ڵ�Ԫ���С, ������ʱ�� DPI ��������
    BOOL            bWndFixed;                      // �����Ƿ�ͨ��ͼ����������ʾ
    POINT           ptDragSrc;                      // �϶�����ʱ��Դ��
    PrivateFontCollection* pFontColl;               // �ı���������

    // ��������ʱ
    SIZE            runtimeResolution;              // �������ʱϵͳ�ֱ���
}*PMAINWNDDATA, MAINWNDDATA;

// ע��������
ATOM RegisterMainWnd(HINSTANCE hInstance);

// �����ڴ�������
HWND CreateMainWnd(HINSTANCE hInstance);

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
Font* CreateFontFromFile(PCWSTR szFontFilePath, REAL emSize = 9, INT style = 0, Unit unit = UnitPoint);

// ���ݵ�ǰ��ʾ�����봰�嵥Ԫ���С���㴰���С, ���ص�λ
DWORD GetWndSizeByShowContent(PMAINWNDDATA pWndData, PSIZE psizeWnd, BYTE byShowContent);

// Ӧ��������
//DWORD ApplyAppConfig(PCFGDATA pCfgData, HWND hMainWnd);

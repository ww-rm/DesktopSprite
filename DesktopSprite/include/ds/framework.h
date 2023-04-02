#pragma once
#ifndef DS_FRAMEWORK_H
#define DS_FRAMEWORK_H

//#pragma comment(linker,"\"/manifestdependency:type='win32' \
//name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
//processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Windows headers
#include <Windows.h>
#include <windowsx.h>
#include <strsafe.h>
#include <dwmapi.h>
#include <Shlwapi.h>
#include <PathCch.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include <gdiplus.h>
//#include <d2d1.h>
//#include <CommCtrl.h>

// C headers
#include <stdlib.h>

// C++ headers
#include <list>
#include <fstream>

#include "resource.h"

#define SZMSG_TASKBARCREATED            L"TaskbarCreated"

#define MAX_LOADSTRING                      256                     // �ַ�������������󳤶�

// ������Ϣ��غ궨��

#define IDT_REFRESHRECT                 1
#define IDT_TIMEALARM                   2

#define WM_NOTIFYICON                   (WM_USER + 1)           // ֪ͨ������Ϣ
#define WM_TIMEALARM                    (WM_USER + 2)           // ��ʱ��Ϣ


#endif // !DS_FRAMEWORK_H

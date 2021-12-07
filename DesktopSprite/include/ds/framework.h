#pragma once
//#pragma comment(linker,"\"/manifestdependency:type='win32' \
//name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
//processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <Windows.h>
#include <windowsx.h>
#include <strsafe.h>
#include <dwmapi.h>
#include <Shlwapi.h>
#include <PathCch.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include <gdiplus.h>
//#include <CommCtrl.h>

#include "resource.h"

#define APPNAME                         L"DesktopSprite"               // TODO: 应用程序的标识字符串

#define SZMSG_TASKBARCREATED            L"TaskbarCreated"

#define MAX_LOADSTRING                      256                     // 字符串缓冲区的最大长度

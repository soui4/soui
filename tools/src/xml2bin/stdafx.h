﻿// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#define  _CRT_SECURE_NO_WARNINGS
#define	 DLL_SOUI
#include <core-def.h>

//需要IAccessible接口支持打开下面的宏: 2018.10.17
//#define SOUI_ENABLE_ACC

#ifdef DLL_CORE
# ifdef SOUI_EXPORTS
#   define SOUI_EXP __declspec(dllexport)
# else
#   define SOUI_EXP __declspec(dllimport)
# endif // SOUI_EXPORTS
#else
#   define SOUI_EXP
#endif

// Change these values to use different versions
#ifndef WINVER 
#define WINVER        0x0500
#define _WIN32_WINNT    0x0502
#endif//WINVER

#ifndef _WIN32_IE
#define _WIN32_IE    0x0601
#endif

#ifndef _RICHEDIT_VER
#define _RICHEDIT_VER    0x0200
#endif

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif//_CRT_NON_CONFORMING_SWPRINTFS


# pragma warning(disable:4661)
# pragma warning(disable:4251)
# pragma warning(disable:4100)    //unreferenced formal parameter
# pragma warning(disable:4355)
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#include <windows.h>

#include <CommCtrl.h>
#include <Shlwapi.h>
#include <OleCtl.h>
#include <tchar.h>
#include <stdio.h>

#include <pugixml/pugixml.hpp>
#include <string/tstring.h>
#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include <windows.h>
#include <stdint.h>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <OleCtl.h>
#include <tchar.h>
#include <guid.h>
#include <sysapi.h>
#include <OleAcc.h>
#include <OCIdl.h>
#if _MSC_VER < 1500
#include <tmschema.h> //for VS2005
#else
#include <vsstyle.h>
#endif
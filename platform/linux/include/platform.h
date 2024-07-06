#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#ifndef WINVER
#define WINVER       0x0500
#endif // WINVER
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif//_WIN32_WINNT

#include <unknown-i.h>
#include <dataobject-i.h>
#include <droptarget-i.h>
#include <sdef.h>
#include <dlfcn.h>

#include "ctypes.h"
#include "tchar.h"
#include "strfun.h"
#include "Objbase.h"
#include "guid.h"
#include "winuser.h"
#include "error.h"
#include "debug.h"
#include "class.h"

#include "ctrl_types.h"
#include "sysapi.h"
#include "wnd.h"
#include "region.h"
#include "gdi.h"

#endif//__PLATFORM_H_

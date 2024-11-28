//需要IAccessible接口支持打开下面的宏: 2018.10.17
//#define SOUI_ENABLE_ACC

// Change these values to use different versions
#ifndef WINVER
#define WINVER       0x0500
#endif // WINVER
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif//_WIN32_WINNT

#ifndef _WIN32_IE
#define _WIN32_IE 0x0601
#endif

#ifndef _RICHEDIT_VER
#define _RICHEDIT_VER 0x0200
#endif

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif //_CRT_NON_CONFORMING_SWPRINTFS

#pragma warning(disable : 4661)
#pragma warning(disable : 4251)
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4355)

#include <config.h>
#include <soui-version.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#ifdef _WIN32
#include <commctrl.h>
#include <Oleacc.h>	//IAccessible
#include <olectl.h>
#include <ShellAPI.h>
#pragma comment(lib,"shell32.lib")
#endif//_WIN32

#include <utilities.h>
#include <soui_exp.h>

#include <core/SDefine.h>

#define _WTYPES_NS SOUI
#include <souicoll.h>
#include <wtl.mini/msgcrack.h>
#include <wtl.mini/souimisc.h>
#include <atl.mini/atldef.h>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <xml/SXml.h>

#include <interface/SRender-i.h>
#include <interface/SImgDecoder-i.h>
#include <helper/slog.h>
#include <helper/SAssertFmt.h>
#include <helper/SAttrCracker.h>
#include <helper/SColor.h>
#include <helper/SplitString.h>
#include <SApp.h>

#include <res.mgr/SFontPool.h>
#include <res.mgr/SResProvider.h>
#include <res.mgr/SUiDef.h>
#include <res.mgr/SDpiAwareFont.h>
#include <control/SouiCtrls.h>
#include <control/SMessageBox.h>


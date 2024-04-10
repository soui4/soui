#ifndef _SYS_API_H
#define _SYS_API_H
#include <platform_exp.h>
#include <platform.h>
int  PLATFORM_API GetWindowScale(HWND hWnd);
BOOL PLATFORM_API IsFilePathValid(LPCTSTR path);
BOOL PLATFORM_API HasFont(LPCTSTR fontName);
BOOL PLATFORM_API IsSuppotMaxMove();

#endif
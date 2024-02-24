#ifndef _SYS_API_H
#define _SYS_API_H
#include <platform_exp.h>
#include <windows.h>
int  PLATFORM_API GetWindowScale(HWND hWnd);
BOOL PLATFORM_API IsFilePathValid(LPCTSTR path);
#endif
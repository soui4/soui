#ifndef _SYS_API_H
#define _SYS_API_H
#include <platform_exp.h>
#include <platform.h>
int  PLATFORM_API GetWindowScale(HWND hWnd);
BOOL PLATFORM_API IsFilePathValid(LPCTSTR path);
BOOL PLATFORM_API HasFont(LPCTSTR fontName);
BOOL PLATFORM_API IsSupportMaxMove();
BOOL PLATFORM_API UpdateDIBPixmap(HBITMAP bmp,int wid,int hei,int bitsPixel,int stride,CONST VOID*pjBits);
long PLATFORM_API file_length_A(const char *path);
long PLATFORM_API file_length_W(const wchar_t *path);
#ifdef _UNICODE
#define file_length file_length_W
#else
#define file_length file_length_A
#endif

#endif
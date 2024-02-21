#ifndef _SYS_API_H
#define _SYS_API_H
#include <ctypes.h>

void DestroyIcon(HICON hIcon);
void GetWindowRect(HWND hWnd,RECT *rc);
int  GetWindowScale(HWND hWnd);
#endif
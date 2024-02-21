#ifndef _SYS_API_H
#define _SYS_API_H
#include <ctypes.h>

void DestroyIcon(HICON hIcon);
void GetWindowRect(HWND hWnd,RECT *rc);
int  GetWindowScale(HWND hWnd);

void PostThreadMessage(int tid, UINT msg,WPARAM wp,LPARAM lp);
BOOL WaitMessage();
#endif
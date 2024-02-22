#ifndef _SYS_API_H
#define _SYS_API_H
#include <ctypes.h>


inline long InterlockedDecrement(long *v){
    return __atomic_fetch_sub(v,1,__ATOMIC_SEQ_CST);
}

inline long InterlockedIncrement(long *v){
    return __atomic_fetch_add(v,1,__ATOMIC_SEQ_CST);
}

BOOL IIDFromString(LPCWSTR s, GUID *id);

void DestroyIcon(HICON hIcon);
void GetWindowRect(HWND hWnd,RECT *rc);
int  GetWindowScale(HWND hWnd);

void PostThreadMessage(int tid, UINT msg,WPARAM wp,LPARAM lp);
BOOL WaitMessage();

void SetLastError(int e);

int GetLastError() ;

int MulDiv(int a,int b,int c);

uint32_t GetCurrentThreadId();

int MultiByteToWideChar(int cp,int flags, const char *src, int len,wchar_t *dst,int dstLen);

int WideCharToMultiByte(int cp,int flags,const wchar_t* src,int len,char * dst,int dstLen,BOOL *p1,BOOL *p2);

#define STIF_DEFAULT        0x00000000L
#define STIF_SUPPORT_HEX    0x00000001L
BOOL StrToInt64ExW(const wchar_t *str, DWORD flags, LONGLONG *ret);
BOOL StrToIntExW(const wchar_t *str, DWORD flags, INT *ret);
#endif
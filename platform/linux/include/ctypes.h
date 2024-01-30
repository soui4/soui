#ifndef __PLATFORM_LINUX_
#define __PLATFORM_LINUX_

#include <stdint.h>
#include <stdarg.h>
#include <stdatomic.h>

#define HRESULT int
#define UINT int
typedef int64_t __int64;// int64_t
#define E_INVALIDARG 1
#define S_OK 0
#define S_FALSE 1


#define FAILED(x) (x)!=S_OK
#define ULONG uint64_t

typedef uint32_t DWORD;
typedef uint16_t WORD;

struct GUID{
DWORD ids[4];
};

typedef int BOOL;
#define FALSE 0
#define TRUE 1


#define HINSTANCE int
#define HMODULE int

typedef char *LPSTR;
typedef wchar_t *LPWSTR;
typedef const char * LPCSTR;
typedef const wchar_t * LPCWSTR;

#define VK_SPACE 0x20

typedef void * LPVOID;

#define __cdecl __attribute__((cdecl))
#define __stdcall __attribute__((stdcall))

#define ARRAYSIZE(x) (sizeof((x))/sizeof((x)[0]))


inline long InterlockedDecrement(long *v){
    return atomic_fetch_sub(v,1);
}

inline long InterlockedIncrement(long *v){
    return atomic_fetch_add(v,1);
}

#endif//__PLATFORM_LINUX_
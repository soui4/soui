#ifndef __PLATFORM_LINUX_
#define __PLATFORM_LINUX_

#include <stdint.h>
#include <stdarg.h>


#define HRESULT int
typedef unsigned int UINT;
typedef int64_t __int64;// int64_t
#define E_INVALIDARG 1
#define E_OUTOFMEMORY 2
#define E_POINTER 3

#define S_OK 0
#define S_FALSE 1


#define FAILED(x) (x)!=S_OK
#define ULONG uint64_t

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int32_t LONG;
typedef int64_t LONGLONG;
typedef int32_t INT;
typedef wchar_t WCHAR;
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
typedef intptr_t INT_PTR;
typedef uintptr_t ULONG_PTR;

#define __cdecl //__attribute__((cdecl))
#define __stdcall //__attribute__((stdcall))

#define ARRAYSIZE(x) (sizeof((x))/sizeof((x)[0]))


inline long InterlockedDecrement(long *v){
    return __atomic_fetch_sub(v,1,__ATOMIC_SEQ_CST);
}

inline long InterlockedIncrement(long *v){
    return __atomic_fetch_add(v,1,__ATOMIC_SEQ_CST);
}

#define UNREFERENCED_PARAMETER(x)

typedef DWORD   COLORREF;
typedef DWORD   *LPCOLORREF;
typedef int16_t SHORT;

#define HFILE_ERROR ((HFILE)-1)

typedef struct tagRECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT, *PRECT,  *NPRECT,  *LPRECT;

typedef const RECT * LPCRECT;

typedef struct _RECTL       /* rcl */
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECTL, *PRECTL, *LPRECTL;

typedef const RECTL * LPCRECTL;

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *PPOINT,  *NPPOINT,  *LPPOINT;

typedef struct _POINTL      /* ptl  */
{
    LONG  x;
    LONG  y;
} POINTL, *PPOINTL;

typedef struct tagSIZE
{
    LONG        cx;
    LONG        cy;
} SIZE, *PSIZE, *LPSIZE;

typedef SIZE               SIZEL;
typedef SIZE               *PSIZEL, *LPSIZEL;

typedef struct tagPOINTS
{
#ifndef _MAC
    SHORT   x;
    SHORT   y;
#else
    SHORT   y;
    SHORT   x;
#endif
} POINTS, *PPOINTS, *LPPOINTS;

#endif//__PLATFORM_LINUX_
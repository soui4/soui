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

typedef uint8_t BYTE;
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int32_t LONG;
typedef int64_t LONGLONG;
typedef int32_t INT;
typedef wchar_t WCHAR;
typedef void * LPVOID;
typedef intptr_t INT_PTR;
typedef uintptr_t ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;

struct GUID{
DWORD ids[4];
};


#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#define RGB(r,g,b) ((r)|(g)<<8|(b)<<16)
#define GetRValue(rgb)   (LOBYTE(rgb))
#define GetGValue(rgb)  (LOBYTE(rgb<<8))
#define GetBValue(rgb)  (LOBYTE(rgb<<16))

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
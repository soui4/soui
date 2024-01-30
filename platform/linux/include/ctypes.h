#ifndef __PLATFORM_LINUX_
#define __PLATFORM_LINUX_

#include <stdint.h>
#include <stdarg.h>


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
typedef intptr_t LONG_PTR;
typedef uintptr_t UINT_PTR;
typedef uintptr_t ULONG_PTR;
typedef intptr_t DWORD_PTR;
typedef int32_t LRESULT;
typedef int32_t HRESULT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;

typedef struct _GUID{
DWORD ids[4];
}GUID;


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

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#define IDCLOSE         8
#define IDHELP          9
#define IDTRYAGAIN      10
#define IDCONTINUE      11


#define HINSTANCE int
#define HMODULE int
#define HICON int
#define HWND  int
#define HMENU int
#define HGDIOBJ int
#define HBRUSH int
#define HPEN   int

typedef char *LPSTR;
typedef wchar_t *LPWSTR;
typedef const char * LPCSTR;
typedef const wchar_t * LPCWSTR;

#define VK_SPACE 0x20



#define __cdecl //__attribute__((cdecl))
#define __stdcall //__attribute__((stdcall))
#define WINAPI __stdcall

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


#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#endif//__PLATFORM_LINUX_
#ifndef __PLATFORM_LINUX_
#define __PLATFORM_LINUX_
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#ifndef _INTPTR_T
#ifndef __intptr_t_defined
typedef __INTPTR_TYPE__ intptr_t;
#define __intptr_t_defined
#define _INTPTR_T
#endif
#endif

#ifndef _UINTPTR_T
typedef __UINTPTR_TYPE__ uintptr_t;
#define _UINTPTR_T
#endif


typedef unsigned int UINT;
typedef int64_t __int64;// int64_t
#define E_INVALIDARG 1
#define E_OUTOFMEMORY 2
#define E_POINTER 3
#define E_FAIL 4
#define E_NOTIMPL 5

#define S_OK 0
#define S_FALSE 1


#define FAILED(x) (x)!=S_OK

typedef unsigned long ULONG;
typedef uint8_t BYTE;
typedef BYTE * LPBYTE;
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
typedef intptr_t LONG_PTR;
typedef uintptr_t UINT_PTR;
typedef uintptr_t ULONG_PTR;
typedef intptr_t DWORD_PTR;
typedef int32_t LRESULT;
typedef int32_t HRESULT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef uint16_t USHORT;
typedef uint64_t ULONG64;
typedef void VOID;
typedef void* LPVOID;
typedef void* PVOID;
typedef uint8_t UCHAR;

#define __time64_t time_t

typedef struct _SYSTEMTIME{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
}SYSTEMTIME;

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
#define HBITMAP int
#define HRSRC int
#define HCURSOR int
#define HDC int
#define HRGN int
#define HFONT int
#define HANDLE int
#define ATOM int

typedef char *LPSTR;
typedef wchar_t *LPWSTR;
typedef const char * LPCSTR;
typedef const wchar_t * LPCWSTR;

#define VK_SPACE 0x20



#define __cdecl //__attribute__((cdecl))
#define __stdcall //__attribute__((stdcall))
#define WINAPI __stdcall
#define CALLBACK 
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

#define SUCCEEDED(x) ((x)>0)
#define INFINITE     (UINT)-1

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif


#define SRCCOPY             (DWORD)0x00CC0020 /* dest = source                   */
#define SRCPAINT            (DWORD)0x00EE0086 /* dest = source OR dest           */
#define SRCAND              (DWORD)0x008800C6 /* dest = source AND dest          */
#define SRCINVERT           (DWORD)0x00660046 /* dest = source XOR dest          */
#define DSTINVERT           (DWORD)0x00550009 /* dest = (NOT dest)               */

#define LF_FACESIZE 32
typedef struct tagLOGFONT
{
    LONG      lfHeight;
    LONG      lfWidth;
    LONG      lfEscapement;
    LONG      lfOrientation;
    LONG      lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    wchar_t      lfFaceName[LF_FACESIZE];
} LOGFONT;

#define RGN_AND             1
#define RGN_OR              2
#define RGN_XOR             3
#define RGN_DIFF            4
#define RGN_COPY            5
#define RGN_MIN             RGN_AND
#define RGN_MAX             RGN_COPY


typedef struct tagMSG
    {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
    } 	MSG;

typedef struct tagMSG *PMSG;
typedef struct tagMSG *LPMSG;


typedef struct _BLENDFUNCTION {
  BYTE     BlendOp;
  BYTE     BlendFlags;
  BYTE     SourceConstantAlpha;
  BYTE     AlphaFormat;
}BLENDFUNCTION, *PBLENDFUNCTION, *LPBLENDFUNCTION;


typedef struct _IDropTarget{

}IDropTarget;


/* Pen Styles */
#define PS_SOLID            0
#define PS_DASH             1       /* -------  */
#define PS_DOT              2       /* .......  */
#define PS_DASHDOT          3       /* _._._._  */
#define PS_DASHDOTDOT       4       /* _.._.._  */
#define PS_NULL             5
#define PS_INSIDEFRAME      6
#define PS_USERSTYLE        7
#define PS_ALTERNATE        8
#define PS_STYLE_MASK       0x0000000F

#define PS_ENDCAP_ROUND     0x00000000
#define PS_ENDCAP_SQUARE    0x00000100
#define PS_ENDCAP_FLAT      0x00000200
#define PS_ENDCAP_MASK      0x00000F00

#define PS_JOIN_ROUND       0x00000000
#define PS_JOIN_BEVEL       0x00001000
#define PS_JOIN_MITER       0x00002000
#define PS_JOIN_MASK        0x0000F000

#define PS_COSMETIC         0x00000000
#define PS_GEOMETRIC        0x00010000
#define PS_TYPE_MASK        0x000F0000

#define AD_COUNTERCLOCKWISE 1
#define AD_CLOCKWISE        2

#endif//__PLATFORM_LINUX_
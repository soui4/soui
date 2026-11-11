#ifndef __SDEFINE__H__
#define __SDEFINE__H__

/** DISALLOW_COPY_AND_ASSIGN disables the copy and assignment constructors. */
/** Must be used within the class's private: access control section. */
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &);            \
    void operator=(const TypeName &)

#ifndef NOSMINMAX
#ifndef smax
#define smax(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef smin
#define smin(a, b) (((a) < (b)) ? (a) : (b))
#endif
#endif

#define sround(x) ((int)((x) + 0.5f))

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam) ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam) ((int)(short)HIWORD(lParam))
#endif

#ifdef _DISABLE_NO_VTABLE
#define S_NO_VTABLE
#else
#define S_NO_VTABLE __declspec(novtable)
#endif

/**
 * SOUI_PLATFORM_WIN32
 *
 * 1 when the build targets the real Win32 API and the real GDI; 0 for every
 * swinx-backed port (Linux / macOS / Android / iOS / wasm / FreeRTOS).
 *
 * "#ifdef _WIN32" does NOT answer that question.  The FreeRTOS port is
 * commonly built on a Windows host with MinGW, where _WIN32 is predefined
 * even though every platform call is served by swinx -- and swinx
 * deliberately differs from the real GDI where this code has to care, most
 * notably COLORREF: the real GDI ignores the alpha byte, swinx implements it.
 * Test this macro instead of _WIN32 whenever real-GDI semantics are what the
 * code actually needs.
 */
#if defined(_WIN32) && !defined(SOUI_PLATFORM_FREERTOS) && !defined(FREERTOS_HOST)
#define SOUI_PLATFORM_WIN32 1
#else
#define SOUI_PLATFORM_WIN32 0
#endif

/** SWindow Handle */
typedef DWORD SWND;
typedef ULONG_PTR HSTREEITEM;

#endif /**< __SDEFINE__H__ */
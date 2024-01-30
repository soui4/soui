#include <config.h>

#if defined(DLL_CORE) && defined(_WIN32)
#ifdef UTILITIES_EXPORTS
#define UTILITIES_API __declspec(dllexport)
#else
#define UTILITIES_API __declspec(dllimport)
#endif
#else
#define UTILITIES_API
#endif


#ifndef SASSERT
#include <assert.h>
#define SASSERT(x) assert(x)
#endif

#ifdef LIB_SOUI_COM
#define SOUI_COM_API
#define SOUI_COM_C
#else
#ifdef _WIN32
#define SOUI_COM_API __declspec(dllexport)
#else
#define SOUI_COM_API
#endif
#define SOUI_COM_C  EXTERN_C
#endif//LIB_SOUI_COM

#ifndef NULL
#define NULL 0
#endif

#include <sdef.h>
#include <stddef.h>
#include <objbase.h>

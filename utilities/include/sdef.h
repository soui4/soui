#ifndef __SDEF__H__
#define __SDEF__H__


#ifdef __cplusplus
#define SNS SOUI	//replase it to your ns
#define SCONST const
#define SNSBEGIN	namespace SNS{
#define SNSEND		}//end of namespace SOUI
#define OVERRIDE override
#define CTHIS_  THIS_
#define CTHIS  THIS
#define DEF_VAL(x) =x
#else
#define SCONST
#define SNSBEGIN
#define SNSEND
#define OVERRIDE
#define CTHIS_ const THIS_
#define CTHIS const THIS
#define DEF_VAL(x)
#endif

#ifndef smax
#define smax(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef smin
#define smin(a,b) ((a)<(b)?(a):(b))
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

#ifdef _WIN32
typedef unsigned int tid_t;
#endif//_WIN32

#endif // __SDEF__H__
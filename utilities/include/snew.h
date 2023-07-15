//copy from <new.h>
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
#ifdef     _MSC_VER
inline void *__CRTDECL operator new(size_t, void *_Where)
{return (_Where); }
#if     _MSC_VER >= 1200
inline void __CRTDECL operator delete(void *, void *)
{return; }
#endif
#else
//mingw 已经定义了，不能重新定义。
#include <new>
#endif
#endif

/********************************************************************
	created:	2014/10/30
	created:	30:10:2014   10:24
	filename: 	soui_mem_wrapper.h
	author:		soui group
	
	purpose:	包装内存动态分配
*********************************************************************/
#ifndef __SOUI_MEM_WRAPPER__H__
#define __SOUI_MEM_WRAPPER__H__
#include "utilities-def.h"

namespace SOUI
{
    class UTILITIES_API soui_mem_wrapper
    {
    public:
        static void * SouiMalloc(size_t szMem);
        static void * SouiRealloc(void *p,size_t szMem);
        static void * SouiCalloc(size_t count, size_t szEle);
        static void   SouiFree(void *p);
    };
}

#endif // __SOUI_MEM_WRAPPER__H__
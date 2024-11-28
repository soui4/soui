#ifndef __OBJ_REF_I__H__
#define __OBJ_REF_I__H__

#include <sdef.h>
#include <objbase.h>
#ifdef _WIN32
#include <sguid.h>
#else
#include <guiddef.h>
#endif//_WIN32

#undef INTERFACE
#define INTERFACE IObjRef
DECLARE_INTERFACE(IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return 新引用计数
    */
    STDMETHOD_(long,AddRef) (THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
    */
    STDMETHOD_(long,Release) (THIS) PURE;

	/**
	 * @brief 释放对象
	 * @return void
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;
};

#endif // __OBJ_REF_I__H__
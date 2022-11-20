#pragma once
#include <interface/obj-ref-i.h>
SNSBEGIN

#define MAX_OBJNAME 100

typedef struct _SObjectInfo{
	wchar_t szName[MAX_OBJNAME];
	int		nType;
}SObjectInfo;

#undef INTERFACE
#define INTERFACE IObjectFactory
DECLARE_INTERFACE_(IObjectFactory,IObjRef){
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

	STDMETHOD_(IObject *,NewObject)(CTHIS) SCONST PURE;
	STDMETHOD_(LPCWSTR,BaseClassName)(CTHIS) SCONST PURE;
	STDMETHOD_(SObjectInfo,GetObjectInfo)(CTHIS) SCONST PURE;
	STDMETHOD_(IObjectFactory *,Clone)(CTHIS) SCONST PURE;
};

SNSEND
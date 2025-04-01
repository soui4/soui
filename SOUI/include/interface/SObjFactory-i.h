#ifndef __SOBJFACTORY_I__H__
#define __SOBJFACTORY_I__H__
#include <interface/obj-ref-i.h>
SNSBEGIN

#define MAX_OBJNAME 100

typedef struct _SObjectInfo
{
    wchar_t szName[MAX_OBJNAME];
    LPCWSTR szAlise;
    int nType;
} SObjectInfo;

#undef INTERFACE
#define INTERFACE IObjectFactory
DECLARE_INTERFACE_(IObjectFactory, IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return long - 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return long - 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 创建新对象
     * @return IObject* - 新创建的对象指针
     */
    STDMETHOD_(IObject *, NewObject)(CTHIS) SCONST PURE;

    /**
     * @brief 获取基类名称
     * @return LPCWSTR - 基类名称
     */
    STDMETHOD_(LPCWSTR, BaseClassName)(CTHIS) SCONST PURE;

    /**
     * @brief 获取对象信息
     * @return SObjectInfo - 对象信息结构体
     */
    STDMETHOD_(SObjectInfo, GetObjectInfo)(CTHIS) SCONST PURE;

    /**
     * @brief 克隆对象工厂
     * @return IObjectFactory* - 克隆的对象工厂指针
     */
    STDMETHOD_(IObjectFactory *, Clone)(CTHIS) SCONST PURE;
};

SNSEND
#endif // __SOBJFACTORY_I__H__

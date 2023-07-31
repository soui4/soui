/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       Sobject.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/01
 *
 * Describe    the base class used in SOUI, which provides type identify of class in runtime
 *             and implement attributes dispatcher described in XML.
 */

#ifndef __SOBJECT_I__H__
#define __SOBJECT_I__H__
#include <interface/obj-ref-i.h>
#include <interface/sstring-i.h>
#include <interface/sxml-i.h>
#include <stdint.h>

#pragma warning(disable : 4275)
//////////////////////////////////////////////////////////////////////////
SNSBEGIN

#define WIDESTR_HELPER(x) L##x
#define WIDESTR(x)        WIDESTR_HELPER(#x)

#ifdef __cplusplus
#define DEF_OBJ_BASE(clsName, clsType) \
    static int GetClassType()          \
    {                                  \
        return clsType;                \
    }                                  \
    static LPCWSTR GetClassName()      \
    {                                  \
        return WIDESTR(clsName);       \
    }
#else
#define DEF_OBJ_BASE(clsName, clsType)
#endif

// SObject Class Name Declaration
#define DEF_SOBJECT(baseCls, clsName)                   \
  public:                                               \
    typedef baseCls __baseCls;                          \
    static LPCWSTR GetClassName()                       \
    {                                                   \
        return clsName;                                 \
    }                                                   \
    static int GetClassType()                           \
    {                                                   \
        return __baseCls::GetClassType();               \
    }                                                   \
                                                        \
    static LPCWSTR BaseClassName()                      \
    {                                                   \
        return __baseCls::GetClassName();               \
    }                                                   \
                                                        \
    virtual LPCWSTR WINAPI GetObjectClass() const       \
    {                                                   \
        return clsName;                                 \
    }                                                   \
                                                        \
    virtual BOOL WINAPI IsClass(LPCWSTR lpszName) const \
    {                                                   \
        if (wcscmp(GetClassName(), lpszName) == 0)      \
            return TRUE;                                \
        return __baseCls::IsClass(lpszName);            \
    }

typedef struct IObject IObject;

typedef HRESULT (*FunAttrHandler)(IObject *pObj, const IStringW *attrName, const IStringW *attrValue, BOOL bLoading);

typedef enum _SObjectType
{
    Undef = 0,
    Window,
    Skin,
    Layout,
    LayoutParam,
    Event,
    Interpolator,
    Animation,
    ValueAnimator,
    UserType,
} SObjectType;

/**
 * @class      SObject
 * @brief      SOUI系统中的对象基类
 *
 * Describe    提供类RTTI机制，实现从XML节点中给派生类对象设置属性
 */
#undef INTERFACE
#define INTERFACE IObject
DECLARE_INTERFACE_(IObject, IObjRef){ DEF_OBJ_BASE(IObject, Undef)
#include <interface/SobjectApi.h>
};

#ifdef __cplusplus
/**
 * sobj_cast
 * @brief    SOUI Object 的类型安全的类型转换接口
 * @param    SObject * pObj --  源对象
 * @return   T * -- 转换后的对象
 * Describe  如果源对象不是待转换对象类型，返回NULL
 */
template <class T>
T *sobj_cast(const IObject *pObj)
{
    if (!pObj)
        return NULL;

    if (pObj->IsClass(T::GetClassName()))
        return (T *)pObj;
    else
        return NULL;
}

#endif

SNSEND

#endif // __SOBJECT_I__H__
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

#pragma once
#include <interface/obj-ref-i.h>
#include <interface/sstring-i.h>
#include <interface/sxml-i.h>
#include <stdint.h>

#pragma warning(disable : 4275)
//////////////////////////////////////////////////////////////////////////
SNSBEGIN

// SObject Class Name Declaration
#define DEF_SOBJECT_EX(baseCls, classname, clsType)     \
  public:                                               \
    typedef baseCls __baseCls;                          \
    static LPCWSTR GetClassName()                       \
    {                                                   \
        return classname;                               \
    }                                                   \
                                                        \
    static int GetClassType()                           \
    {                                                   \
        int ret = clsType;                              \
        if (ret == SOUI::Undef)                         \
            ret = __baseCls::GetClassType();            \
        return ret;                                     \
    }                                                   \
                                                        \
    static LPCWSTR BaseClassName()                      \
    {                                                   \
        return __baseCls::GetClassName();               \
    }                                                   \
                                                        \
    virtual LPCWSTR WINAPI GetObjectClass() const       \
    {                                                   \
        return classname;                               \
    }                                                   \
                                                        \
    virtual int WINAPI GetObjectType() const            \
    {                                                   \
        int ret = clsType;                              \
        if (ret == SOUI::Undef)                         \
            ret = __baseCls::GetObjectType();           \
        return ret;                                     \
    }                                                   \
                                                        \
    virtual BOOL WINAPI IsClass(LPCWSTR lpszName) const \
    {                                                   \
        if (wcscmp(GetClassName(), lpszName) == 0)      \
            return TRUE;                                \
        return baseCls::IsClass(lpszName);              \
    }

#define DEF_SOBJECT(baseCls, classname) DEF_SOBJECT_EX(baseCls, classname, 0)


typedef struct IObject IObject;

typedef HRESULT (*FunAttrHandler)(IObject * pObj,const IStringW * attrName,const IStringW * attrValue,BOOL bLoading);

/**
 * @class      SObject
 * @brief      SOUI系统中的对象基类
 *
 * Describe    提供类RTTI机制，实现从XML节点中给派生类对象设置属性
 */
#undef INTERFACE
#define INTERFACE IObject
DECLARE_INTERFACE_(IObject, IObjRef)
{
#ifdef __cplusplus
    /**
     * GetClassName
     * @brief    获得对象类型名称
     * @return   LPCWSTR -- 类名
     * Describe  静态函数
     */
    static LPCWSTR GetClassName()
    {
        return L"object";
    }

    /**
     * GetClassType
     * @brief    获得对象类型
     * @return   int -- 类型
     * Describe  静态函数
     */
    static int GetClassType()
    {
        return 0;
    }
#endif

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


typedef enum _SObjectType
{
	None = -1,
	Undef = 0,
	NativeWnd,
	Window,
	Skin,
	Layout,
	Event,
	Interpolator,
	Animation,
	ValueAnimator,
	UserType,
} SObjectType;


SNSEND

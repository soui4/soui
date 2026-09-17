/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       sobject-i.h
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

#if defined(_MSC_VER)
#pragma warning(disable : 4275)
#endif
///////////////////////////////////////////////////////////////////////
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
    }                                  \
    static LPCWSTR GetClassAlias()     \
    {                                  \
        return NULL;                   \
    }

#else
#define DEF_OBJ_BASE(clsName, clsType)
#endif

/** SObject Class Name Declaration */
#define DEF_SOBJECT_EX(baseCls, clsName, clsAlise)                                                             \
  public:                                                                                                      \
    typedef baseCls __baseCls;                                                                                 \
    static LPCWSTR GetClassName()                                                                              \
    {                                                                                                          \
        return clsName;                                                                                        \
    }                                                                                                          \
    static int GetClassType()                                                                                  \
    {                                                                                                          \
        return __baseCls::GetClassType();                                                                      \
    }                                                                                                          \
    static LPCWSTR GetClassAlise()                                                                             \
    {                                                                                                          \
        return clsAlise;                                                                                       \
    }                                                                                                          \
                                                                                                               \
    static LPCWSTR BaseClassName()                                                                             \
    {                                                                                                          \
        return __baseCls::GetClassName();                                                                      \
    }                                                                                                          \
                                                                                                               \
    virtual LPCWSTR WINAPI GetObjectClass() const OVERRIDE \
    {                                                                                                          \
        return clsName;                                                                                        \
    }                                                                                                          \
                                                                                                               \
    virtual BOOL WINAPI IsClass(LPCWSTR lpszName) const OVERRIDE \
    {                                                                                                          \
        if (wcscmp(GetClassName(), lpszName) == 0)                                                             \
            return TRUE;                                                                                       \
        return __baseCls::IsClass(lpszName);                                                                   \
    }                                                                                                          \
    virtual int WINAPI GetClassNameList(wchar_t ppClassNameList[][50], int nMaxCount) const OVERRIDE \
    {                                                                                                          \
        if (nMaxCount > 0 && ppClassNameList)                                                                  \
            wcscpy_s(ppClassNameList[0], 50, GetClassName());                                                  \
        return __baseCls::GetClassNameList(ppClassNameList ? (ppClassNameList + 1) : NULL, nMaxCount - 1) + 1; \
    }

#define DEF_SOBJECT(baseCls, clsName) DEF_SOBJECT_EX(baseCls, clsName, NULL)

typedef struct IObject IObject;

typedef HRESULT (*FunAttrHandler)(IObject *pObj, const IStringW *attrName, const IStringW *attrValue, BOOL bLoading);

typedef enum _SObjectType
{
    Invalid = -1,
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
 * @brief      Object base class in the SOUI system
 *
 * Describe     Provides RTTI mechanism for derived class objects to set properties from XML nodes
 */
#undef INTERFACE
#define INTERFACE IObject
DECLARE_INTERFACE_(IObject, IObjRef){ DEF_OBJ_BASE(IObject, Undef)
#include <interface/SobjectApi.h>
};

#ifdef __cplusplus
/**
 * sobj_cast
 * @brief    Type-safe type cast interface for SOUI Object
 * @param    SObject * pObj --  source object
 * @return   T * -- converted object
 * Describe  Returns NULL if the source object is not of the target type
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

#endif /**< __SOBJECT_I__H__ */
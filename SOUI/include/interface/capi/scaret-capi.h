#ifndef __SCARET_CAPI_H__
#define __SCARET_CAPI_H__

#include "../scaret-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for ICaret Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * ICaret inherits from IObject.
 */

/* IObjRef base interface macros */
#define ICaret_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ICaret_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ICaret_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IObject base interface macros */
#define ICaret_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define ICaret_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define ICaret_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define ICaret_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define ICaret_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define ICaret_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define ICaret_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define ICaret_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define ICaret_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define ICaret_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define ICaret_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define ICaret_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define ICaret_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define ICaret_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define ICaret_SetAttribute_Default(This, pszAttr, pszValue) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, FALSE))

#define ICaret_SetAttributeA_Default(This, pszAttr, pszValue) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, FALSE))

#define ICaret_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define ICaret_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define ICaret_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

/* ICaret specific interface macros */
#define ICaret_Init(This, hBmp, nWid, nHei) \
    ((This)->lpVtbl->Init(This, hBmp, nWid, nHei))

#define ICaret_SetPosition(This, x, y) \
    ((This)->lpVtbl->SetPosition(This, x, y))

#define ICaret_SetVisible(This, bVisible, owner) \
    ((This)->lpVtbl->SetVisible(This, bVisible, owner))

#define ICaret_IsVisible(This) \
    ((This)->lpVtbl->IsVisible(This))

#define ICaret_Draw(This, pRT) \
    ((This)->lpVtbl->Draw(This, pRT))

#define ICaret_GetRect(This) \
    ((This)->lpVtbl->GetRect(This))

#ifdef __cplusplus
}
#endif

#endif /* __SCARET_CAPI_H__ */

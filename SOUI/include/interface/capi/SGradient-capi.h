#ifndef __SGRADIENT_CAPI_H__
#define __SGRADIENT_CAPI_H__

#include "../SGradient-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SGradient Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * IGradient inherits from IObject.
 */

/* IObject base interface macros */
#define IGradient_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IGradient_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IGradient_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IGradient_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IGradient_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define IGradient_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IGradient_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IGradient_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IGradient_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define IGradient_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define IGradient_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define IGradient_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define IGradient_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define IGradient_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define IGradient_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define IGradient_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define IGradient_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define IGradient_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define IGradient_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define IGradient_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

/* IGradient specific interface macros */
#define IGradient_GetGradientData(This) \
    ((This)->lpVtbl->GetGradientData(This))

#define IGradient_GetGradientLength(This) \
    ((This)->lpVtbl->GetGradientLength(This))

#ifdef __cplusplus
}
#endif

#endif /* __SGRADIENT_CAPI_H__ */

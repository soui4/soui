#ifndef __SINTERPOLATOR_CAPI_H__
#define __SINTERPOLATOR_CAPI_H__

#include "../sinterpolator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Interpolator constants
 */
#define INTERPOLATOR_INPUT_MIN      0.0f
#define INTERPOLATOR_INPUT_MAX      1.0f

/*
 * C API Helper Macros for sinterpolator Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IInterpolator C API Macros (IObject base interface) */
#define IInterpolator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IInterpolator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IInterpolator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IInterpolator_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IInterpolator_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define IInterpolator_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IInterpolator_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IInterpolator_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IInterpolator_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define IInterpolator_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define IInterpolator_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define IInterpolator_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define IInterpolator_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define IInterpolator_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define IInterpolator_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define IInterpolator_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define IInterpolator_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define IInterpolator_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define IInterpolator_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define IInterpolator_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

/* IInterpolator specific interface macros */
#define IInterpolator_getInterpolation(This, input) \
    ((This)->lpVtbl->getInterpolation(This, input))

#ifdef __cplusplus
}
#endif

#endif /* __SINTERPOLATOR_CAPI_H__ */

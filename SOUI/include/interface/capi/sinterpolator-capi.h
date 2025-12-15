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

/* IInterpolator C API Macros */
#define IInterpolator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IInterpolator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IInterpolator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IInterpolator_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IInterpolator_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IInterpolator_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IInterpolator_GetClassInfo(This) \
    ((This)->lpVtbl->GetClassInfo(This))

#define IInterpolator_SetAttribute(This, strName, strValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, strName, strValue, bLoading))

#define IInterpolator_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define IInterpolator_GetAttribute(This, strName, strValue) \
    ((This)->lpVtbl->GetAttribute(This, strName, strValue))

#define IInterpolator_GetAttributeA(This, pszAttr, strValue) \
    ((This)->lpVtbl->GetAttributeA(This, pszAttr, strValue))

#define IInterpolator_AfterAttribute(This) \
    ((This)->lpVtbl->AfterAttribute(This))

#define IInterpolator_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define IInterpolator_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define IInterpolator_getInterpolation(This, input) \
    ((This)->lpVtbl->getInterpolation(This, input))

#ifdef __cplusplus
}
#endif

#endif /* __SINTERPOLATOR_CAPI_H__ */

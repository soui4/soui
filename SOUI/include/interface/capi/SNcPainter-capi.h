#ifndef __SNCPAINTER_CAPI_H__
#define __SNCPAINTER_CAPI_H__

#include "../SNcPainter-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SNcPainter Interface
 * These macros provide C-style function call syntax for C++ interface methods
 * INcPainter inherits from IObject.
 */

/* IObject base interface macros */
#define INcPainter_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define INcPainter_Release(This) \
    ((This)->lpVtbl->Release(This))

#define INcPainter_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define INcPainter_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define INcPainter_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define INcPainter_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define INcPainter_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define INcPainter_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define INcPainter_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define INcPainter_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define INcPainter_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define INcPainter_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define INcPainter_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define INcPainter_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define INcPainter_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define INcPainter_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define INcPainter_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define INcPainter_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define INcPainter_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define INcPainter_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

/* INcPainter specific interface macros */
#define INcPainter_GetRoot(This) \
    ((This)->lpVtbl->GetRoot(This))

#ifdef __cplusplus
}
#endif

#endif /* __SNCPAINTER_CAPI_H__ */

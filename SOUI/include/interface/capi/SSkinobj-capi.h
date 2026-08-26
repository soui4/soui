#ifndef __SSKINOBJ_CAPI_H__
#define __SSKINOBJ_CAPI_H__

#include "../SSkinobj-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SSkinobj Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ISkinObj C API Macros (IObject base interface methods) */
#define ISkinObj_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ISkinObj_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ISkinObj_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ISkinObj_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define ISkinObj_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define ISkinObj_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define ISkinObj_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define ISkinObj_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define ISkinObj_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define ISkinObj_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define ISkinObj_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define ISkinObj_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define ISkinObj_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define ISkinObj_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define ISkinObj_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define ISkinObj_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define ISkinObj_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define ISkinObj_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define ISkinObj_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define ISkinObj_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

/* ISkinObj C API Macros (ISkinObj specific methods) */
#define ISkinObj_DrawByState2(This, pRT, rcDraw, dwState, byAlpha) \
    ((This)->lpVtbl->DrawByState2(This, pRT, rcDraw, dwState, byAlpha))

#define ISkinObj_DrawByState(This, pRT, rcDraw, dwState) \
    ((This)->lpVtbl->DrawByState(This, pRT, rcDraw, dwState))

#define ISkinObj_DrawByIndex2(This, pRT, rcDraw, iState, byAlpha) \
    ((This)->lpVtbl->DrawByIndex2(This, pRT, rcDraw, iState, byAlpha))

#define ISkinObj_DrawByIndex(This, pRT, rcDraw, iState) \
    ((This)->lpVtbl->DrawByIndex(This, pRT, rcDraw, iState))

#define ISkinObj_GetSkinSize(This) \
    ((This)->lpVtbl->GetSkinSize(This))

#define ISkinObj_GetStates(This) \
    ((This)->lpVtbl->GetStates(This))

#define ISkinObj_GetAlpha(This) \
    ((This)->lpVtbl->GetAlpha(This))

#define ISkinObj_SetAlpha(This, byAlpha) \
    ((This)->lpVtbl->SetAlpha(This, byAlpha))

#define ISkinObj_OnColorize(This, cr) \
    ((This)->lpVtbl->OnColorize(This, cr))

#define ISkinObj_GetScale(This) \
    ((This)->lpVtbl->GetScale(This))

#define ISkinObj_SetScale(This, scale) \
    ((This)->lpVtbl->SetScale(This, scale))

#define ISkinObj_Scale(This, nScale) \
    ((This)->lpVtbl->Scale(This, nScale))

#ifdef __cplusplus
}
#endif

#endif /* __SSKINOBJ_CAPI_H__ */

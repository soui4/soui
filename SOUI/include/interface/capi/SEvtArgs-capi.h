#ifndef __SEVTARGS_CAPI_H__
#define __SEVTARGS_CAPI_H__

#include "../SEvtArgs-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SEvtArgs Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IEvtArgs C API Macros */
#define IEvtArgs_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IEvtArgs_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IEvtArgs_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IEvtArgs_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IEvtArgs_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define IEvtArgs_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IEvtArgs_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IEvtArgs_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IEvtArgs_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define IEvtArgs_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define IEvtArgs_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define IEvtArgs_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define IEvtArgs_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define IEvtArgs_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define IEvtArgs_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define IEvtArgs_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define IEvtArgs_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define IEvtArgs_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define IEvtArgs_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define IEvtArgs_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

#define IEvtArgs_Sender(This) \
    ((This)->lpVtbl->Sender(This))

#define IEvtArgs_IdFrom(This) \
    ((This)->lpVtbl->IdFrom(This))

#define IEvtArgs_SetIdFrom(This, id) \
    ((This)->lpVtbl->SetIdFrom(This, id))

#define IEvtArgs_NameFrom(This) \
    ((This)->lpVtbl->NameFrom(This))

#define IEvtArgs_SetNameFrom(This, name) \
    ((This)->lpVtbl->SetNameFrom(This, name))

#define IEvtArgs_IsBubbleUp(This) \
    ((This)->lpVtbl->IsBubbleUp(This))

#define IEvtArgs_SetBubbleUp(This, bBubbleUp) \
    ((This)->lpVtbl->SetBubbleUp(This, bBubbleUp))

#define IEvtArgs_HandleCount(This) \
    ((This)->lpVtbl->HandleCount(This))

#define IEvtArgs_IncreaseHandleCount(This) \
    ((This)->lpVtbl->IncreaseHandleCount(This))

#define IEvtArgs_Data(This) \
    ((This)->lpVtbl->Data(This))

/* IEvtSlot C API Macros */
#define IEvtSlot_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IEvtSlot_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IEvtSlot_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IEvtSlot_Run(This, pArg) \
    ((This)->lpVtbl->Run(This, pArg))

#define IEvtSlot_Clone(This) \
    ((This)->lpVtbl->Clone(This))

#define IEvtSlot_Equal(This, sour) \
    ((This)->lpVtbl->Equal(This, sour))

#define IEvtSlot_GetSlotType(This) \
    ((This)->lpVtbl->GetSlotType(This))

#ifdef __cplusplus
}
#endif

#endif /* __SEVTARGS_CAPI_H__ */

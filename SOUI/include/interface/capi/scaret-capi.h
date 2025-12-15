#ifndef __SCARET_CAPI_H__
#define __SCARET_CAPI_H__

#include "../scaret-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IObjRef base interface macros */
#define ICaret_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ICaret_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ICaret_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IObject base interface macros */
#define ICaret_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define ICaret_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define ICaret_GetObjectInfo(This, pInfo) \
    ((This)->lpVtbl->GetObjectInfo(This, pInfo))

#define ICaret_OnLanguageChange(This) \
    ((This)->lpVtbl->OnLanguageChange(This))

#define ICaret_SetLanguage(This, lang) \
    ((This)->lpVtbl->SetLanguage(This, lang))

#define ICaret_GetLanguage(This) \
    ((This)->lpVtbl->GetLanguage(This))

#define ICaret_GetTranslator(This) \
    ((This)->lpVtbl->GetTranslator(This))

#define ICaret_InitTranslator(This) \
    ((This)->lpVtbl->InitTranslator(This))

#define ICaret_GetScale(This) \
    ((This)->lpVtbl->GetScale(This))

#define ICaret_SetScale(This, scale) \
    ((This)->lpVtbl->SetScale(This, scale))

#define ICaret_InitEventMap(This) \
    ((This)->lpVtbl->InitEventMap(This))

#define ICaret_FireEvent(This, e) \
    ((This)->lpVtbl->FireEvent(This, e))

#define ICaret_SubscribeEvent(This, evtId, subscriber) \
    ((This)->lpVtbl->SubscribeEvent(This, evtId, subscriber))

#define ICaret_UnsubscribeEvent(This, evtId, subscriber) \
    ((This)->lpVtbl->UnsubscribeEvent(This, evtId, subscriber))

#define ICaret_SetAttribute(This, pAttr) \
    ((This)->lpVtbl->SetAttribute(This, pAttr))

#define ICaret_SetAttributeW(This, attr, value) \
    ((This)->lpVtbl->SetAttributeW(This, attr, value))

#define ICaret_GetAttributeW(This, attr, value, nBufLen) \
    ((This)->lpVtbl->GetAttributeW(This, attr, value, nBufLen))

#define ICaret_OnAttribute(This, attr, value) \
    ((This)->lpVtbl->OnAttribute(This, attr, value))

#define ICaret_OnAttributeValue(This, attr, value) \
    ((This)->lpVtbl->OnAttributeValue(This, attr, value))

#define ICaret_OnParsed(This) \
    ((This)->lpVtbl->OnParsed(This))

#define ICaret_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define ICaret_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define ICaret_SetID(This, id) \
    ((This)->lpVtbl->SetID(This, id))

#define ICaret_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define ICaret_SetName(This, name) \
    ((This)->lpVtbl->SetName(This, name))

#define ICaret_GetObjectTypeName(This) \
    ((This)->lpVtbl->GetObjectTypeName(This))

#define ICaret_Destroy(This) \
    ((This)->lpVtbl->Destroy(This))

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

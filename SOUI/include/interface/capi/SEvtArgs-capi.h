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

#define IEvtArgs_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IEvtArgs_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IEvtArgs_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IEvtArgs_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IEvtArgs_SetID(This, uID) \
    ((This)->lpVtbl->SetID(This, uID))

#define IEvtArgs_InitFromXml(This, pNode) \
    ((This)->lpVtbl->InitFromXml(This, pNode))

#define IEvtArgs_OnInitFinished(This, pNode) \
    ((This)->lpVtbl->OnInitFinished(This, pNode))

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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IEvtArgs Helper Functions */
static inline long IEvtArgs_AddRef_C(IEvtArgs* pThis)
{
    return IEvtArgs_AddRef(pThis);
}

static inline long IEvtArgs_Release_C(IEvtArgs* pThis)
{
    return IEvtArgs_Release(pThis);
}

static inline IObject* IEvtArgs_Sender_C(IEvtArgs* pThis)
{
    return IEvtArgs_Sender(pThis);
}

static inline int IEvtArgs_IdFrom_C(IEvtArgs* pThis)
{
    return IEvtArgs_IdFrom(pThis);
}

static inline void IEvtArgs_SetIdFrom_C(IEvtArgs* pThis, int id)
{
    IEvtArgs_SetIdFrom(pThis, id);
}

static inline LPCWSTR IEvtArgs_NameFrom_C(IEvtArgs* pThis)
{
    return IEvtArgs_NameFrom(pThis);
}

static inline void IEvtArgs_SetNameFrom_C(IEvtArgs* pThis, LPCWSTR name)
{
    IEvtArgs_SetNameFrom(pThis, name);
}

static inline BOOL IEvtArgs_IsBubbleUp_C(IEvtArgs* pThis)
{
    return IEvtArgs_IsBubbleUp(pThis);
}

static inline void IEvtArgs_SetBubbleUp_C(IEvtArgs* pThis, BOOL bBubbleUp)
{
    IEvtArgs_SetBubbleUp(pThis, bBubbleUp);
}

static inline UINT IEvtArgs_HandleCount_C(IEvtArgs* pThis)
{
    return IEvtArgs_HandleCount(pThis);
}

static inline void IEvtArgs_IncreaseHandleCount_C(IEvtArgs* pThis)
{
    IEvtArgs_IncreaseHandleCount(pThis);
}

static inline LPVOID IEvtArgs_Data_C(IEvtArgs* pThis)
{
    return IEvtArgs_Data(pThis);
}

/* IEvtSlot Helper Functions */
static inline long IEvtSlot_AddRef_C(IEvtSlot* pThis)
{
    return IEvtSlot_AddRef(pThis);
}

static inline long IEvtSlot_Release_C(IEvtSlot* pThis)
{
    return IEvtSlot_Release(pThis);
}

static inline BOOL IEvtSlot_Run_C(IEvtSlot* pThis, IEvtArgs* pArg)
{
    return IEvtSlot_Run(pThis, pArg);
}

static inline IEvtSlot* IEvtSlot_Clone_C(IEvtSlot* pThis)
{
    return IEvtSlot_Clone(pThis);
}

static inline BOOL IEvtSlot_Equal_C(IEvtSlot* pThis, const IEvtSlot* sour)
{
    return IEvtSlot_Equal(pThis, sour);
}

static inline UINT IEvtSlot_GetSlotType_C(IEvtSlot* pThis)
{
    return IEvtSlot_GetSlotType(pThis);
}

/*
 * Convenience macros for common event operations
 */

/* Event bubble control */
#define IEvtArgs_StopBubble(This) \
    IEvtArgs_SetBubbleUp(This, FALSE)

#define IEvtArgs_AllowBubble(This) \
    IEvtArgs_SetBubbleUp(This, TRUE)

/* Event data access with type casting */
#define IEvtArgs_GetDataAs(This, Type) \
    ((Type*)IEvtArgs_Data(This))

/* Event source information */
#define IEvtArgs_GetSenderId(This) \
    IEvtArgs_IdFrom(This)

#define IEvtArgs_GetSenderName(This) \
    IEvtArgs_NameFrom(This)

#define IEvtArgs_SetSenderId(This, id) \
    IEvtArgs_SetIdFrom(This, id)

#define IEvtArgs_SetSenderName(This, name) \
    IEvtArgs_SetNameFrom(This, name)

/* Event handling status */
#define IEvtArgs_IsHandled(This) \
    (IEvtArgs_HandleCount(This) > 0)

#define IEvtArgs_MarkHandled(This) \
    IEvtArgs_IncreaseHandleCount(This)

/* Safe event slot operations */
#define IEvtSlot_SafeRun(This, pArg) \
    (IObjRef_IsValidInterface(This) && (pArg) ? IEvtSlot_Run(This, pArg) : FALSE)

#define IEvtSlot_SafeClone(This, ppClone) \
    do { \
        if (IObjRef_IsValidInterface(This) && (ppClone)) { \
            *(ppClone) = IEvtSlot_Clone(This); \
        } else if (ppClone) { \
            *(ppClone) = NULL; \
        } \
    } while(0)

/*
 * Event callback function wrapper
 */
static inline BOOL IEvtArgs_RunCallback_C(IEvtArgs* pEvt, FunCallback callback, void* ctx)
{
    if (callback && pEvt) {
        return callback(pEvt, ctx);
    }
    return FALSE;
}

/*
 * Reference counting helpers
 */
#define IEvtArgs_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IEvtArgs_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IEvtSlot_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IEvtSlot_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#ifdef __cplusplus
}
#endif

#endif /* __SEVTARGS_CAPI_H__ */

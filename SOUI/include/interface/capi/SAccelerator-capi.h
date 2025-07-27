#ifndef __SACCELERATOR_CAPI_H__
#define __SACCELERATOR_CAPI_H__

#include "../SAccelerator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for IAccelerator Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IAccelerator C API Macros */
#define IAccelerator_GetModifier(This) \
    ((This)->lpVtbl->GetModifier(This))

#define IAccelerator_GetKey(This) \
    ((This)->lpVtbl->GetKey(This))

#define IAccelerator_GetAcc(This) \
    ((This)->lpVtbl->GetAcc(This))

/* IAcceleratorTarget C API Macros */
#define IAcceleratorTarget_OnAcceleratorPressed(This, acc) \
    ((This)->lpVtbl->OnAcceleratorPressed(This, acc))

/* IAcceleratorMgr C API Macros */
#define IAcceleratorMgr_RegisterAccelerator(This, pAcc, target) \
    ((This)->lpVtbl->RegisterAccelerator(This, pAcc, target))

#define IAcceleratorMgr_UnregisterAccelerator(This, pAcc, target) \
    ((This)->lpVtbl->UnregisterAccelerator(This, pAcc, target))

#define IAcceleratorMgr_UnregisterAccelerators(This, target) \
    ((This)->lpVtbl->UnregisterAccelerators(This, target))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 * These functions provide an alternative C-style API
 */

/* IAccelerator Helper Functions */
static inline WORD IAccelerator_GetModifier_C(IAccelerator* pThis)
{
    return IAccelerator_GetModifier(pThis);
}

static inline WORD IAccelerator_GetKey_C(IAccelerator* pThis)
{
    return IAccelerator_GetKey(pThis);
}

static inline DWORD IAccelerator_GetAcc_C(IAccelerator* pThis)
{
    return IAccelerator_GetAcc(pThis);
}

/* IAcceleratorTarget Helper Functions */
static inline BOOL IAcceleratorTarget_OnAcceleratorPressed_C(IAcceleratorTarget* pThis, const IAccelerator* acc)
{
    return IAcceleratorTarget_OnAcceleratorPressed(pThis, acc);
}

/* IAcceleratorMgr Helper Functions */
static inline void IAcceleratorMgr_RegisterAccelerator_C(IAcceleratorMgr* pThis, const IAccelerator* pAcc, IAcceleratorTarget* target)
{
    IAcceleratorMgr_RegisterAccelerator(pThis, pAcc, target);
}

static inline void IAcceleratorMgr_UnregisterAccelerator_C(IAcceleratorMgr* pThis, const IAccelerator* pAcc, IAcceleratorTarget* target)
{
    IAcceleratorMgr_UnregisterAccelerator(pThis, pAcc, target);
}

static inline void IAcceleratorMgr_UnregisterAccelerators_C(IAcceleratorMgr* pThis, IAcceleratorTarget* target)
{
    IAcceleratorMgr_UnregisterAccelerators(pThis, target);
}

#ifdef __cplusplus
}
#endif

#endif /* __SACCELERATOR_CAPI_H__ */

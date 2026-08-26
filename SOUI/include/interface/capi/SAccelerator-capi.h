#ifndef __SACCELERATOR_CAPI_H__
#define __SACCELERATOR_CAPI_H__

#include "../SAccelerator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SAccelerator Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 * IAccelerator, IAcceleratorTarget and IAcceleratorMgr are declared with DECLARE_INTERFACE
 * (no parent interface, no AddRef/Release/OnFinalRelease).
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

#ifdef __cplusplus
}
#endif

#endif /* __SACCELERATOR_CAPI_H__ */

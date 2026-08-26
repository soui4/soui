#ifndef __SCTRL_CAPI_H__
#define __SCTRL_CAPI_H__

#include "../SCtrls-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SCtrl Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 *
 * Interfaces covered:
 *   ICtrl      (inherits IObjRef)
 *   IOsrPanel  (inherits ICtrl)
 *   IItemPanel (inherits IOsrPanel)
 */

/* ICtrl C API Macros */
#define ICtrl_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ICtrl_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ICtrl_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ICtrl_ToIWindow(This) \
    ((This)->lpVtbl->ToIWindow(This))

/* IOsrPanel C API Macros */
#define IOsrPanel_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IOsrPanel_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IOsrPanel_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IOsrPanel_ToIWindow(This) \
    ((This)->lpVtbl->ToIWindow(This))

#define IOsrPanel_SetItemIndex(This, index) \
    ((This)->lpVtbl->SetItemIndex(This, index))

#define IOsrPanel_GetItemIndex(This) \
    ((This)->lpVtbl->GetItemIndex(This))

#define IOsrPanel_SetItemData(This, dwData) \
    ((This)->lpVtbl->SetItemData(This, dwData))

#define IOsrPanel_GetItemData(This) \
    ((This)->lpVtbl->GetItemData(This))

#define IOsrPanel_PtToHost(This, pt) \
    ((This)->lpVtbl->PtToHost(This, pt))

/* IItemPanel C API Macros */
#define IItemPanel_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IItemPanel_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IItemPanel_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IItemPanel_ToIWindow(This) \
    ((This)->lpVtbl->ToIWindow(This))

#define IItemPanel_SetItemIndex(This, index) \
    ((This)->lpVtbl->SetItemIndex(This, index))

#define IItemPanel_GetItemIndex(This) \
    ((This)->lpVtbl->GetItemIndex(This))

#define IItemPanel_SetItemData(This, dwData) \
    ((This)->lpVtbl->SetItemData(This, dwData))

#define IItemPanel_GetItemData(This) \
    ((This)->lpVtbl->GetItemData(This))

#define IItemPanel_PtToHost(This, pt) \
    ((This)->lpVtbl->PtToHost(This, pt))

#define IItemPanel_SetSkin(This, pSkin) \
    ((This)->lpVtbl->SetSkin(This, pSkin))

#define IItemPanel_SetColor(This, crBk, crSelBk) \
    ((This)->lpVtbl->SetColor(This, crBk, crSelBk))

#define IItemPanel_IsSelected(This) \
    ((This)->lpVtbl->IsSelected(This))

#define IItemPanel_SetSelected(This, bSelected, bUpdate) \
    ((This)->lpVtbl->SetSelected(This, bSelected, bUpdate))

#ifdef __cplusplus
}
#endif

#endif /* __SCTRL_CAPI_H__ */

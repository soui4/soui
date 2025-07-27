#ifndef __SCTRL_CAPI_H__
#define __SCTRL_CAPI_H__

#include "../SCtrl-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SCtrl Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 * 
 * Note: This file covers the main control interfaces. Due to the large number
 * of control interfaces in SCtrl-i.h (3475+ lines), this implementation focuses
 * on the most commonly used base interfaces.
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

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* ICtrl Helper Functions */
static inline long ICtrl_AddRef_C(ICtrl* pThis)
{
    return ICtrl_AddRef(pThis);
}

static inline long ICtrl_Release_C(ICtrl* pThis)
{
    return ICtrl_Release(pThis);
}

static inline IWindow* ICtrl_ToIWindow_C(ICtrl* pThis)
{
    return ICtrl_ToIWindow(pThis);
}

/* IOsrPanel Helper Functions */
static inline long IOsrPanel_AddRef_C(IOsrPanel* pThis)
{
    return IOsrPanel_AddRef(pThis);
}

static inline long IOsrPanel_Release_C(IOsrPanel* pThis)
{
    return IOsrPanel_Release(pThis);
}

static inline IWindow* IOsrPanel_ToIWindow_C(IOsrPanel* pThis)
{
    return IOsrPanel_ToIWindow(pThis);
}

static inline void IOsrPanel_SetItemIndex_C(IOsrPanel* pThis, LPARAM index)
{
    IOsrPanel_SetItemIndex(pThis, index);
}

static inline LPARAM IOsrPanel_GetItemIndex_C(IOsrPanel* pThis)
{
    return IOsrPanel_GetItemIndex(pThis);
}

static inline void IOsrPanel_SetItemData_C(IOsrPanel* pThis, LPARAM dwData)
{
    IOsrPanel_SetItemData(pThis, dwData);
}

static inline LPARAM IOsrPanel_GetItemData_C(IOsrPanel* pThis)
{
    return IOsrPanel_GetItemData(pThis);
}

static inline void IOsrPanel_PtToHost_C(IOsrPanel* pThis, POINT* pt)
{
    IOsrPanel_PtToHost(pThis, pt);
}

/* IItemPanel Helper Functions */
static inline long IItemPanel_AddRef_C(IItemPanel* pThis)
{
    return IItemPanel_AddRef(pThis);
}

static inline long IItemPanel_Release_C(IItemPanel* pThis)
{
    return IItemPanel_Release(pThis);
}

static inline IWindow* IItemPanel_ToIWindow_C(IItemPanel* pThis)
{
    return IItemPanel_ToIWindow(pThis);
}

static inline void IItemPanel_SetSkin_C(IItemPanel* pThis, ISkinObj* pSkin)
{
    IItemPanel_SetSkin(pThis, pSkin);
}

/*
 * Convenience macros for common control operations
 */

/* Control conversion shortcuts */
#define ICtrl_AsWindow(This) \
    ICtrl_ToIWindow(This)

#define IOsrPanel_AsWindow(This) \
    IOsrPanel_ToIWindow(This)

#define IItemPanel_AsWindow(This) \
    IItemPanel_ToIWindow(This)

/* Item index shortcuts */
#define IOsrPanel_SetIndex(This, index) \
    IOsrPanel_SetItemIndex(This, index)

#define IOsrPanel_GetIndex(This) \
    IOsrPanel_GetItemIndex(This)

#define IItemPanel_SetIndex(This, index) \
    IItemPanel_SetItemIndex(This, index)

#define IItemPanel_GetIndex(This) \
    IItemPanel_GetItemIndex(This)

/* Item data shortcuts */
#define IOsrPanel_SetData(This, data) \
    IOsrPanel_SetItemData(This, data)

#define IOsrPanel_GetData(This) \
    IOsrPanel_GetItemData(This)

#define IItemPanel_SetData(This, data) \
    IItemPanel_SetItemData(This, data)

#define IItemPanel_GetData(This) \
    IItemPanel_GetItemData(This)

/* Safe control operations */
#define ICtrl_SafeToIWindow(This, ppWindow) \
    do { \
        if ((This) && (ppWindow)) { \
            *(ppWindow) = ICtrl_ToIWindow(This); \
        } else if (ppWindow) { \
            *(ppWindow) = NULL; \
        } \
    } while(0)

#define IOsrPanel_SafeSetItemIndex(This, index) \
    do { \
        if (This) { \
            IOsrPanel_SetItemIndex(This, index); \
        } \
    } while(0)

#define IOsrPanel_SafeSetItemData(This, data) \
    do { \
        if (This) { \
            IOsrPanel_SetItemData(This, data); \
        } \
    } while(0)

#define IItemPanel_SafeSetSkin(This, skin) \
    do { \
        if ((This) && (skin)) { \
            IItemPanel_SetSkin(This, skin); \
        } \
    } while(0)

/*
 * Control helper functions
 */
static inline BOOL ICtrl_IsValid_C(ICtrl* pThis)
{
    return (pThis != NULL);
}

static inline IWindow* ICtrl_GetWindow_C(ICtrl* pThis)
{
    return pThis ? ICtrl_ToIWindow(pThis) : NULL;
}

static inline BOOL IOsrPanel_HasValidIndex_C(IOsrPanel* pThis)
{
    return pThis ? (IOsrPanel_GetItemIndex(pThis) != -1) : FALSE;
}

static inline BOOL IOsrPanel_HasItemData_C(IOsrPanel* pThis)
{
    return pThis ? (IOsrPanel_GetItemData(pThis) != 0) : FALSE;
}

static inline POINT IOsrPanel_ConvertPointToHost_C(IOsrPanel* pThis, POINT pt)
{
    if (pThis) {
        IOsrPanel_PtToHost(pThis, &pt);
    }
    return pt;
}

/*
 * Control state management
 */
typedef struct CtrlState {
    ICtrl* ctrl;
    IWindow* window;
    BOOL isValid;
} CtrlState;

static inline void CtrlState_Init(CtrlState* state, ICtrl* ctrl)
{
    if (state) {
        state->ctrl = ctrl;
        state->window = ctrl ? ICtrl_ToIWindow(ctrl) : NULL;
        state->isValid = (ctrl != NULL && state->window != NULL);
    }
}

static inline void CtrlState_Update(CtrlState* state)
{
    if (state && state->ctrl) {
        state->window = ICtrl_ToIWindow(state->ctrl);
        state->isValid = (state->window != NULL);
    }
}

typedef struct OsrPanelState {
    IOsrPanel* panel;
    IWindow* window;
    LPARAM itemIndex;
    LPARAM itemData;
    BOOL isValid;
} OsrPanelState;

static inline void OsrPanelState_Init(OsrPanelState* state, IOsrPanel* panel)
{
    if (state) {
        state->panel = panel;
        state->window = panel ? IOsrPanel_ToIWindow(panel) : NULL;
        state->itemIndex = panel ? IOsrPanel_GetItemIndex(panel) : -1;
        state->itemData = panel ? IOsrPanel_GetItemData(panel) : 0;
        state->isValid = (panel != NULL && state->window != NULL);
    }
}

static inline void OsrPanelState_Update(OsrPanelState* state)
{
    if (state && state->panel) {
        state->window = IOsrPanel_ToIWindow(state->panel);
        state->itemIndex = IOsrPanel_GetItemIndex(state->panel);
        state->itemData = IOsrPanel_GetItemData(state->panel);
        state->isValid = (state->window != NULL);
    }
}

static inline void OsrPanelState_SetIndex(OsrPanelState* state, LPARAM index)
{
    if (state && state->panel) {
        IOsrPanel_SetItemIndex(state->panel, index);
        state->itemIndex = index;
    }
}

static inline void OsrPanelState_SetData(OsrPanelState* state, LPARAM data)
{
    if (state && state->panel) {
        IOsrPanel_SetItemData(state->panel, data);
        state->itemData = data;
    }
}

/*
 * Reference counting helpers
 */
#define ICtrl_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define ICtrl_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IOsrPanel_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IOsrPanel_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IItemPanel_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IItemPanel_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

/*
 * Control type checking helpers
 */
static inline BOOL ICtrl_IsOsrPanel_C(ICtrl* pThis)
{
    // This would require QueryInterface or similar mechanism
    // For now, return FALSE as we can't determine type without additional info
    return FALSE;
}

static inline BOOL ICtrl_IsItemPanel_C(ICtrl* pThis)
{
    // This would require QueryInterface or similar mechanism
    return FALSE;
}

/*
 * Note: Additional control interfaces from SCtrl-i.h can be added here
 * as needed. The file contains many specialized control interfaces such as:
 * - IListBox, IComboBox, IEdit, IButton
 * - ITreeView, IListView, ITabCtrl
 * - IProgressBar, ISlider, ISpinButton
 * - And many more...
 * 
 * Each can be implemented following the same pattern as above.
 */

#ifdef __cplusplus
}
#endif

#endif /* __SCTRL_CAPI_H__ */

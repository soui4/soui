#ifndef __SWNDCONTAINER_CAPI_H__
#define __SWNDCONTAINER_CAPI_H__

#include "../SWndContainer-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SWndContainer Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* ISwndContainer C API Macros */
#define ISwndContainer_RegisterTimelineHandler(This, pHandler) \
    ((This)->lpVtbl->RegisterTimelineHandler(This, pHandler))

#define ISwndContainer_UnregisterTimelineHandler(This, pHandler) \
    ((This)->lpVtbl->UnregisterTimelineHandler(This, pHandler))

#define ISwndContainer_RegisterDragDrop(This, swnd, pDropTarget) \
    ((This)->lpVtbl->RegisterDragDrop(This, swnd, pDropTarget))

#define ISwndContainer_UnregisterDragDrop(This, swnd) \
    ((This)->lpVtbl->UnregisterDragDrop(This, swnd))

#define ISwndContainer_OnFireEvent(This, evt) \
    ((This)->lpVtbl->OnFireEvent(This, evt))

#define ISwndContainer_GetMsgLoop(This) \
    ((This)->lpVtbl->GetMsgLoop(This))

#define ISwndContainer_GetScriptModule(This) \
    ((This)->lpVtbl->GetScriptModule(This))

#define ISwndContainer_GetHostHwnd(This) \
    ((This)->lpVtbl->GetHostHwnd(This))

#define ISwndContainer_IsTranslucent(This) \
    ((This)->lpVtbl->IsTranslucent(This))

#define ISwndContainer_IsSendWheel2Hover(This) \
    ((This)->lpVtbl->IsSendWheel2Hover(This))

#define ISwndContainer_GetScale(This) \
    ((This)->lpVtbl->GetScale(This))

#define ISwndContainer_GetTranslatorContext(This) \
    ((This)->lpVtbl->GetTranslatorContext(This))

#define ISwndContainer_GetContainerRect(This, ret) \
    ((This)->lpVtbl->GetContainerRect(This, ret))

#define ISwndContainer_UpdateRegion(This, rgn) \
    ((This)->lpVtbl->UpdateRegion(This, rgn))

#define ISwndContainer_OnRedraw(This, rc, bClip) \
    ((This)->lpVtbl->OnRedraw(This, rc, bClip))

#define ISwndContainer_OnGetSwndCapture(This) \
    ((This)->lpVtbl->OnGetSwndCapture(This))

#define ISwndContainer_OnSetSwndCapture(This, swnd) \
    ((This)->lpVtbl->OnSetSwndCapture(This, swnd))

#define ISwndContainer_OnReleaseSwndCapture(This) \
    ((This)->lpVtbl->OnReleaseSwndCapture(This))

#define ISwndContainer_GetHover(This) \
    ((This)->lpVtbl->GetHover(This))

#define ISwndContainer_GetFocus(This) \
    ((This)->lpVtbl->GetFocus(This))

#define ISwndContainer_OnSetSwndFocus(This, swnd) \
    ((This)->lpVtbl->OnSetSwndFocus(This, swnd))

#define ISwndContainer_UpdateWindow(This, bForce) \
    ((This)->lpVtbl->UpdateWindow(This, bForce))

#define ISwndContainer_UpdateWindow_Default(This) \
    ((This)->lpVtbl->UpdateWindow(This, TRUE))

#define ISwndContainer_UpdateTooltip(This) \
    ((This)->lpVtbl->UpdateTooltip(This))

#define ISwndContainer_SetToolTip(This, rc, tipAlign, pszTip) \
    ((This)->lpVtbl->SetToolTip(This, rc, tipAlign, pszTip))

#define ISwndContainer_FrameToHost(This, rc) \
    ((This)->lpVtbl->FrameToHost(This, rc))

#define ISwndContainer_GetAcceleratorMgr(This) \
    ((This)->lpVtbl->GetAcceleratorMgr(This))

#define ISwndContainer_RegisterTrackMouseEvent(This, swnd) \
    ((This)->lpVtbl->RegisterTrackMouseEvent(This, swnd))

#define ISwndContainer_UnregisterTrackMouseEvent(This, swnd) \
    ((This)->lpVtbl->UnregisterTrackMouseEvent(This, swnd))

#define ISwndContainer_MarkWndTreeZorderDirty(This) \
    ((This)->lpVtbl->MarkWndTreeZorderDirty(This))

#define ISwndContainer_BuildWndTreeZorder(This) \
    ((This)->lpVtbl->BuildWndTreeZorder(This))

#define ISwndContainer_EnableIME(This, bEnable) \
    ((This)->lpVtbl->EnableIME(This, bEnable))

#define ISwndContainer_OnUpdateCursor(This) \
    ((This)->lpVtbl->OnUpdateCursor(This))

#define ISwndContainer_RegisterVideoCanvas(This, swnd) \
    ((This)->lpVtbl->RegisterVideoCanvas(This, swnd))

#define ISwndContainer_UnregisterVideoCanvas(This, swnd) \
    ((This)->lpVtbl->UnregisterVideoCanvas(This, swnd))

#define ISwndContainer_EnableHostPrivateUiDef(This, bEnable) \
    ((This)->lpVtbl->EnableHostPrivateUiDef(This, bEnable))

#define ISwndContainer_PostTask(This, runable, bAsync) \
    ((This)->lpVtbl->PostTask(This, runable, bAsync))

#define ISwndContainer_PostTask_Default(This, runable) \
    ((This)->lpVtbl->PostTask(This, runable, TRUE))

#define ISwndContainer_RemoveTasksForObject(This, pObj) \
    ((This)->lpVtbl->RemoveTasksForObject(This, pObj))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* Timeline Handler Management */
static inline BOOL ISwndContainer_RegisterTimelineHandler_C(ISwndContainer* pThis, ITimelineHandler* pHandler)
{
    return ISwndContainer_RegisterTimelineHandler(pThis, pHandler);
}

static inline BOOL ISwndContainer_UnregisterTimelineHandler_C(ISwndContainer* pThis, ITimelineHandler* pHandler)
{
    return ISwndContainer_UnregisterTimelineHandler(pThis, pHandler);
}

/* Drag and Drop Management */
static inline BOOL ISwndContainer_RegisterDragDrop_C(ISwndContainer* pThis, SWND swnd, IDropTarget* pDropTarget)
{
    return ISwndContainer_RegisterDragDrop(pThis, swnd, pDropTarget);
}

static inline BOOL ISwndContainer_UnregisterDragDrop_C(ISwndContainer* pThis, SWND swnd)
{
    return ISwndContainer_UnregisterDragDrop(pThis, swnd);
}

/* Event Management */
static inline BOOL ISwndContainer_OnFireEvent_C(ISwndContainer* pThis, IEvtArgs* evt)
{
    return ISwndContainer_OnFireEvent(pThis, evt);
}

/* Container Properties */
static inline IMessageLoop* ISwndContainer_GetMsgLoop_C(ISwndContainer* pThis)
{
    return ISwndContainer_GetMsgLoop(pThis);
}

static inline IScriptModule* ISwndContainer_GetScriptModule_C(ISwndContainer* pThis)
{
    return ISwndContainer_GetScriptModule(pThis);
}

static inline HWND ISwndContainer_GetHostHwnd_C(ISwndContainer* pThis)
{
    return ISwndContainer_GetHostHwnd(pThis);
}

static inline BOOL ISwndContainer_IsTranslucent_C(ISwndContainer* pThis)
{
    return ISwndContainer_IsTranslucent(pThis);
}

static inline int ISwndContainer_GetScale_C(ISwndContainer* pThis)
{
    return ISwndContainer_GetScale(pThis);
}

static inline LPCWSTR ISwndContainer_GetTranslatorContext_C(ISwndContainer* pThis)
{
    return ISwndContainer_GetTranslatorContext(pThis);
}

static inline void ISwndContainer_GetContainerRect_C(ISwndContainer* pThis, RECT* ret)
{
    ISwndContainer_GetContainerRect(pThis, ret);
}

/* Drawing and Update */
static inline void ISwndContainer_UpdateRegion_C(ISwndContainer* pThis, IRegionS* rgn)
{
    ISwndContainer_UpdateRegion(pThis, rgn);
}

static inline void ISwndContainer_OnRedraw_C(ISwndContainer* pThis, LPCRECT rc, BOOL bClip)
{
    ISwndContainer_OnRedraw(pThis, rc, bClip);
}

static inline BOOL ISwndContainer_UpdateWindow_C(ISwndContainer* pThis, BOOL bForce)
{
    return ISwndContainer_UpdateWindow(pThis, bForce);
}

/* Focus and Capture Management */
static inline SWND ISwndContainer_GetFocus_C(ISwndContainer* pThis)
{
    return ISwndContainer_GetFocus(pThis);
}

static inline void ISwndContainer_OnSetSwndFocus_C(ISwndContainer* pThis, SWND swnd)
{
    ISwndContainer_OnSetSwndFocus(pThis, swnd);
}

static inline SWND ISwndContainer_OnGetSwndCapture_C(ISwndContainer* pThis)
{
    return ISwndContainer_OnGetSwndCapture(pThis);
}

static inline SWND ISwndContainer_OnSetSwndCapture_C(ISwndContainer* pThis, SWND swnd)
{
    return ISwndContainer_OnSetSwndCapture(pThis, swnd);
}

static inline BOOL ISwndContainer_OnReleaseSwndCapture_C(ISwndContainer* pThis)
{
    return ISwndContainer_OnReleaseSwndCapture(pThis);
}

/* Task Management */
static inline BOOL ISwndContainer_PostTask_C(ISwndContainer* pThis, IRunnable* runable, BOOL bAsync)
{
    return ISwndContainer_PostTask(pThis, runable, bAsync);
}

static inline int ISwndContainer_RemoveTasksForObject_C(ISwndContainer* pThis, void* pObj)
{
    return ISwndContainer_RemoveTasksForObject(pThis, pObj);
}

/*
 * Convenience macros for common container operations
 */

/* Window update shortcuts */
#define ISwndContainer_ForceUpdate(This) \
    ISwndContainer_UpdateWindow(This, TRUE)

#define ISwndContainer_ConditionalUpdate(This) \
    ISwndContainer_UpdateWindow(This, FALSE)

#define ISwndContainer_Invalidate(This, rc) \
    ISwndContainer_OnRedraw(This, rc, TRUE)

#define ISwndContainer_InvalidateAll(This) \
    ISwndContainer_OnRedraw(This, NULL, FALSE)

/* Focus management shortcuts */
#define ISwndContainer_HasFocus(This) \
    (ISwndContainer_GetFocus(This) != 0)

#define ISwndContainer_ClearFocus(This) \
    ISwndContainer_OnSetSwndFocus(This, 0)

/* Capture management shortcuts */
#define ISwndContainer_HasCapture(This) \
    (ISwndContainer_OnGetSwndCapture(This) != 0)

#define ISwndContainer_ReleaseCapture(This) \
    ISwndContainer_OnReleaseSwndCapture(This)

/* IME shortcuts */
#define ISwndContainer_EnableIME_On(This) \
    ISwndContainer_EnableIME(This, TRUE)

#define ISwndContainer_EnableIME_Off(This) \
    ISwndContainer_EnableIME(This, FALSE)

/* Tooltip shortcuts */
#define ISwndContainer_HideToolTip(This) \
    ISwndContainer_SetToolTip(This, NULL, 0, NULL)

/* Task posting shortcuts */
#define ISwndContainer_PostAsyncTask(This, runable) \
    ISwndContainer_PostTask(This, runable, TRUE)

#define ISwndContainer_PostSyncTask(This, runable) \
    ISwndContainer_PostTask(This, runable, FALSE)

#ifdef __cplusplus
}
#endif

#endif /* __SWNDCONTAINER_CAPI_H__ */

#ifndef __SWNDCONTAINER_CAPI_H__
#define __SWNDCONTAINER_CAPI_H__

#include "../SWndContainer-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SWndContainer Interface
 * These macros provide C-style function call syntax for C++ interface methods
 *
 * Interfaces covered:
 *   ISwndContainer (inherits ITimelineHandlersMgr)
 */

/* ISwndContainer C API Macros (ITimelineHandlersMgr base interface) */
#define ISwndContainer_RegisterTimelineHandler(This, pHandler) \
    ((This)->lpVtbl->RegisterTimelineHandler(This, pHandler))

#define ISwndContainer_UnregisterTimelineHandler(This, pHandler) \
    ((This)->lpVtbl->UnregisterTimelineHandler(This, pHandler))

#define ISwndContainer_RegisterValueAnimator(This, pAnimator) \
    ((This)->lpVtbl->RegisterValueAnimator(This, pAnimator))

#define ISwndContainer_UnregisterValueAnimator(This, pAnimator) \
    ((This)->lpVtbl->UnregisterValueAnimator(This, pAnimator))

/* ISwndContainer specific interface macros */
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

#define ISwndContainer_OnDropdownState(This, pDropdown, bCreate) \
    ((This)->lpVtbl->OnDropdownState(This, pDropdown, bCreate))

#define ISwndContainer_PostTask(This, runable, bAsync) \
    ((This)->lpVtbl->PostTask(This, runable, bAsync))

#define ISwndContainer_RemoveTasksForObject(This, pObj) \
    ((This)->lpVtbl->RemoveTasksForObject(This, pObj))

#define ISwndContainer_GetSwndFinder(This) \
    ((This)->lpVtbl->GetSwndFinder(This))

#define ISwndContainer_IsTimelineEnabled(This) \
    ((This)->lpVtbl->IsTimelineEnabled(This))

#define ISwndContainer_IsDesignerMode(This) \
    ((This)->lpVtbl->IsDesignerMode(This))

#ifdef __cplusplus
}
#endif

#endif /* __SWNDCONTAINER_CAPI_H__ */

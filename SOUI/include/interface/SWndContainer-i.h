/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SwndContainer-i.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe    DUI窗口容器接口
 */

#ifndef __SWNDCONTAINER_I__H__
#define __SWNDCONTAINER_I__H__

#include <core/SDefine.h>
#include <interface/SEvtArgs-i.h>
#include <interface/SMsgLoop-i.h>
#include <interface/SScriptModule-i.h>
#include <interface/scaret-i.h>
#include <interface/STimelineHandler-i.h>
#include <interface/SAccelerator-i.h>
#include <interface/SWndFinder-i.h>

SNSBEGIN

enum
{
    ZORDER_MIN = 0,        /**< Minimum Z-Order */
    ZORDER_MAX = (UINT)-1, /**< Maximum Z-Order */
};

typedef enum GrtFlag
{
    GRT_NODRAW = 0, /**< No drawing */
    GRT_PAINTBKGND, /**< Paint background */
    GRT_OFFSCREEN,  /**< Offscreen drawing */
} GrtFlag;

/**
 * @struct     ISwndContainer
 * @brief      SOUI Window Container Interface
 *
 * This interface defines the methods for managing a window container in SOUI.
 */

typedef struct IRegionS IRegionS;

#undef INTERFACE
#define INTERFACE ISwndContainer
DECLARE_INTERFACE_(ISwndContainer, ITimelineHandlersMgr)
{
    /**
     * @brief Registers an animation frame handler.
     * @param pHandler Pointer to the ITimelineHandler interface.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, RegisterTimelineHandler)(THIS_ ITimelineHandler * pHandler) PURE;

    /**
     * @brief Unregisters an animation frame handler.
     * @param pHandler Pointer to the ITimelineHandler interface.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, UnregisterTimelineHandler)(THIS_ ITimelineHandler * pHandler) PURE;

    /**
     * @brief Registers an IDropTarget with a Swnd.
     * @param swnd Handle to the target window.
     * @param pDropTarget Pointer to the IDropTarget interface.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, RegisterDragDrop)(THIS_ SWND swnd, IDropTarget * pDropTarget) PURE;

    /**
     * @brief Unregisters an IDropTarget from a Swnd.
     * @param swnd Handle to the target window.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, UnregisterDragDrop)(THIS_ SWND swnd) PURE;

    /**
     * @brief Fires an event.
     * @param evt Pointer to the IEvtArgs event.
     * @return TRUE if the event was handled.
     */
    STDMETHOD_(BOOL, OnFireEvent)(THIS_ IEvtArgs * evt) PURE;

    /**
     * @brief Retrieves the current message loop.
     * @return Pointer to the IMessageLoop interface.
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) PURE;

    /**
     * @brief Retrieves the script module.
     * @return Pointer to the IScriptModule interface.
     */
    STDMETHOD_(IScriptModule *, GetScriptModule)(THIS) PURE;

    /**
     * @brief Retrieves the handle to the host window.
     * @return Handle to the host window (HWND).
     */
    STDMETHOD_(HWND, GetHostHwnd)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if the host window is translucent.
     * @return TRUE if the host window is translucent.
     */
    STDMETHOD_(BOOL, IsTranslucent)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if mouseWheel messages are sent to the hover window.
     * @return TRUE if mouseWheel messages are sent to the hover window, FALSE if sent to the focus window.
     */
    STDMETHOD_(BOOL, IsSendWheel2Hover)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the current DPI scale factor.
     * @return The scale factor.
     */
    STDMETHOD_(int, GetScale)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the translation context.
     * @return The translation context.
     */
    STDMETHOD_(LPCWSTR, GetTranslatorContext)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the container's display rectangle.
     * @param ret Pointer to the RECT structure to receive the container's display rectangle.
     * @return void
     */
    STDMETHOD_(void, GetContainerRect)(CTHIS_ RECT * ret) SCONST PURE;

    /**
     * @brief Updates the specified region of the memory bitmap.
     * @param rgn Pointer to the IRegionS region to update.
     * @return void
     */
    STDMETHOD_(void, UpdateRegion)(THIS_ IRegionS * rgn) PURE;

    /**
     * @brief Requests a redraw of the specified area.
     * @param rc Pointer to the LPCRECT structure defining the area to redraw.
     * @param bClip TRUE if the area should be clipped.
     * @return void
     */
    STDMETHOD_(void, OnRedraw)(THIS_ LPCRECT rc, BOOL bClip) PURE;

    /**
     * @brief Retrieves the Swnd object that has captured the mouse.
     * @return Handle to the Swnd object that has captured the mouse.
     */
    STDMETHOD_(SWND, OnGetSwndCapture)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the Swnd object to capture the mouse.
     * @param swnd Handle to the Swnd object to capture the mouse.
     * @return Handle to the previous Swnd object that had capture.
     */
    STDMETHOD_(SWND, OnSetSwndCapture)(THIS_ SWND swnd) PURE;

    /**
     * @brief Releases the mouse capture from the Swnd object.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, OnReleaseSwndCapture)(THIS) PURE;

    /**
     * @brief Retrieves the Swnd object that is hovered over.
     * @return Handle to the Swnd object that is hovered over.
     */
    STDMETHOD_(SWND, GetHover)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the Swnd object that has focus.
     * @return Handle to the Swnd object that has focus.
     */
    STDMETHOD_(SWND, GetFocus)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the focus to the specified Swnd object.
     * @param swnd Handle to the Swnd object to set focus to.
     * @return void
     */
    STDMETHOD_(void, OnSetSwndFocus)(THIS_ SWND swnd) PURE;

    /**
     * @brief Requests an immediate update of the window.
     * @param bForce TRUE to force a refresh, FALSE to refresh only if there are invalid regions.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, UpdateWindow)(THIS_ BOOL bForce DEF_VAL(TRUE)) PURE;

    /**
     * @brief Requests an update of the tooltip.
     * @return void
     */
    STDMETHOD_(void, UpdateTooltip)(THIS) PURE;

    /**
     * @brief Displays a tooltip at the specified position.
     * @param rc Pointer to the LPCRECT structure defining the position of the tooltip relative to the container.
     * @param tipAlign Alignment flags for the tooltip.
     * @param pszTip Content of the tooltip, set to NULL to exit a fixed tooltip.
     * @return void
     */
    STDMETHOD_(void, SetToolTip)(THIS_ LPCRECT rc, UINT tipAlign, LPCTSTR pszTip) PURE;

    /**
     * @brief Converts the rectangle coordinates of the current frame to the final host coordinates.
     * @param rc Pointer to the RECT structure containing the rectangle to convert.
     * @return void
     */
    STDMETHOD_(void, FrameToHost)(CTHIS_ RECT * rc) SCONST PURE;

    /**
     * @brief Retrieves the accelerator manager.
     * @return Pointer to the IAcceleratorMgr interface.
     */
    STDMETHOD_(IAcceleratorMgr *, GetAcceleratorMgr)(THIS) PURE;

    /**
     * @brief Registers a Swnd as a TrackMouseEvent window.
     * @param swnd Handle to the target window.
     * @return TRUE if successful.
     * @remark After registration, the Swnd can capture hover and leave messages even if it is obscured by other windows.
     */
    STDMETHOD_(BOOL, RegisterTrackMouseEvent)(THIS_ SWND swnd) PURE;

    /**
     * @brief Unregisters a Swnd as a TrackMouseEvent window.
     * @param swnd Handle to the target window.
     * @return TRUE if successful.
     * @remark After registration, the Swnd can capture hover and leave messages even if it is obscured by other windows.
     */
    STDMETHOD_(BOOL, UnregisterTrackMouseEvent)(THIS_ SWND swnd) PURE;

    /**
     * @brief Marks the window tree's z-order as dirty.
     * @return void
     */
    STDMETHOD_(void, MarkWndTreeZorderDirty)(THIS) PURE;

    /**
     * @brief Rebuilds the window tree's z-order.
     * @return void
     */
    STDMETHOD_(void, BuildWndTreeZorder)(THIS) PURE;

    /**
     * @brief Enables or disables the input method editor (IME).
     * @param bEnable TRUE to enable IME, FALSE to disable.
     * @return void
     */
    STDMETHOD_(void, EnableIME)(THIS_ BOOL bEnable) PURE;

    /**
     * @brief Updates the cursor.
     * @return void
     */
    STDMETHOD_(void, OnUpdateCursor)(THIS) PURE;

    /**
     * @brief Registers a VideoCanvas window.
     * @param swnd Handle to the VideoCanvas window.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, RegisterVideoCanvas)(THIS_ SWND swnd) PURE;

    /**
     * @brief Unregisters a VideoCanvas window.
     * @param swnd Handle to the VideoCanvas window.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, UnregisterVideoCanvas)(THIS_ SWND swnd) PURE;

    /**
     * @brief Enables or disables the host's private UI definitions.
     * @param bEnable TRUE to enable, FALSE to disable.
     * @return void
     */
    STDMETHOD_(void, EnableHostPrivateUiDef)(THIS_ BOOL bEnable) PURE;

    /**
     * @brief Posts a task to the message queue.
     * @param runable Pointer to the IRunnable task object.
     * @param bAsync TRUE to execute asynchronously, FALSE to execute synchronously.
     * @return TRUE if successful.
     */
    STDMETHOD_(BOOL, PostTask)(THIS_ IRunnable * runable, BOOL bAsync DEF_VAL(TRUE)) PURE;

    /**
     * @brief Removes all tasks associated with an object.
     * @param pObj Pointer to the object.
     * @return The number of tasks removed.
     */
    STDMETHOD_(int, RemoveTasksForObject)(THIS_ void *pObj) PURE;

    /**
     * @brief Gets the window finder object.
     * @return Pointer to the window finder object.
     */
    STDMETHOD_(ISwndFinder*, GetSwndFinder)(THIS) PURE;
};

SNSEND

#endif // __SWNDCONTAINER_I__H__
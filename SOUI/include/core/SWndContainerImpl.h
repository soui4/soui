/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SwndContainerImpl.h
 * @brief      SOUI窗口容器的实现
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: This file defines the SwndContainerImpl class, which implements the ISwndContainer
 *              and ITimelineHandler interfaces for managing a container of SOUI windows.
 */

#ifndef __SWNDCONTAINERIMPL__H__
#define __SWNDCONTAINERIMPL__H__

#include <core/SDropTargetDispatcher.h>
#include <core/SFocusManager.h>
#include <core/STimerlineHandlerMgr.h>
#include <helper/SwndFinder.h>
SNSBEGIN

/**
 * @class SwndContainerImpl
 * @brief Implementation of the SOUI window container.
 *
 * This class implements the ISwndContainer and ITimelineHandler interfaces to manage a container
 * of SOUI windows. It handles drag-and-drop operations, focus management, mouse and keyboard events,
 * and timeline handling.
 */
class SOUI_EXP SwndContainerImpl
    : public ISwndContainer
    , public ITimelineHandler {
  public:
    /**
     * @brief Constructor.
     */
    SwndContainerImpl();

    /**
     * @brief Destructor.
     */
    ~SwndContainerImpl();

    /**
     * @brief Retrieves the drop target.
     * @return Pointer to the drop target.
     */
    IDropTarget *GetDropTarget()
    {
        return &m_dropTarget;
    }

    /**
     * @brief Retrieves the focus manager.
     * @return Pointer to the focus manager.
     */
    SFocusManager *GetFocusManager()
    {
        return &m_focusMgr;
    }

    /**
     * @brief Sets the root window of the container.
     * @param pRoot Pointer to the root window.
     */
    void SetRoot(SWindow *pRoot);

    /**
     * @brief Processes frame events.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     * @return Result of the message processing.
     */
    virtual LRESULT DoFrameEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

  protected:
    // ISwndContainer
    /**
     * @brief Registers a drop target for a window.
     * @param swnd Window handle.
     * @param pDropTarget Pointer to the drop target.
     * @return TRUE if registration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RegisterDragDrop)(THIS_ SWND swnd, IDropTarget *pDropTarget) OVERRIDE;

    /**
     * @brief Unregisters a drop target for a window.
     * @param swnd Window handle.
     * @return TRUE if unregistration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UnregisterDragDrop)(THIS_ SWND swnd) OVERRIDE;

    /**
     * @brief Releases the window capture.
     * @return TRUE if the capture is released, FALSE otherwise.
     */
    STDMETHOD_(BOOL, OnReleaseSwndCapture)(THIS) OVERRIDE;

    /**
     * @brief Sets the window capture.
     * @param swnd Window handle.
     * @return Previous window handle that had capture.
     */
    STDMETHOD_(SWND, OnSetSwndCapture)(THIS_ SWND swnd) OVERRIDE;

    /**
     * @brief Sets the window focus.
     * @param swnd Window handle.
     */
    STDMETHOD_(void, OnSetSwndFocus)(THIS_ SWND swnd) OVERRIDE;

    /**
     * @brief Transforms a rectangle from frame coordinates to host coordinates.
     * @param rc Rectangle to transform.
     */
    STDMETHOD_(void, FrameToHost)(THIS_ RECT *rc) SCONST OVERRIDE
    {
    }

    /**
     * @brief Retrieves the window handle that has capture.
     * @return Window handle with capture.
     */
    STDMETHOD_(SWND, OnGetSwndCapture)(THIS) SCONST OVERRIDE;

    /**
     * @brief Retrieves the window handle that has focus.
     * @return Window handle with focus.
     */
    STDMETHOD_(SWND, GetFocus)(THIS) SCONST OVERRIDE;

    /**
     * @brief Retrieves the window handle that is hovered.
     * @return Window handle that is hovered.
     */
    STDMETHOD_(SWND, GetHover)(THIS) SCONST OVERRIDE;

    /**
     * @brief Retrieves the accelerator manager.
     * @return Pointer to the accelerator manager.
     */
    STDMETHOD_(IAcceleratorMgr *, GetAcceleratorMgr)(THIS)
    {
        return &m_focusMgr;
    }

    STDMETHOD_(ISwndFinder *, GetSwndFinder)(THIS) OVERRIDE
    {
        return m_swndFinder;
    }

    /**
     * @brief Registers a timeline handler.
     * @param pHandler Pointer to the timeline handler.
     * @return TRUE if registration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RegisterTimelineHandler)(THIS_ ITimelineHandler *pHandler) OVERRIDE;

    /**
     * @brief Unregisters a timeline handler.
     * @param pHandler Pointer to the timeline handler.
     * @return TRUE if unregistration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UnregisterTimelineHandler)(THIS_ ITimelineHandler *pHandler) OVERRIDE;

    /**
     * @brief Registers a value animator.
     * @param pAnimator Pointer to the value animator.
     * @return TRUE if registration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RegisterValueAnimator)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

    /**
     * @brief Unregisters a value animator.
     * @param pAnimator Pointer to the value animator.
     * @return TRUE if unregistration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UnregisterValueAnimator)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

    /**
     * @brief Registers a window for tracking mouse events.
     * @param swnd Window handle.
     * @return TRUE if registration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RegisterTrackMouseEvent)(THIS_ SWND swnd) OVERRIDE;

    /**
     * @brief Unregisters a window for tracking mouse events.
     * @param swnd Window handle.
     * @return TRUE if unregistration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UnregisterTrackMouseEvent)(THIS_ SWND swnd) OVERRIDE;

    /**
     * @brief Marks the window tree's z-order as dirty.
     */
    STDMETHOD_(void, MarkWndTreeZorderDirty)(THIS) OVERRIDE;

    /**
     * @brief Rebuilds the window tree's z-order.
     */
    STDMETHOD_(void, BuildWndTreeZorder)(THIS) OVERRIDE;

    /**
     * @brief Registers a window as a video canvas.
     * @param swnd Window handle.
     * @return TRUE if registration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RegisterVideoCanvas)(THIS_ SWND swnd) OVERRIDE;

    /**
     * @brief Unregisters a window as a video canvas.
     * @param swnd Window handle.
     * @return TRUE if unregistration is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, UnregisterVideoCanvas)(THIS_ SWND swnd) OVERRIDE;

    /**
     * @brief Checks if the timeline is enabled.
     * @return TRUE if timeline is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsTimelineEnabled)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Checks if the designer mode is enabled.
     * @return TRUE if designer mode is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsDesignerMode)(CTHIS) SCONST OVERRIDE;
  public: // ITimelineHandler
    /**
     * @brief Called when the next frame is ready.
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

  protected:
    /**
     * @brief Handles mouse move events within the frame.
     * @param uFlag Flags indicating the state of the mouse buttons.
     * @param pt Current mouse position.
     */
    void OnFrameMouseMove(UINT uFlag, CPoint pt);

    /**
     * @brief Handles mouse leave events within the frame.
     */
    void OnFrameMouseLeave();

    /**
     * @brief Sets the cursor based on the mouse position.
     * @param pt Current mouse position.
     * @return TRUE if the cursor is set, FALSE otherwise.
     */
    BOOL OnFrameSetCursor(const CPoint &pt);

    /**
     * @brief Handles mouse events within the frame.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     */
    void OnFrameMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles mouse wheel events within the frame.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     */
    void OnFrameMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles key events within the frame.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     * @return Result of the message processing.
     */
    LRESULT OnFrameKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles key down events within the frame.
     * @param nChar Virtual-key code of the key.
     * @param nRepCnt Repeat count of the key.
     * @param nFlags Flags indicating the state of the key.
     */
    void OnFrameKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handles window activation events.
     * @param nState Activation state.
     */
    void OnActivate(UINT nState);

    /**
     * @brief Handles application activation events.
     * @param bActive TRUE if the application is being activated, FALSE if deactivated.
     * @param dwThreadID Thread identifier of the application.
     */
    void OnActivateApp(BOOL bActive, DWORD dwThreadID);

  protected:
    SWindow *m_pRoot;                          /**< Root window of the container */
    SWND m_hCapture;                           /**< Window handle with capture */
    SWND m_hHover;                             /**< Window handle that is hovered */
    BOOL m_bNcHover;                           /**< Non-client area hover flag */
    SFocusManager m_focusMgr;                  /**< Focus manager */
    SDropTargetDispatcher m_dropTarget;        /**< Drop target dispatcher */
    BOOL m_bZorderDirty;                       /**< Z-order dirty flag */
    SList<SWND> m_lstTrackMouseEvtWnd;         /**< List of windows tracking mouse events */
    SList<SWND> m_lstVideoCanvas;              /**< List of video canvas windows */
    SAutoRefPtr<ICaret> m_caret;               /**< Caret */
    STimerlineHandlerMgr m_timelineHandlerMgr; /**< Timeline handler manager */
    SAutoRefPtr<ISwndFinder> m_swndFinder;     /**< Swnd finder */
};

SNSEND
#endif // __SWNDCONTAINERIMPL__H__
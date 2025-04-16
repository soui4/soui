/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SDropTargetDispatcher.h
 * @brief      Drag-and-drop target dispatcher module
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * @details    Manages the dispatching of drag-and-drop targets between DUI windows.
 */

#ifndef __SDROPTARGETDISPATCHER__H__
#define __SDROPTARGETDISPATCHER__H__

#include <windows.h>
#include <souicoll.h>
#include <core/SWnd.h>

SNSBEGIN

/**
 * @class      SDropTargetDispatcher
 * @brief      Manages the dispatching of drag-and-drop targets between DUI windows.
 *
 * @details    This class handles the registration and dispatching of drag-and-drop targets
 *             for DUI windows. It implements the `IDropTarget` interface to manage drag-and-drop
 *             operations and routes them to the appropriate window.
 */
class SDropTargetDispatcher : public IDropTarget {
  public:
    /**
     * @brief    Constructor
     *
     * @details  Initializes the drop target dispatcher.
     */
    SDropTargetDispatcher();

    /**
     * @brief    Destructor
     *
     * @details  Cleans up the drop target dispatcher.
     */
    ~SDropTargetDispatcher(void);

    /**
     * @brief    Sets the owner window
     * @param    pOwner  Pointer to the owner window
     *
     * @details  Sets the owner window for the drop target dispatcher.
     */
    void SetOwner(SWindow *pOwner);

    /**
     * @brief    Registers a drag-and-drop target for a window
     * @param    swnd        Handle to the window
     * @param    pDropTarget Pointer to the drop target
     * @return   TRUE if successful, otherwise FALSE
     *
     * @details  Registers a drag-and-drop target for the specified window.
     */
    BOOL RegisterDragDrop(SWND swnd, IDropTarget *pDropTarget);

    /**
     * @brief    Unregisters a drag-and-drop target for a window
     * @param    swnd  Handle to the window
     * @return   TRUE if successful, otherwise FALSE
     *
     * @details  Unregisters the drag-and-drop target for the specified window.
     */
    BOOL UnregisterDragDrop(SWND swnd);

    //////////////////////////////////////////////////////////////////////////
    // IUnknown

    /**
     * @brief    Queries for a specific interface
     * @param    riid      Interface identifier
     * @param    ppvObject Pointer to the interface pointer
     * @return   HRESULT indicating success or failure
     *
     * @details  Queries for the specified interface and returns a pointer to it.
     */
    STDMETHOD_(HRESULT, QueryInterface)
    (
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void **ppvObject);

    /**
     * @brief    Increments the reference count
     * @return   New reference count
     *
     * @details  Increments the reference count of the object. Always returns 1.
     */
    STDMETHOD_(ULONG, AddRef)(void)
    {
        return 1;
    }

    /**
     * @brief    Decrements the reference count
     * @return   New reference count
     *
     * @details  Decrements the reference count of the object. Always returns 1.
     */
    STDMETHOD_(ULONG, Release)(void)
    {
        return 1;
    }

    //////////////////////////////////////////////////////////////////////////
    // IDropTarget

    /**
     * @brief    Handles the DragEnter event
     * @param    pDataObj    Pointer to the data object
     * @param    grfKeyState State of the keyboard and mouse
     * @param    pt          Mouse position
     * @param    pdwEffect   Pointer to the drag effect
     * @return   HRESULT indicating success or failure
     *
     * @details  Handles the DragEnter event and routes it to the appropriate window.
     */
    STDMETHOD_(HRESULT, DragEnter)
    (
        /* [unique][in] */ IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect);

    /**
     * @brief    Handles the DragOver event
     * @param    grfKeyState State of the keyboard and mouse
     * @param    pt          Mouse position
     * @param    pdwEffect   Pointer to the drag effect
     * @return   HRESULT indicating success or failure
     *
     * @details  Handles the DragOver event and routes it to the appropriate window.
     */
    STDMETHOD_(HRESULT, DragOver)
    (
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect);

    /**
     * @brief    Handles the DragLeave event
     * @return   HRESULT indicating success or failure
     *
     * @details  Handles the DragLeave event and routes it to the appropriate window.
     */
    STDMETHOD_(HRESULT, DragLeave)(void);

    /**
     * @brief    Handles the Drop event
     * @param    pDataObj    Pointer to the data object
     * @param    grfKeyState State of the keyboard and mouse
     * @param    pt          Mouse position
     * @param    pdwEffect   Pointer to the drag effect
     * @return   HRESULT indicating success or failure
     *
     * @details  Handles the Drop event and routes it to the appropriate window.
     */
    STDMETHOD_(HRESULT, Drop)
    (
        /* [unique][in] */ IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ DWORD *pdwEffect);

  protected:
    /**
     * @brief    Converts POINTL to frame client coordinates
     * @param    pt  POINTL structure
     * @return   Converted POINT structure in frame client coordinates
     *
     * @details  Converts the POINTL structure to frame client coordinates.
     */
    POINT PointL2FrameClient(const POINTL &pt);

    typedef SMap<SWND, IDropTarget *> DTMAP; /**< Map type for storing drop targets. */
    DTMAP m_mapDropTarget;                   /**< Map of drop targets associated with windows. */
    IDataObject *m_pDataObj;                 /**< Pointer to the data object. */
    SWND m_hHover;                           /**< Handle to the window currently being hovered over. */
    SWindow *m_pOwner;                       /**< Pointer to the owner window. */
};

SNSEND

#endif // __SDROPTARGETDISPATCHER__H__
/**
 * @file       SRealWndHandler-i.h
 * @brief      Interface for handling real window creation and destruction.
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/14
 *
 * This file provides the interface for managing the creation, destruction, and
 * initialization of real windows.
 */

#ifndef __SREALWNDHANDLER_I__H__
#define __SREALWNDHANDLER_I__H__
#include <interface/obj-ref-i.h>

SNSBEGIN

interface IWindow;

/**
 * @interface IRealWndHandler
 * @brief Interface for handling real window operations.
 *
 * This interface defines methods for creating, destroying, initializing, and
 * managing the position of real windows.
 */
#undef INTERFACE
#define INTERFACE IRealWndHandler
DECLARE_INTERFACE_(IRealWndHandler, IObjRef)
{
    /**
     * @brief Increment the reference count.
     * @return long - Current reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count.
     * @return long - Current reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Final release of the object.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create a real window.
     * @param pRealWnd - Pointer to the window interface.
     * @return HWND - Handle to the created window.
     *
     * This method is called when a real window is created.
     */
    STDMETHOD_(HWND, OnRealWndCreate)(THIS_ IWindow * pRealWnd) PURE;

    /**
     * @brief Destroy a real window.
     * @param pRealWnd - Pointer to the window interface.
     *
     * This method is called when a real window is destroyed.
     */
    STDMETHOD_(void, OnRealWndDestroy)(THIS_ IWindow * pRealWnd) PURE;

    /**
     * @brief Initialize a real window.
     * @param pRealWnd - Pointer to the window interface.
     * @return BOOL - FALSE if the system should handle the initialization, TRUE if the user should handle it.
     *
     * This method is called to initialize a real window.
     */
    STDMETHOD_(BOOL, OnRealWndInit)(THIS_ IWindow * pRealWnd) PURE;

    /**
     * @brief Adjust the position of a real window.
     * @param pRealWnd - Pointer to the window interface.
     * @param rcWnd - New position and size of the window.
     * @return BOOL - FALSE if SOUI should handle the position adjustment, TRUE if the user should manage it.
     *
     * This method is called to adjust the position and size of a real window.
     */
    STDMETHOD_(BOOL, OnRealWndPosition)(THIS_ IWindow * pRealWnd, const RECT *rcWnd) PURE;
};

SNSEND
#endif // __SREALWNDHANDLER_I__H__
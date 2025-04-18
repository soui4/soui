/**
 * @file       stooltip-i.h
 * @brief      Interface for Tooltip Objects
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/01
 *
 * @details    This file defines the interfaces for tooltip objects and their factory.
 */

#ifndef __STOOLTIP_I__H__
#define __STOOLTIP_I__H__

#include <interface/obj-ref-i.h>
#include <interface/SMsgLoop-i.h>

SNSBEGIN

/**
 * @struct TIPID
 * @brief  Identifier for a tooltip
 *
 * @details This structure uniquely identifies a tooltip using two DWORD values and a boolean flag
 * indicating whether it is a non-client area tooltip.
 */
typedef struct TIPID
{
    DWORD dwHi;  //!< ID1, used to save a SWND.
    DWORD dwLow; //!< ID2, used to save additional data.
    BOOL bNcTip; //!< Flag indicating if the tooltip is for the non-client area.
} TIPID;

/**
 * @enum TipAlign
 * @brief Alignment options for tooltips
 *
 * @details These constants define the possible alignment options for tooltips, including horizontal and vertical alignments.
 */
typedef enum TipAlign
{
    TA_AUTO = 0,        //!< Automatic alignment.
    TA_X_LEFT = 1,      //!< Align left horizontally.
    TA_X_CENTER = 2,    //!< Align center horizontally.
    TA_X_RIGHT = 3,     //!< Align right horizontally.
    TA_X_MASK = 0x0f,   //!< Mask for horizontal alignment.
    TA_Y_TOP = 0x10,    //!< Align top vertically.
    TA_Y_CENTER = 0x20, //!< Align center vertically.
    TA_Y_BOTTOM = 0x30, //!< Align bottom vertically.
    TA_Y_MASK = 0xF0,   //!< Mask for vertical alignment.
} TipAlign;

/**
 * @struct IToolTip
 * @brief Interface for Tooltip Objects
 *
 * @details This interface defines the methods for tooltip objects, which handle the display and management of tooltips.
 */
#undef INTERFACE
#define INTERFACE IToolTip
DECLARE_INTERFACE_(IToolTip, IMsgFilter)
{
    /**
     * @brief Pre-translates messages for the tooltip.
     *
     * @param pMsg Pointer to the message to be translated.
     * @return BOOL TRUE if the message is handled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG * pMsg) PURE;

    /**
     * @brief Updates the current tooltip.
     *
     * @param id Pointer to the tooltip ID.
     * @param rc Rectangle defining the tooltip's感应 area.
     * @param pszTip Tooltip string.
     * @param nScale Current scaling factor.
     */
    STDMETHOD_(void, UpdateTip)(THIS_ const TIPID *id, RECT rc, LPCTSTR pszTip, int nScale) PURE;

    /**
     * @brief Displays the tooltip at a specified position.
     *
     * @param rc Rectangle defining the tooltip's display position relative to the container.
     * @param tipAlign Tooltip alignment options.
     * @param pszTip Tooltip content. If NULL, the fixed tooltip is exited.
     * @param nScale Current scaling factor.
     */
    STDMETHOD_(void, SetToolTip)(THIS_ LPCRECT rc, UINT tipAlign, LPCTSTR pszTip, int nScale) PURE;

    /**
     * @brief Clears the current tooltip.
     */
    STDMETHOD_(void, ClearTip)(THIS) PURE;

    /**
     * @brief Relays mouse messages to the tooltip.
     *
     * @param pMsg Pointer to the mouse message.
     */
    STDMETHOD_(void, RelayEvent)(THIS_ const MSG *pMsg) PURE;
};

/**
 * @struct IToolTipFactory
 * @brief Interface for Tooltip Factory
 *
 * @details This interface defines the methods for creating and destroying tooltip objects.
 */
#undef INTERFACE
#define INTERFACE IToolTipFactory
DECLARE_INTERFACE_(IToolTipFactory, IObjRef)
{
    /**
     * @brief Increments the reference count of the object.
     *
     * @return long The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrements the reference count of the object.
     *
     * @return long The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Final release of the object, performing cleanup if necessary.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Creates a tooltip object.
     *
     * @param hHost Handle to the SOUI host window.
     * @return IToolTip* Pointer to the created tooltip object.
     *
     * @details Do not use hHost as the owner of the tooltip to avoid potential runtime issues.
     */
    STDMETHOD_(IToolTip *, CreateToolTip)(THIS_ HWND hHost) PURE;

    /**
     * @brief Destroys a tooltip object.
     *
     * @param pToolTip Pointer to the tooltip object to be destroyed.
     */
    STDMETHOD_(void, DestroyToolTip)(THIS_ IToolTip * pToolTip) PURE;
};

SNSEND
#endif // __STOOLTIP_I__H__
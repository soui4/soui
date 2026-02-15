/**
 * @file       STileView.h
 * @brief      Definition of the STileView class.
 * @version    v1.0
 * @author     soui
 * @date       2014-07-06
 *
 * @copyright  Copyright (C) 2014-2050 SOUI团队
 *             All rights reserved.
 */

#ifndef __STILEVIEW__H__
#define __STILEVIEW__H__

#include <core/SWnd.h>
#include <core/SPanel.h>
#include <core/SItemPanel.h>
#include <interface/SAdapter-i.h>
#include <helper/STileViewItemLocator.h>
#include <proxy/SPanelProxy.h>
#include "SViewBase.h"

SNSBEGIN

namespace STileView_style
{
    static const LPCWSTR kStyle_template = L"template";
    static const LPCWSTR kStyle_itemHeight = L"itemHeight";
    static const LPCWSTR kStyle_itemWidth = L"itemWidth";
}

/**
 * @class STileView
 * @brief A tile view control for displaying items in a grid layout.
 *
 * This class represents a tile view control that can display items in a grid layout with various customization options.
 */
class SOUI_EXP STileView
    : public TPanelProxy<ITileView>
    , public SViewBase {
    DEF_SOBJECT(SPanel, L"tileview")

    friend class STileViewDataSetObserver;

  public:
    /**
     * @brief Constructor for STileView.
     */
    STileView();

    /**
     * @brief Destructor for STileView.
     */
    ~STileView();

  public:
    /**
     * @brief Sets the adapter for the tile view.
     * @param adapter Pointer to the adapter.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter *adapter) OVERRIDE;

    /**
     * @brief Gets the adapter for the tile view.
     * @return Pointer to the adapter.
     */
    STDMETHOD_(ILvAdapter *, GetAdapter)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the item locator for the tile view.
     * @param pItemLocator Pointer to the item locator.
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ ITileViewItemLocator *pItemLocator) OVERRIDE;

    /**
     * @brief Gets the item locator for the tile view.
     * @return Pointer to the item locator.
     */
    STDMETHOD_(ITileViewItemLocator *, GetItemLocator)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the selected item.
     * @param iItem Index of the item to select.
     * @param bNotify Flag indicating if a notification should be sent.
     */
    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify = FALSE) OVERRIDE;

    /**
     * @brief Gets the selected item.
     * @return Index of the selected item.
     */
    STDMETHOD_(int, GetSel)(THIS) SCONST OVERRIDE;

    /**
     * @brief Ensures the specified item is visible.
     * @param iItem Index of the item to make visible.
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Performs a hit test on the tile view.
     * @param pt Pointer to the point to test.
     * @return Pointer to the item panel at the specified point.
     */
    STDMETHOD_(IItemPanel *, HitTest)(THIS_ const POINT *pt) SCONST OVERRIDE;

    /**
     * @brief Sets the multiple selection mode.
     * @param bMultiSel TRUE to enable multiple selection, FALSE otherwise.
     */
    STDMETHOD_(void, SetMultiSel)(THIS_ BOOL bMultiSel) OVERRIDE;

    /**
     * @brief Gets the multiple selection mode.
     * @return TRUE if multiple selection is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetMultiSel)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Adds an item to the selection.
     * @param iItem Index of the item.
     */
    STDMETHOD_(void, AddSelItem)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Removes an item from the selection.
     * @param iItem Index of the item.
     */
    STDMETHOD_(void, RemoveSelItem)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Clears all selected items.
     */
    STDMETHOD_(void, ClearSelItems)(THIS) OVERRIDE;

    /**
     * @brief Checks if an item is selected.
     * @param iItem Index of the item.
     * @return TRUE if the item is selected, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsItemSelected)(THIS_ int iItem) SCONST OVERRIDE;

    /**
     * @brief Gets the count of selected items.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItemCount)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets all selected items.
     * @param items Output parameter to store the selected item indices.
     * @param nMaxCount Maximum number of items to retrieve.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItems)(THIS_ int *items, int nMaxCount) SCONST OVERRIDE;

  public:
    /**
     * @brief Performs a hit test on the tile view.
     * @param pt The point to test.
     * @return Pointer to the item panel at the specified point.
     */
    SItemPanel *HitTest(CPoint &pt) const;

  protected:
    /**
     * @brief Handles scroll events.
     * @param bVertical Flag indicating if the scroll is vertical.
     * @param uCode Scroll code.
     * @param nPos Scroll position.
     * @return TRUE if the event was handled, otherwise FALSE.
     */
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);

    /**
     * @brief Gets the scroll line size.
     * @param bVertical Flag indicating if the scroll is vertical.
     * @return Scroll line size.
     */
    virtual int GetScrollLineSize(BOOL bVertical);

    /**
     * @brief Creates child windows from an XML node.
     * @param xmlNode XML node containing the child window definitions.
     * @return TRUE if successful, otherwise FALSE.
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Updates the tooltip.
     * @param pt Mouse position.
     * @param tipInfo Tooltip information.
     * @return TRUE if the tooltip was updated, otherwise FALSE.
     */
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);

    /**
     * @brief Sets the cursor.
     * @param pt Mouse position.
     * @return TRUE if the cursor was set, otherwise FALSE.
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Calculates the drawing rectangle for an item.
     * @param iItem Index of the item.
     * @return Drawing rectangle of the item.
     */
    CRect CalcItemDrawRect(int iItem) const; // 计算item实际绘制的位置

    /**
     * @brief Updates the scroll bars.
     */
    void UpdateScrollBar();

    /**
     * @brief Gets the item panel by index.
     * @param iItem Index of the item.
     * @return Pointer to the item panel.
     */
    SItemPanel *GetItemPanel(int iItem);

    /**
     * @brief Updates the visible items.
     */
    void UpdateVisibleItems();

    /**
     * @brief Updates a specific visible item.
     * @param iItem Index of the item.
     */
    void UpdateVisibleItem(int iItem);

    /**
     * @brief Handles the paint event.
     * @param pRT Pointer to the rendering target.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handles the size event.
     * @param nType Type of size change.
     * @param size New size.
     */
    void OnSize(UINT nType, CSize size);

    /**
     * @brief Handles the destroy event.
     */
    void OnDestroy();

    /**
     * @brief Handles mouse events.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     * @return Result of the message handling.
     */
    LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles key events.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     * @return Result of the message handling.
     */
    LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the key down event.
     * @param nChar Virtual key code of the key.
     * @param nRepCnt Repeat count for the key.
     * @param nFlags Flags associated with the key event.
     */
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handles the mouse wheel event.
     * @param nFlags Flags associated with the mouse event.
     * @param zDelta Wheel delta.
     * @param pt Mouse position.
     * @return TRUE if the event was handled, otherwise FALSE.
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handles the set scale event.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     * @return Result of the message handling.
     */
    LRESULT OnSetScale(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle mouse leave event
     */
    void OnMouseLeave();

    /**
     * @brief Handle kill focus event
     * @param wndFocus New focus window
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handle set focus event
     * @param wndOld Old focus window
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Handle show window event
     * @param bShow Whether the window is shown
     * @param nStatus Show status
     */
    void OnShowWindow(BOOL bShow, UINT nStatus);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)                                                      /**< Paint event. */
        MSG_WM_SIZE(OnSize)                                                           /**< Size event. */
        MSG_WM_DESTROY(OnDestroy)                                                     /**< Destroy event. */
        MSG_WM_MOUSEWHEEL(OnMouseWheel)                                               /**< Mouse wheel event. */
        MSG_WM_MOUSELEAVE(OnMouseLeave)                                               /**< Mouse leave event. */
        MSG_WM_KEYDOWN(OnKeyDown)                                                     /**< Key down event. */
        MSG_WM_KILLFOCUS_EX(OnKillFocus)                                              /**< Kill focus event. */
        MSG_WM_SETFOCUS_EX(OnSetFocus)                                                /**< Set focus event. */
        MESSAGE_HANDLER_EX(UM_SETSCALE, OnSetScale)                                   /**< Set scale event. */
        MSG_WM_SHOWWINDOW(OnShowWindow)                                               /**< Show window event. */
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)           /**< Mouse event range. */
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent)                 /**< Key event range. */
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, WM_IME_KEYLAST, OnKeyEvent) /**< IME key event range. */
        MESSAGE_HANDLER_EX(WM_IME_CHAR, OnKeyEvent)                                   /**< IME character event. */
        MESSAGE_HANDLER_EX(WM_IME_REQUEST, OnKeyEvent)                                /**< IME request event. */
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"marginSize", m_nMarginSize, FALSE) /**< Margin size for items. */
        ATTR_CHAIN_CLASS(SViewBase)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Handle data set changed event
     */
    virtual void onDataSetChanged();

    /**
     * @brief Handle data set invalidated event
     */
    virtual void onDataSetInvalidated();

    /**
     * @brief Handle item data changed event
     * @param iItem Index of the item
     */
    virtual void onItemDataChanged(int iItem);

    /**
     * @brief Indicates if item redraw is delayed
     * @return TRUE if redraw is delayed, FALSE otherwise
     */
    virtual BOOL IsItemRedrawDelay() const;

    /**
     * @brief Indicates if timeline is enabled
     * @return TRUE if timeline is enabled, FALSE otherwise
     */
    virtual BOOL IsTimelineEnabled() const;

    /**
     * @brief Gets the rectangle of an item
     * @param pItem Pointer to the item panel
     * @param rcItem Reference to the rectangle to fill
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const;

    /**
     * @brief Sets or releases capture on an item
     * @param pItem Pointer to the item panel
     * @param bCapture TRUE to set capture, FALSE to release
     */
    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture);

    /**
     * @brief Redraws an item
     * @param pItem Pointer to the item panel
     */
    virtual void RedrawItem(SOsrPanel *pItem);
    virtual ILvAdapter *getAdapter() override
    {
        return m_adapter;
    }
    /**
     * @brief Gets the dialog code
     * @return Dialog code
     */
    virtual UINT WINAPI OnGetDlgCode() const;

    /**
     * @brief Handles colorization event
     * @param cr Color reference
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief Handles scale change event
     * @param nScale Scale factor
     */
    virtual void OnScaleChanged(int nScale);

    /**
     * @brief Handles language change event
     * @return HRESULT
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Handles font rebuild event
     */
    virtual void OnRebuildFont();

    /**
     * @brief Dispatches messages to items
     * @param uMsg Message identifier
     * @param wParam Additional message parameter
     * @param lParam Additional message parameter
     */
    virtual void DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam);

  protected:
    SAutoRefPtr<ILvAdapter> m_adapter;                 /**< Adapter for the tile view. */
    SAutoRefPtr<ILvDataSetObserver> m_observer;        /**< Data set observer. */
    SAutoRefPtr<ITileViewItemLocator> m_tvItemLocator; /**< Item locator for the tile view. */

    SLayoutSize m_nMarginSize;  /**< Margin size for items. */
    BOOL m_bDatasetInvalidated; /**< Flag indicating if the data set is invalidated. */
};

SNSEND

#endif // __STILEVIEW__H__
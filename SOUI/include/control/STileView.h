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

SNSBEGIN

/**
 * @class STileView
 * @brief A tile view control for displaying items in a grid layout.
 *
 * This class represents a tile view control that can display items in a grid layout with various customization options.
 */
class SOUI_EXP STileView
    : public TPanelProxy<ITileView>
    , protected SHostProxy
    , protected IItemContainer {
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

  public:
    /**
     * @brief Performs a hit test on the tile view.
     * @param pt The point to test.
     * @return Pointer to the item panel at the specified point.
     */
    SItemPanel *HitTest(CPoint &pt) const;

  protected:
    /**
     * @brief Handles the capture of an item.
     * @param pItem Pointer to the item panel.
     * @param bCapture Flag indicating if the item should be captured.
     */
    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture);

    /**
     * @brief Gets the rectangle of an item.
     * @param pItem Pointer to the item panel.
     * @param rcItem Rectangle to receive the item position.
     * @return TRUE if successful, otherwise FALSE.
     */
    virtual BOOL OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const;

    /**
     * @brief Checks if item redraw is delayed.
     * @return TRUE if item redraw is delayed, otherwise FALSE.
     */
    virtual BOOL IsItemRedrawDelay() const;

    virtual BOOL IsTimelineEnabled() const;

  protected:
    /**
     * @brief Handles data set changes.
     */
    void onDataSetChanged();

    /**
     * @brief Handles data set invalidation.
     */
    void onDataSetInvalidated();

    /**
     * @brief Handles data changes for a specific item.
     * @param iItem Index of the item.
     */
    void onItemDataChanged(int iItem);

  protected:
    /**
     * @brief Handles item click events.
     * @param pEvt Pointer to the event arguments.
     * @return TRUE if the event was handled, otherwise FALSE.
     */
    BOOL OnItemClick(IEvtArgs *pEvt);

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
     * @brief Gets the dialog code for the window.
     * @return Dialog code.
     */
    virtual UINT WINAPI OnGetDlgCode() const;

    /**
     * @brief Sets the cursor.
     * @param pt Mouse position.
     * @return TRUE if the cursor was set, otherwise FALSE.
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Handles colorization.
     * @param cr Color reference.
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief Handles scale changes.
     * @param nScale New scale value.
     */
    virtual void OnScaleChanged(int nScale);

    /**
     * @brief Handles language changes.
     * @return Result of the language change handling.
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Rebuilds the font.
     */
    virtual void OnRebuildFont();

  protected:
    /**
     * @brief Dispatches messages to items.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     */
    void DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Updates the scroll bars.
     */
    void UpdateScrollBar();

    /**
     * @brief Redraws a specific item.
     * @param pItem Pointer to the item panel.
     */
    void RedrawItem(SOsrPanel *pItem);

    /**
     * @brief Gets the item panel by index.
     * @param iItem Index of the item.
     * @return Pointer to the item panel.
     */
    SItemPanel *GetItemPanel(int iItem);

    /**
     * @brief Calculates the drawing rectangle for an item.
     * @param iItem Index of the item.
     * @return Drawing rectangle of the item.
     */
    CRect CalcItemDrawRect(int iItem) const; // 计算item实际绘制的位置

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
     * @brief Handles the mouse leave event.
     */
    void OnMouseLeave();

    /**
     * @brief Handles the mouse wheel event.
     * @param nFlags Flags associated with the mouse event.
     * @param zDelta Wheel delta.
     * @param pt Mouse position.
     * @return TRUE if the event was handled, otherwise FALSE.
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handles the kill focus event.
     * @param wndFocus Handle to the window receiving focus.
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handles the set focus event.
     * @param wndOld Handle to the window losing focus.
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Handles the set scale event.
     * @param uMsg Message identifier.
     * @param wParam Additional message-specific information.
     * @param lParam Additional message-specific information.
     * @return Result of the message handling.
     */
    LRESULT OnSetScale(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the show window event.
     * @param bShow Flag indicating if the window is being shown.
     * @param nStatus Status of the window.
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
        ATTR_BOOL(L"wantTab", m_bWantTab, FALSE)             /**< Flag indicating if tabbing is wanted. */
    SOUI_ATTRS_END()

  protected:
    SAutoRefPtr<ILvAdapter> m_adapter;                 /**< Adapter for the tile view. */
    SAutoRefPtr<ILvDataSetObserver> m_observer;        /**< Data set observer. */
    SAutoRefPtr<ITileViewItemLocator> m_tvItemLocator; /**< Item locator for the tile view. */

    /**
     * @struct ItemInfo
     * @brief Information about an item in the tile view.
     */
    struct ItemInfo
    {
        SItemPanel *pItem; /**< Pointer to the item panel. */
        int nType;         /**< Type of the item. */
    };

    bool m_bPendingUpdate;    /**< Flag indicating if an update is pending due to data set changes. */
    int m_iPendingUpdateItem; /**< Index of the item to update, -1 for all, -2 for nothing. */
    int m_iPendingViewItem;   /**< Index of the item to view, -1 for initialization. */

    int m_iFirstVisible;        /**< Index of the first visible item. */
    SList<ItemInfo> m_lstItems; // List of currently visible items.

    SOsrPanel *m_itemCapture; /**< Item panel that has been set capture. */

    int m_iSelItem;          /**< Index of the selected item. */
    SOsrPanel *m_pHoverItem; /**< Item panel under the hover state. */

    SArray<SList<SItemPanel *> *> m_itemRecycle; // Recycle bin for item panels, one list per style for reuse.

    SXmlDoc m_xmlTemplate;      /**< XML template for item panels. */
    SLayoutSize m_nMarginSize;  /**< Margin size for items. */
    BOOL m_bWantTab;            /**< Flag indicating if tabbing is wanted. */
    BOOL m_bDatasetInvalidated; /**< Flag indicating if the data set is invalidated. */
};

SNSEND

#endif // __STILEVIEW__H__
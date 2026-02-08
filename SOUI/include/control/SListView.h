#ifndef __SLISTVIEW__H__
#define __SLISTVIEW__H__

#include <core/SPanel.h>
#include <core/SItemPanel.h>
#include <interface/SAdapter-i.h>
#include <interface/SListViewItemLocator-i.h>
#include <proxy/SPanelProxy.h>
#include "SViewBase.h"

SNSBEGIN

class SOUI_EXP SListView
    : public TPanelProxy<IListView>
    , public SViewBase {
    DEF_SOBJECT(SPanel, L"listview")

    friend class SListViewDataSetObserver;

  public:
    /**
     * @brief Constructor
     */
    SListView();

    /**
     * @brief Destructor
     */
    ~SListView();

    /**
     * @brief Set the adapter for the list view
     * @param adapter Pointer to the list view adapter
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter *adapter) OVERRIDE;

    /**
     * @brief Get the adapter for the list view
     * @return Pointer to the list view adapter
     */
    STDMETHOD_(ILvAdapter *, GetAdapter)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the item locator for the list view
     * @return Pointer to the item locator
     */
    STDMETHOD_(IListViewItemLocator *, GetItemLocator)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the item locator for the list view
     * @param pItemLocator Pointer to the item locator
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator *pItemLocator) OVERRIDE;

    /**
     * @brief Ensure an item is visible
     * @param iItem Index of the item to ensure visible
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) OVERRIDE;

    /**
     * @brief Set the selected item
     * @param iItem Index of the item to select
     * @param bNotify Whether to notify of the change
     */
    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify = FALSE) OVERRIDE;

    /**
     * @brief Get the selected item
     * @return Index of the selected item
     */
    STDMETHOD_(int, GetSel)(THIS) SCONST OVERRIDE;

    /**
     * @brief Hit test to determine the item under the mouse
     * @param pt Pointer to the mouse coordinates
     * @return Pointer to the item panel if found, NULL otherwise
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
     * @brief Hit test to determine the item under the mouse
     * @param pt Mouse coordinates
     * @return Pointer to the item panel if found, NULL otherwise
     */
    SItemPanel *HitTest(CPoint &pt) const;

    /**
     * @brief Get the desired size of the control
     * @param psz Output size
     * @param nParentWid Parent container width
     * @param nParentHei Parent container height
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int nParentWid, int nParentHei) OVERRIDE;
  protected:
    /**
     * @brief Handle scroll event
     * @param bVertical Whether the scroll is vertical
     * @param uCode Scroll type
     * @param nPos Scroll position
     * @return TRUE if handled, FALSE otherwise
     */
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);

    /**
     * @brief Get the scroll line size
     * @param bVertical Whether the scroll is vertical
     * @return Scroll line size
     */
    virtual int GetScrollLineSize(BOOL bVertical);

    /**
     * @brief Create child items from XML configuration
     * @param xmlNode XML node for the child items
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Update tooltip information
     * @param pt Mouse coordinates
     * @param tipInfo Tooltip information
     * @return TRUE if handled, FALSE otherwise
     */
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);

    /**
     * @brief Handle set cursor event
     * @param pt Mouse coordinates
     * @return TRUE if handled, FALSE otherwise
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Dispatch messages to items
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     */
    void DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Update the scroll bar
     */
    void UpdateScrollBar();

    /**
     * @brief Get the item panel for a specific item
     * @param iItem Index of the item
     * @return Pointer to the item panel
     */
    SItemPanel *GetItemPanel(int iItem) override;

    /**
     * @brief Update visible items
     */
    void UpdateVisibleItems() override;

    /**
     * @brief Update a specific visible item
     * @param iItem Index of the item
     */
    void UpdateVisibleItem(int iItem) override;

    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle size change event
     * @param nType Size change type
     * @param size New size
     */
    void OnSize(UINT nType, CSize size);

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

    /**
     * @brief Handle mouse event
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle key event
     * @param uMsg Message identifier
     * @param wParam Additional message-specific information
     * @param lParam Additional message-specific information
     * @return Result of the message processing
     */
    LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handle key down event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle mouse wheel event
     * @param nFlags Flags
     * @param zDelta Delta value
     * @param pt Mouse coordinates
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    
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
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, WM_IME_KEYLAST, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_CHAR, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_REQUEST, OnKeyEvent)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"dividerSkin", m_pSkinDivider, TRUE)
        ATTR_LAYOUTSIZE(L"dividerSize", m_nDividerSize, FALSE)
        ATTR_BOOL(L"vertical", m_bVertical, FALSE)
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
    virtual ILvAdapter *getAdapter() override { return m_adapter; }
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

  protected:
    SAutoRefPtr<ILvAdapter> m_adapter;                 /**< Pointer to the list view adapter */
    SAutoRefPtr<ILvDataSetObserver> m_observer;        /**< Pointer to the data set observer */
    SAutoRefPtr<IListViewItemLocator> m_lvItemLocator; /**< Pointer to the item locator */

    SAutoRefPtr<ISkinObj> m_pSkinDivider; /**< Skin for dividers */
    SLayoutSize m_nDividerSize;           /**< Size of dividers */
    BOOL m_bVertical;                     /**< Flag indicating vertical orientation */
};

SNSEND

#endif // __SLISTVIEW__H__
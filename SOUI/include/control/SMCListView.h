#ifndef __SMCLISTVIEW__H__
#define __SMCLISTVIEW__H__

#include "core/SPanel.h"
#include "core/SItemPanel.h"
#include "SHeaderCtrl.h"

SNSBEGIN

/**
 * @class SMCListView
 * @brief Multi-Column List View Control
 * @details A control that displays a list of items with multiple columns.
 */
class SOUI_EXP SMCListView
    : public TPanelProxy<IMcListView>
    , protected SHostProxy
    , protected IItemContainer {
    DEF_SOBJECT(SPanel, L"mclistview")

    friend class SMCListViewDataSetObserver;

  public:
    /**
     * @brief Constructor
     */
    SMCListView();

    /**
     * @brief Destructor
     */
    virtual ~SMCListView();

  public:
    /**
     * @brief Set the adapter for the list view
     * @param adapter Pointer to the multi-column adapter
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ IMcAdapter *adapter) OVERRIDE;

    /**
     * @brief Get the adapter for the list view
     * @return Pointer to the multi-column adapter
     */
    STDMETHOD_(IMcAdapter *, GetAdapter)(THIS) SCONST OVERRIDE;

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
     * @brief Get the header control
     * @return Pointer to the header control
     */
    STDMETHOD_(IHeaderCtrl *, GetIHeaderCtrl)(THIS) SCONST OVERRIDE;

    /**
     * @brief Insert a column
     * @param nIndex Index at which to insert the column
     * @param pszText Column title
     * @param nWidth Column width
     * @param fmt Format flags
     * @param lParam Additional parameter
     * @param bDpiAware DPI awareness flag
     * @param fWeight Font weight
     * @return Index of the inserted column
     */
    STDMETHOD_(int, InsertColumn)
    (THIS_ int nIndex, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam = 0, BOOL bDpiAware = TRUE, float fWeight = 0.0f) OVERRIDE;

    /**
     * @brief Delete a specific column
     * @param iCol Index of the column to delete
     */
    STDMETHOD_(void, DeleteColumn)(THIS_ int iCol) OVERRIDE;

    /**
     * @brief Get the total number of columns
     * @return Number of columns
     */
    STDMETHOD_(int, GetColumnCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the desired size of the control
     * @param psz Output size
     * @param nParentWid Parent container width
     * @param nParentHei Parent container height
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int nParentWid, int nParentHei) OVERRIDE;

  public:
    /**
     * @brief Hit test to determine the item under the mouse
     * @param pt Mouse coordinates
     * @return Pointer to the item panel if found, NULL otherwise
     */
    SItemPanel *HitTest(CPoint &pt) const;

    /**
     * @brief Get the header control
     * @return Pointer to the header control
     */
    SHeaderCtrl *GetHeaderCtrl() const;

    /**
     * @brief Update visible items
     */
    void UpdateVisibleItems();

    /**
     * @brief Update a specific visible item
     * @param iItem Index of the item
     */
    void UpdateVisibleItem(int iItem);

  protected:
    /**
     * @brief Handle item capture
     * @param pItem Pointer to the item panel
     * @param bCapture Capture flag
     */
    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture);

    /**
     * @brief Get the rectangle of an item
     * @param pItem Pointer to the item panel
     * @param rcItem Output rectangle
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const;

    /**
     * @brief Check if item redraw is delayed
     * @return TRUE if redraw is delayed, FALSE otherwise
     */
    virtual BOOL IsItemRedrawDelay() const;

    virtual BOOL IsTimelineEnabled() const;

  protected:
    /**
     * @brief Handle data set changed event
     */
    void onDataSetChanged();

    /**
     * @brief Handle data set invalidated event
     */
    void onDataSetInvalidated();

    /**
     * @brief Handle item data changed event
     * @param iItem Index of the item
     */
    void onItemDataChanged(int iItem);

  protected:
    /**
     * @brief Handle item click event
     * @param pEvt Event arguments
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnItemClick(IEvtArgs *pEvt);

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
     * @brief Get the dialog code
     * @return Dialog code
     */
    virtual UINT WINAPI OnGetDlgCode() const;

    /**
     * @brief Handle set cursor event
     * @param pt Mouse coordinates
     * @return TRUE if handled, FALSE otherwise
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Handle colorization event
     * @param cr Color reference
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief Handle scale change event
     * @param nScale Scale factor
     */
    virtual void OnScaleChanged(int nScale);

    /**
     * @brief Handle language change event
     * @return HRESULT
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Handle rebuild font event
     */
    virtual void OnRebuildFont();

  protected:
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
     * @brief Redraw a specific item
     * @param pItem Pointer to the item panel
     */
    void RedrawItem(SOsrPanel *pItem);

    /**
     * @brief Get the item panel for a specific item
     * @param iItem Index of the item
     * @return Pointer to the item panel
     */
    SItemPanel *GetItemPanel(int iItem);

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
     * @brief Handle mouse leave event
     */
    void OnMouseLeave();

    /**
     * @brief Handle mouse wheel event
     * @param nFlags Flags
     * @param zDelta Delta value
     * @param pt Mouse coordinates
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handle kill focus event
     * @param wndFocus New focus window handle
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handle set focus event
     * @param wndOld Previous focus window handle
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Handle show window event
     * @param bShow Show flag
     * @param nStatus Status code
     */
    void OnShowWindow(BOOL bShow, UINT nStatus);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, WM_IME_KEYLAST, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_CHAR, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_REQUEST, OnKeyEvent)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"headerHeight", m_nHeaderHeight, FALSE)
        ATTR_INT(L"hotTrack", m_bHotTrack, FALSE)
        ATTR_SKIN(L"dividerSkin", m_pSkinDivider, TRUE)
        ATTR_LAYOUTSIZE(L"dividerSize", m_nDividerSize, FALSE)
        ATTR_INT(L"wantTab", m_bWantTab, FALSE)
        ATTR_COLOR(L"colorGrid", m_crGrid, TRUE)
    SOUI_ATTRS_END()

  protected:
    SAutoRefPtr<IMcAdapter> m_adapter;                 /**< Pointer to the multi-column adapter */
    SAutoRefPtr<ILvDataSetObserver> m_observer;        /**< Pointer to the data set observer */
    SAutoRefPtr<IListViewItemLocator> m_lvItemLocator; /**< Pointer to the item locator */

    struct ItemInfo
    {
        SItemPanel *pItem; /**< Pointer to the item panel */
        int nType;         /**< Type of the item */
    };

    bool m_bPendingUpdate;    /**< Flag indicating pending update */
    int m_iPendingUpdateItem; /**< Index of the item to update, -1 for all, -2 for nothing */
    int m_iPendingViewItem;   /**< Index of the item to view, -1 for init */

    int m_iFirstVisible;        /**< Index of the first visible item */
    SList<ItemInfo> m_lstItems; /**< List of currently visible items */
    SOsrPanel *m_itemCapture;   /**< Item panel that has been set capture */

    int m_iSelItem;          /**< Index of the selected item */
    SOsrPanel *m_pHoverItem; /**< Item panel under the mouse */

    SArray<SList<SItemPanel *> *> m_itemRecycle; /**< Recycle bin for item panels */

    SXmlDoc m_xmlTemplate;                /**< XML template for items */
    SAutoRefPtr<ISkinObj> m_pSkinDivider; /**< Skin for dividers */
    SLayoutSize m_nDividerSize;           /**< Size of dividers */
    BOOL m_bWantTab;                      /**< Flag indicating whether to want tab */
    BOOL m_bDatasetInvalidated;           /**< Flag indicating data set is invalidated */
    COLORREF m_crGrid;                    /**< Grid color */

  protected:
    /**
     * @brief Handle header click event
     * @param pEvt Event arguments
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnHeaderClick(IEvtArgs *pEvt);

    /**
     * @brief Handle header size changing event
     * @param pEvt Event arguments
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnHeaderSizeChanging(IEvtArgs *pEvt);

    /**
     * @brief Handle header swap event
     * @param pEvt Event arguments
     * @return TRUE if handled, FALSE otherwise
     */
    BOOL OnHeaderSwap(IEvtArgs *pEvt);

    /**
     * @brief Get the rectangle of the list
     * @return Rectangle of the list
     */
    CRect GetListRect();

    /**
     * @brief Update the header control
     */
    void UpdateHeaderCtrl();

    /**
     * @brief Update the position of child items
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    /**
     * @brief Get the height of the header
     * @return Height of the header
     */
    int GetHeaderHeight() const;

  protected:
    /**
     * @brief Get the rectangle of an item by position
     * @param iPosition Position of the item
     * @return Rectangle of the item
     */
    CRect _OnItemGetRect(int iPosition) const;

    /**
     * @brief Update the column widths in the adapter
     */
    void _UpdateAdapterColumnsWidth() const;

  protected:
    SHeaderCtrl *m_pHeader;      /**< Pointer to the header control */
    SLayoutSize m_nHeaderHeight; /**< Height of the header */
    BOOL m_bHotTrack;            /**< Hot tracking flag */
};

SNSEND

#endif // __SMCLISTVIEW__H__
#ifndef __STREEVIEW__H__
#define __STREEVIEW__H__

#include <core/SWnd.h>
#include <core/SPanel.h>
#include <core/SItemPanel.h>
#include <interface/SAdapter-i.h>
#include <interface/STreeViewItemLocator-i.h>
#include <proxy/SPanelProxy.h>

SNSBEGIN

/**
 * @class STreeViewItemLocator
 * @brief Class responsible for locating and managing tree view items.
 */
class SOUI_EXP STreeViewItemLocator : public TObjRefImpl<ITreeViewItemLocator> {
  public:
    /**
     * @brief Constructor for STreeViewItemLocator.
     * @param nIndent Indentation between levels.
     */
    STreeViewItemLocator(int nIndent = 16);

    /**
     * @brief Destructor for STreeViewItemLocator.
     */
    ~STreeViewItemLocator();

  public:
    /**
     * @brief Sets the adapter for the tree view items.
     * @param pAdapter Pointer to the adapter.
     */
    STDMETHOD_(void, SetAdapter)(THIS_ ITvAdapter *pAdapter) OVERRIDE;

    /**
     * @brief Notifies the locator that a branch has changed.
     * @param hItem Handle to the branch item.
     */
    STDMETHOD_(void, OnBranchChanged)(THIS_ HSTREEITEM hItem) OVERRIDE;

    /**
     * @brief Notifies the locator that the expanded state of a branch has changed.
     * @param hItem Handle to the branch item.
     * @param bExpandedOld Old expanded state.
     * @param bExpandedNew New expanded state.
     */
    STDMETHOD_(void, OnBranchExpandedChanged)
    (THIS_ HSTREEITEM hItem, BOOL bExpandedOld, BOOL bExpandedNew) OVERRIDE;

    /**
     * @brief Gets the total height of the tree view.
     * @return Total height.
     */
    STDMETHOD_(int, GetTotalHeight)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the total width of the tree view.
     * @return Total width.
     */
    STDMETHOD_(int, GetTotalWidth)(THIS) SCONST OVERRIDE;

    /**
     * @brief Converts an item handle to its position.
     * @param hItem Handle to the item.
     * @return Position of the item.
     */
    STDMETHOD_(int, Item2Position)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Converts a position to an item handle.
     * @param position Position in the tree view.
     * @return Handle to the item.
     */
    STDMETHOD_(HSTREEITEM, Position2Item)(THIS_ int position) SCONST OVERRIDE;

    /**
     * @brief Gets the scroll line size.
     * @return Scroll line size.
     */
    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the width of an item.
     * @param hItem Handle to the item.
     * @param nWidth New width of the item.
     */
    STDMETHOD_(void, SetItemWidth)(THIS_ HSTREEITEM hItem, int nWidth) OVERRIDE;

    /**
     * @brief Gets the width of an item.
     * @param hItem Handle to the item.
     * @return Width of the item.
     */
    STDMETHOD_(int, GetItemWidth)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Sets the height of an item.
     * @param hItem Handle to the item.
     * @param nHeight New height of the item.
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ HSTREEITEM hItem, int nHeight) OVERRIDE;

    /**
     * @brief Gets the height of an item.
     * @param hItem Handle to the item.
     * @return Height of the item.
     */
    STDMETHOD_(int, GetItemHeight)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Gets the indentation of an item.
     * @param hItem Handle to the item.
     * @return Indentation of the item.
     */
    STDMETHOD_(int, GetItemIndent)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Gets the indentation between levels.
     * @return Indentation between levels.
     */
    STDMETHOD_(int, GetIndent)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Sets the indentation between levels.
     * @param nIndent New indentation value.
     */
    STDMETHOD_(void, SetIndent)(THIS_ int nIndent) OVERRIDE;

  protected:
    /**
     * @brief Checks if an item is expanded.
     * @param hItem Handle to the item.
     * @return TRUE if the item is expanded, otherwise FALSE.
     */
    BOOL IsItemExpanded(HSTREEITEM hItem) const;

    /**
     * @brief Updates the branch width data in the parent window.
     * @param hItem Handle to the item.
     * @param nOldWidth Old width of the branch.
     * @param nNewWidth New width of the branch.
     */
    void _UpdateBranchWidth(HSTREEITEM hItem, int nOldWidth, int nNewWidth);

    /**
     * @brief Gets the width of a branch.
     * @param hBranch Handle to the branch.
     * @return Width of the branch.
     */
    int _GetBranchWidth(HSTREEITEM hBranch) const;

    /**
     * @brief Sets the width of a branch.
     * @param hBranch Handle to the branch.
     * @param nWidth New width of the branch.
     */
    void _SetBranchWidth(HSTREEITEM hBranch, int nWidth);

    /**
     * @brief Sets the width of an item.
     * @param hItem Handle to the item.
     * @param nWidth New width of the item.
     */
    void _SetItemWidth(HSTREEITEM hItem, int nWidth);

    /**
     * @brief Gets the height of a branch.
     * @param hItem Handle to the item.
     * @return Height of the branch.
     */
    int _GetBranchHeight(HSTREEITEM hItem) const;

    /**
     * @brief Sets the height of a branch.
     * @param hItem Handle to the item.
     * @param nHeight New height of the branch.
     */
    void _SetBranchHeight(HSTREEITEM hItem, int nHeight);

    /**
     * @brief Updates the height of a branch.
     * @param hItem Handle to the item.
     * @param nDiff Difference in height.
     */
    void _UpdateBranchHeight(HSTREEITEM hItem, int nDiff);

    /**
     * @brief Updates the offset of sibling items.
     * @param hItem Handle to the item.
     */
    void _UpdateSiblingsOffset(HSTREEITEM hItem);

    /**
     * @brief Gets the offset of an item.
     * @param hItem Handle to the item.
     * @return Offset of the item.
     */
    int _GetItemOffset(HSTREEITEM hItem) const;

    /**
     * @brief Sets the offset of an item.
     * @param hItem Handle to the item.
     * @param nOffset New offset of the item.
     */
    void _SetItemOffset(HSTREEITEM hItem, int nOffset);

    /**
     * @brief Sets the height of an item.
     * @param hItem Handle to the item.
     * @param nHeight New height of the item.
     */
    void _SetItemHeight(HSTREEITEM hItem, int nHeight);

    /**
     * @brief Gets the visible height of an item.
     * @param hItem Handle to the item.
     * @return Visible height of the item.
     */
    int _GetItemVisibleHeight(HSTREEITEM hItem) const;

    /**
     * @brief Gets the visible width of an item.
     * @param hItem Handle to the item.
     * @return Visible width of the item.
     */
    int _GetItemVisibleWidth(HSTREEITEM hItem) const;

    /**
     * @brief Converts a position to an item handle.
     * @param position Position in the tree view.
     * @param hParent Handle to the parent item.
     * @param nParentPosition Position of the parent item.
     * @return Handle to the item.
     */
    HSTREEITEM _Position2Item(int position, HSTREEITEM hParent, int nParentPosition) const;

    /**
     * @brief Checks if an item is visible.
     * @param hItem Handle to the item.
     * @return TRUE if the item is visible, otherwise FALSE.
     */
    BOOL _IsItemVisible(HSTREEITEM hItem) const;

    /**
     * @brief Initializes a branch.
     * @param hItem Handle to the item.
     */
    void _InitBranch(HSTREEITEM hItem);

    SAutoRefPtr<ITvAdapter> m_adapter; /**< Adapter for the tree view items. */
    int m_nLineHeight;                 /**< Line height. */
    int m_nIndent;                     /**< Indentation between levels. */
    CSize m_szDef;                     /**< Default size. */
};

/**
 * @class STreeView
 * @brief Class representing a tree view control.
 */
class SOUI_EXP STreeView
    : public TPanelProxy<ITreeView>
    , protected SHostProxy
    , protected IItemContainer {
    DEF_SOBJECT(SPanel, L"treeview")
    friend class STreeViewDataSetObserver;

  public:
    /**
     * @brief Constructor for STreeView.
     */
    STreeView();

    /**
     * @brief Destructor for STreeView.
     */
    ~STreeView();

  public:
    /**
     * @brief Sets the adapter for the tree view.
     * @param adapter Pointer to the adapter.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ITvAdapter *adapter) OVERRIDE;

    /**
     * @brief Gets the adapter for the tree view.
     * @return Pointer to the adapter.
     */
    STDMETHOD_(ITvAdapter *, GetAdapter)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the item locator for the tree view.
     * @param pItemLocator Pointer to the item locator.
     */
    STDMETHOD_(void, SetItemLocator)(THIS_ ITreeViewItemLocator *pItemLocator) OVERRIDE;

    /**
     * @brief Gets the item locator for the tree view.
     * @return Pointer to the item locator.
     */
    STDMETHOD_(ITreeViewItemLocator *, GetItemLocator)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the selected item.
     * @param hItem Handle to the item.
     * @param bNotify TRUE to notify listeners, FALSE otherwise.
     */
    STDMETHOD_(void, SetSel)(THIS_ HSTREEITEM hItem, BOOL bNotify = FALSE) OVERRIDE;

    /**
     * @brief Gets the selected item.
     * @return Handle to the selected item.
     */
    STDMETHOD_(HSTREEITEM, GetSel)(THIS) SCONST OVERRIDE;
    

    /**
     * @brief Ensures an item is visible.
     * @param hItem Handle to the item.
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ HSTREEITEM hItem) OVERRIDE;

    /**
     * @brief Performs a hit test on the tree view.
     * @param pt Mouse position.
     * @return Pointer to the item panel at the specified position.
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
    STDMETHOD_(BOOL, GetMultiSel)(THIS) SCONST OVERRIDE;
    
    /**
     * @brief Adds an item to the selection.
     * @param hItem Handle to the item.
     */
    STDMETHOD_(void, AddSelItem)(THIS_ HSTREEITEM hItem) OVERRIDE;
    
    /**
     * @brief Removes an item from the selection.
     * @param hItem Handle to the item.
     */
    STDMETHOD_(void, RemoveSelItem)(THIS_ HSTREEITEM hItem) OVERRIDE;
    
    /**
     * @brief Clears all selected items.
     */
    STDMETHOD_(void, ClearSelItems)(THIS) OVERRIDE;
    
    /**
     * @brief Checks if an item is selected.
     * @param hItem Handle to the item.
     * @return TRUE if the item is selected, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsItemSelected)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Gets the count of selected items.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItemCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets all selected items.
     * @param items Output parameter to store the selected item handles.
     * @param nMaxCount Maximum number of items to retrieve.
     * @return Number of selected items.
     */
    STDMETHOD_(int, GetSelItems)(THIS_ HSTREEITEM *items, int nMaxCount) SCONST OVERRIDE;

  public:
    /**
     * @brief Performs a hit test on the tree view.
     * @param pt Mouse position.
     * @return Pointer to the item panel at the specified position.
     */
    SItemPanel *HitTest(CPoint &pt) const;

  protected:
    /**
     * @brief Handles the item click event.
     * @param pEvt Event arguments.
     * @return TRUE if the event is handled, otherwise FALSE.
     */
    BOOL OnItemClick(IEvtArgs *pEvt);

    /**
     * @brief Handles the item double-click event.
     * @param pEvt Event arguments.
     * @return TRUE if the event is handled, otherwise FALSE.
     */
    BOOL OnItemDblClick(IEvtArgs *pEvt);

    /**
     * @brief Notifies the tree view that a branch has changed.
     * @param hBranch Handle to the branch item.
     */
    void onBranchChanged(HSTREEITEM hBranch);

    /**
     * @brief Notifies the tree view that a branch has been invalidated.
     * @param hBranch Handle to the branch item.
     * @param bInvalidParents TRUE to invalidate parent branches, FALSE otherwise.
     * @param bInvalidChildren TRUE to invalidate child branches, FALSE otherwise.
     */
    void onBranchInvalidated(HSTREEITEM hBranch, BOOL bInvalidParents, BOOL bInvalidChildren);

    /**
     * @brief Notifies the tree view that the expanded state of a branch has changed.
     * @param hBranch Handle to the branch item.
     * @param bExpandedOld Old expanded state.
     * @param bExpandedNew New expanded state.
     */
    void onBranchExpandedChanged(HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew);

    /**
     * @brief Notifies the tree view that an item is about to be removed.
     * @param hItem Handle to the item.
     */
    void onItemBeforeRemove(HSTREEITEM hItem);

  protected:
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
     * @brief Handles the destruction event.
     */
    void OnDestroy();

    /**
     * @brief Handles the key down event.
     * @param nChar Character code.
     * @param nRepCnt Repeat count.
     * @param nFlags Flags associated with the key event.
     */
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handles key events.
     * @param uMsg Message identifier.
     * @param wParam Additional message information.
     * @param lParam Additional message information.
     * @return Result of the message processing.
     */
    LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the mouse leave event.
     */
    void OnMouseLeave();

    /**
     * @brief Handles the mouse wheel event.
     * @param nFlags Flags associated with the mouse event.
     * @param zDelta Wheel delta.
     * @param pt Mouse position.
     * @return TRUE if the event is handled, otherwise FALSE.
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * @brief Handles mouse events.
     * @param uMsg Message identifier.
     * @param wParam Additional message information.
     * @param lParam Additional message information.
     * @return Result of the message processing.
     */
    LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Handles the kill focus event.
     * @param wndFocus Window receiving focus.
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handles the set focus event.
     * @param wndOld Window losing focus.
     */
    void OnSetFocus(SWND wndOld);

    /**
     * @brief Handles the left mouse button down event.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, WM_IME_KEYLAST, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_CHAR, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_REQUEST, OnKeyEvent)
    SOUI_MSG_MAP_END()

  protected:
    /**
     * @brief Handles the indentation attribute.
     * @param strValue Attribute value.
     * @param bLoading TRUE if loading, FALSE otherwise.
     * @return Result of the attribute handling.
     */
    HRESULT OnAttrIndent(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"indent", OnAttrIndent)
        ATTR_INT(L"wantTab", m_bWantTab, FALSE)
        ATTR_BOOL(L"hasLines", m_bHasLines, TRUE)
        ATTR_BOOL(L"multiSel", m_bMultiSel, FALSE)
        ATTR_SKIN(L"lineSkin", m_pLineSkin, TRUE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Gets the dialog code for the tree view.
     * @return Dialog code.
     */
    virtual UINT WINAPI OnGetDlgCode() const;

    /**
     * @brief Sets the cursor for the tree view.
     * @param pt Mouse position.
     * @return TRUE if the cursor is set, otherwise FALSE.
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * @brief Updates the tooltip information.
     * @param pt Mouse position.
     * @param tipInfo Tooltip information.
     * @return TRUE if the tooltip is updated, otherwise FALSE.
     */
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);

    /**
     * @brief Applies colorization to the tree view.
     * @param cr Color reference.
     */
    virtual void OnColorize(COLORREF cr);

    /**
     * @brief Handles scale changes.
     * @param nScale New scale.
     */
    virtual void OnScaleChanged(int nScale);

    /**
     * @brief Handles language change events.
     * @return Result of the language change handling.
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Rebuilds the font settings.
     */
    virtual void OnRebuildFont();

    /**
     * @brief Handles scroll events.
     * @param bVertical TRUE if the scroll is vertical, FALSE if horizontal.
     * @param uCode Scroll code.
     * @param nPos New position.
     * @return TRUE if the scroll event is handled, otherwise FALSE.
     */
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);

    /**
     * @brief Gets the scroll line size.
     * @param bVertical TRUE if the scroll is vertical, FALSE if horizontal.
     * @return Scroll line size.
     */
    virtual int GetScrollLineSize(BOOL bVertical);

    /**
     * @brief Creates child elements from XML node.
     * @param xmlNode XML node containing child elements.
     * @return TRUE if successful, otherwise FALSE.
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Sets or releases mouse capture for an item.
     * @param pItem Pointer to the item panel.
     * @param bCapture TRUE to set capture, FALSE to release capture.
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
     * @brief Indicates if item redraw is delayed.
     * @return TRUE if redraw is delayed, otherwise FALSE.
     */
    virtual BOOL IsItemRedrawDelay() const;

    virtual BOOL IsTimelineEnabled() const;

    /**
     * @brief Draws lines for an item.
     * @param pRT Pointer to the rendering target.
     * @param rc Rectangle to draw the lines.
     * @param hItem Handle to the item.
     */
    virtual void DrawLines(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem);

  protected:
    /**
     * @brief Updates the scroll bar.
     */
    void UpdateScrollBar();

    /**
     * @brief Updates the visible items.
     */
    void UpdateVisibleItems();

    /**
     * @brief Redraws an item.
     * @param pItem Pointer to the item panel.
     */
    void RedrawItem(SItemPanel *pItem);

    /**
     * @brief Gets the item panel for a given item handle.
     * @param hItem Handle to the item.
     * @return Pointer to the item panel.
     */
    SItemPanel *GetItemPanel(HSTREEITEM hItem);

    /**
     * @brief Dispatches a message to items.
     * @param uMsg Message identifier.
     * @param wParam Additional message information.
     * @param lParam Additional message information.
     */
    void DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam);

  protected:
    SAutoRefPtr<ITvAdapter> m_adapter;                 /**< Adapter for the tree view items. */
    SAutoRefPtr<ITvDataSetObserver> m_observer;        /**< Observer for data set changes. */
    SAutoRefPtr<ITreeViewItemLocator> m_tvItemLocator; /**< Item locator for the tree view. */

    struct ItemInfo
    {
        SItemPanel *pItem; /**< Pointer to the item panel. */
        int nType;         /**< Type of the item. */
    };
    SArray<SList<SItemPanel *> *> m_itemRecycle; /**< Item recycle bin, each style has a list for reuse. */
    SList<ItemInfo> m_visible_items;             /**< List of visible items. */

    typedef SMap<HSTREEITEM, ItemInfo> VISIBLEITEMSMAP;
    VISIBLEITEMSMAP *m_pVisibleMap; /**< Map of visible items. */

    SXmlDoc m_xmlTemplate; /**< XML template for items. */

    SOsrPanel *m_itemCapture; /**< Item that has capture. */
    SOsrPanel *m_pHoverItem;  /**< Item under hover state. */

    HSTREEITEM m_hSelected; /**< Current selected item. */

    BOOL m_bWantTab;                   /**< Flag indicating if tab is wanted. */
    BOOL m_bHasLines;                  /**< Flag indicating if lines are drawn between items. */
    BOOL m_bMultiSel;                  /**< Flag indicating if multiple selection is enabled. */
    SAutoRefPtr<ISkinObj> m_pLineSkin; /**< Skin for the lines. */
    SLayoutSize m_indent;              /**< Indentation between levels. */
    
    typedef SMap<HSTREEITEM, BOOL> ItemSelectionMap;
    ItemSelectionMap m_mapSelItems; /**< Map of selected items. */
};
SNSEND
#endif // __STREEVIEW__H__
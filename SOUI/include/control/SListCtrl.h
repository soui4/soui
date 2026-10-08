#ifndef __SLISTCTRL__H__
#define __SLISTCTRL__H__

#include "core/SPanel.h"
#include "SHeaderCtrl.h"

SNSBEGIN

/**
 * @enum SListCtrlFlags
 * @brief Flags for list control items
 * @details Flags used to specify which attributes of a list item are valid.
 */
enum SListCtrlFlags
{
    S_LVIF_TEXT = 0x01,   /**< Text attribute is valid */
    S_LVIF_IMAGE = 0x02,  /**< Image attribute is valid */
    S_LVIF_INDENT = 0x04, /**< Indent attribute is valid */
};

/**
 * @typedef PFNLVCOMPAREEX
 * @brief Comparison function type for sorting
 * @details Function pointer type for the comparison function used in sorting.
 */
typedef int(__cdecl *PFNLVCOMPAREEX)(void *, const void *, const void *);

/**
 * @struct DXLVSUBITEM
 * @brief Subitem structure
 * @details Structure representing a subitem in the list control.
 */
typedef struct DXLVSUBITEM
{
    /**
     * @brief Constructor
     */
    DXLVSUBITEM()
    {
        mask = 0;
        nImage = 0;
        strText = NULL;
        cchTextMax = 0;
        nIndent = 0;
    }

    UINT mask;      /**< Mask indicating which attributes are valid */
    LPTSTR strText; /**< Text of the subitem */
    int cchTextMax; /**< Maximum length of the text */
    UINT nImage;    /**< Icon index */
    int nIndent;    /**< Indent level */
} DXLVSUBITEM;

typedef SArray<DXLVSUBITEM> ArrSubItem; /**< Array of subitems */

/**
 * @struct DXLVITEM
 * @brief Item structure
 * @details Structure representing an item in the list control.
 */
typedef struct DXLVITEM
{
    /**
     * @brief Constructor
     */
    DXLVITEM()
    {
        dwData = 0;
        arSubItems = NULL;
        checked = FALSE;
    }

    ArrSubItem *arSubItems; /**< Array of subitems */
    LPARAM dwData;          /**< Additional data */
    BOOL checked;           /**< Check state */
} DXLVITEM;

/**
 * @class SListCtrl
 * @brief List Control
 * @details A control that displays a list of items with multiple columns and subitems.
 */
class SOUI_EXP SListCtrl : public SPanel {
    DEF_SOBJECT(SPanel, L"listctrl")

  public:
    /**
     * @brief Constructor
     */
    SListCtrl();

    /**
     * @brief Destructor
     */
    virtual ~SListCtrl();

    /**
     * @brief Insert a column
     * @param nIndex Index at which to insert the column
     * @param pszText Column title
     * @param nWidth Column width
     * @param fmt Format flags
     * @param lParam Additional parameter
     * @return Index of the inserted column
     */
    int InsertColumn(int nIndex, LPCTSTR pszText, int nWidth, UINT fmt, LPARAM lParam = 0);

    /**
     * @brief Insert an item
     * @param nItem Index at which to insert the item
     * @param pszText Text of the item
     * @param nImage Icon index
     * @return Index of the inserted item
     */
    int InsertItem(int nItem, LPCTSTR pszText, int nImage = -1);

    /**
     * @brief Set the data associated with an item
     * @param nItem Index of the item
     * @param dwData Additional data
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL SetItemData(int nItem, LPARAM dwData);

    /**
     * @brief Get the data associated with an item
     * @param nItem Index of the item
     * @return Additional data
     */
    LPARAM GetItemData(int nItem);

    /**
     * @brief Set a subitem
     * @param nItem Index of the item
     * @param nSubItem Index of the subitem
     * @param plv Pointer to the subitem structure
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL SetSubItem(int nItem, int nSubItem, const DXLVSUBITEM *plv);

    /**
     * @brief Get a subitem
     * @param nItem Index of the item
     * @param nSubItem Index of the subitem
     * @param plv Pointer to the subitem structure
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL GetSubItem(int nItem, int nSubItem, DXLVSUBITEM *plv) const;

    /**
     * @brief Set the text of a subitem
     * @param nItem Index of the item
     * @param nSubItem Index of the subitem
     * @param pszText Text to set
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL SetSubItemText(int nItem, int nSubItem, LPCTSTR pszText);

    /**
     * @brief Get the text of a subitem
     * @param nItem Index of the item
     * @param nSubItem Index of the subitem
     * @return Text of the subitem
     */
    SStringT GetSubItemText(int nItem, int nSubItem) const;

    /**
     * @brief Get the selected item
     * @return Index of the selected item
     */
    int GetSelectedItem();

    /**
     * @brief Set the selected item
     * @param nItem Index of the item to select
     */
    void SetSelectedItem(int nItem);

    /**
     * @brief Get the selected column
     * @return Index of the selected column
     */
    int GetSelectedColumn();

    /**
     * @brief Set the selected column
     * @param nColumn Index of the column to select
     */
    void SetSelectedColumn(int nColumn);

    /**
     * @brief Get the total number of items
     * @return Number of items
     */
    int GetItemCount() const;

    /**
     * @brief Set the total number of items
     * @param nItems Number of items
     * @param nGrowBy Growth increment
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL SetItemCount(int nItems, int nGrowBy);

    /**
     * @brief Get the total number of columns
     * @return Number of columns
     */
    int GetColumnCount() const;

    /**
     * @brief Get the number of items per page
     * @param bPartial Whether to include partial items
     * @return Number of items per page
     */
    int GetCountPerPage(BOOL bPartial);

    /**
     * @brief Delete a specific item
     * @param nItem Index of the item to delete
     */
    void DeleteItem(int nItem);

    /**
     * @brief Delete a specific column
     * @param iCol Index of the column to delete
     */
    void DeleteColumn(int iCol);

    /**
     * @brief Delete all items
     */
    void DeleteAllItems();

    /**
     * @brief Sort items using a comparison function
     * @param pfnCompare Comparison function
     * @param pContext Context for the comparison function
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL SortItems(PFNLVCOMPAREEX pfnCompare, void *pContext);

    /**
     * @brief Get the check state of an item
     * @param nItem Index of the item
     * @return Check state of the item
     */
    BOOL GetCheckState(int nItem);

    /**
     * @brief Set the check state of an item
     * @param nItem Index of the item
     * @param bCheck Check state to set
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL SetCheckState(int nItem, BOOL bCheck);

    /**
     * @brief Get the number of checked items
     * @return Number of checked items
     */
    int GetCheckedItemCount();

    /**
     * @brief Get the number of selected items
     *
     * Mode-aware: multi-selection mode counts the checked items (the only
     * selection record); single-selection mode reports 1 while the anchor
     * is set.
     *
     * @return Number of selected items
     */
    int GetSelItemCount() const;

    /**
     * @brief Get all selected items
     *
     * Mode-aware: multi-selection mode enumerates the checked items;
     * single-selection mode reports the anchor.
     *
     * @param items Output array of item indexes.
     * @param nMaxCount Maximum number of indexes to retrieve.
     * @return Number of indexes retrieved.
     */
    int GetSelItems(int *items, int nMaxCount) const;

    /**
     * @brief Get the header control
     * @return Pointer to the header control
     */
    SHeaderCtrl *GetHeaderCtrl() const;

    /**
     * @brief Get the first checked item
     * @return Index of the first checked item
     */
    int GetFirstCheckedItem();

    /**
     * @brief Get the last checked item
     * @return Index of the last checked item
     */
    int GetLastCheckedItem();

    /**
     * @brief Enable or disable multiple selection
     *
     * Dual-track model: in single-selection mode the anchor m_nSelectItem is
     * the selection record (the checked flag of that item is kept in sync);
     * in multi-selection mode the checked flags are the only record and the
     * anchor acts as the cursor (keyboard / shift base) only. Enabling is a
     * no-op for the record (the checked flag already mirrors the anchor);
     * disabling transfers the sole checked item to the anchor or clears all.
     *
     * @param enable Enable flag
     */
    VOID EnableMultiSelection(BOOL enable)
    {
        if (!enable && m_bMultiSelection)
        {
            // Turning multi-selection off. The checked flag marks the
            // selection in both modes; the anchor m_nSelectItem must follow
            // it (single-selection paths read m_nSelectItem).
            int iChecked = -1;
            int nSelected = 0;
            for (int i = 0; i < GetItemCount(); i++)
            {
                if (m_arrItems[i].checked)
                {
                    nSelected++;
                    if (iChecked == -1)
                        iChecked = i;
                }
            }
            if (nSelected > 1)
            {
                // Several items checked: the whole selection is cleared.
                for (int i = 0; i < GetItemCount(); i++)
                {
                    if (m_arrItems[i].checked)
                    {
                        m_arrItems[i].checked = FALSE;
                        RedrawItem(i);
                        NotifyItemSelState(i, FALSE);
                    }
                }
                m_nSelectItem = -1;
                m_nSelAnchor = -1;
            }
            else if (nSelected == 1)
            {
                // Exactly one item checked: it becomes the single selection
                // (the flag is already set, only the anchor moves).
                m_nSelectItem = iChecked;
                m_nSelAnchor = iChecked;
            }
            // nSelected == 0: the anchor is kept unchanged.
        }
        else if (enable && !m_bMultiSelection)
        {
            // Turning multi-selection on: the range anchor starts at the
            // cursor so the first Shift+Click/arrow works from there.
            m_nSelAnchor = m_nSelectItem;
        }
        m_bMultiSelection = enable;
    }

    /**
     * @brief Enable or disable checkboxes
     * @param enable Enable flag
     */
    VOID EnableCheckBox(BOOL enable)
    {
        m_bCheckBox = enable;
    }

    /**
     * @brief Enable or disable hot tracking
     * @param enable Enable flag
     */
    VOID EnableHotTrack(BOOL enable)
    {
        m_bHotTrack = enable;
    }

  protected:
    /**
     * @brief Create child items from XML configuration
     * @param xmlNode XML node for the child items
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE;

    /**
     * @brief Hit test to determine the item under the mouse
     * @param pt Mouse coordinates
     * @return Index of the item or -1 if no item
     */
    int HitTest(const CPoint &pt);

    /**
     * @brief Hit test to determine the item and subitem under the mouse
     * @param pt Mouse coordinates
     * @param pnSubItem Pointer to receive the subitem index
     * @return Index of the item or -1 if no item
     */
    int HitTest(const CPoint &pt, int *pnSubItem);

    /**
     * @brief Get the index of the top visible item
     * @return Index of the top visible item
     */
    int GetTopIndex() const;

    /**
     * @brief Get the rectangle of an item
     * @param nItem Index of the item
     * @param nSubItem Index of the subitem
     * @return Rectangle of the item
     */
    CRect GetItemRect(int nItem, int nSubItem = 0);

    /**
     * @brief Draw an item
     * @param pRT Rendering target handle
     * @param rcItem Rectangle for the item
     * @param nItem Index of the item
     */
    virtual void DrawItem(IRenderTarget *pRT, CRect rcItem, int nItem);

    /**
     * @brief Redraw a specific item
     * @param nItem Index of the item to redraw
     */
    void RedrawItem(int nItem);

    /**
     * @brief Notify of selection change
     * @param nOldSel Old selected index
     * @param nNewSel New selected index
     * @param checkBox Whether the change is due to a checkbox
     * @param nFlags Modifier keys from the mouse message wParam (MK_CONTROL / MK_SHIFT);
     *              pass 0 for programmatic changes. Replaces the former
     *              GetKeyState() lookups so keyboard state is testable headless.
     */
    void NotifySelChange(int nOldSel, int nNewSel, BOOL checkBox = FALSE, UINT nFlags = 0);

    /**
     * @brief Fire the per-item selection state event (multi-selection)
     * @param iItem Index of the item whose checked / selected state flipped
     * @param bSelected New state: TRUE=selected(checked), FALSE=deselected
     */
    void NotifyItemSelState(int iItem, BOOL bSelected)
    {
        EventItemSelChanged evt(this);
        evt.iItem = iItem;
        evt.bSelected = bSelected;
        FireEvent(evt);
    }

    /**
     * @brief Scrolls the list so an item becomes visible (keyboard navigation)
     * @param nItem Index of the item to reveal
     */
    void EnsureVisible(int nItem);

    /**
     * @brief Handle key down event (arrows / PgUp / PgDn / Home / End /
     *        SPACE toggle / Ctrl+A, with Ctrl- and Shift- modifiers)
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Reports the arrow keys as wanted dialog codes so the control
     *        receives keyboard navigation
     * @return Dialog codes
     */
    virtual UINT WINAPI OnGetDlgCode() const OVERRIDE;

    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

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
     * @brief Handle scroll event
     * @param bVertical Whether the scroll is vertical
     * @param uCode Scroll type
     * @param nPos Scroll position
     * @return TRUE if handled, FALSE otherwise
     */
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos) OVERRIDE;

    /**
     * @brief Handle left mouse button double-click event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonDbClick(UINT nFlags, CPoint pt);

    /**
     * @brief Handle right mouse button up event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnRButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse leave event
     */
    void OnMouseLeave();

    /**
     * @brief Handle focus loss: the keyboard cursor frame is drawn only
     *        while the control is focused, so repaint
     * @param wndFocus Handle of the window receiving focus
     */
    void OnKillFocus(SWND wndFocus);

    /**
     * @brief Handle size change event
     * @param nType Size change type
     * @param size New size
     */
    void OnSize(UINT nType, CSize size);

  protected:
    /**
     * @brief Handle left mouse button down when not drag scrolling
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonDownEx(UINT nFlags, CPoint pt) override;

    /**
     * @brief Handle mouse move when not drag scrolling
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnMouseMoveEx(UINT nFlags, CPoint pt) override;

    /**
     * @brief Handle left mouse button up when not drag scrolling
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonUpEx(UINT nFlags, CPoint pt) override;

    /**
     * @brief Checks whether rubber band selection is enabled (multi-selection on).
     */
    virtual BOOL IsRubberBandSelEnabled() const override;

    /**
     * @brief Records the selection anchor when the band starts.
     */
    virtual void OnRubberBandStart() override;

    /**
     * @brief Updates the checked state so it matches the rows covered by the band.
     * @param rcBand Band rectangle in client coordinates.
     * @param bAdd TRUE to add to the existing selection (Ctrl held).
     */
    virtual void OnRubberBandSelect(const CRect &rcBand, BOOL bAdd) override;

    /**
     * @brief Fires the selection changed event when the band finishes.
     */
    virtual void OnRubberBandEnd(const CRect &rcBand, BOOL bCancelled) override;

    /**
     * @brief Update the position of child items
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    /**
     * @brief Get the rectangle of the list
     * @return Rectangle of the list
     */
    CRect GetListRect();

    /**
     * @brief Update the scroll bar
     */
    void UpdateScrollBar();

    /**
     * @brief Update the header control
     */
    void UpdateHeaderCtrl();

    /**
     * @brief Hit test to determine if the point is on a checkbox
     * @param pt Mouse coordinates
     * @return TRUE if on a checkbox, FALSE otherwise
     */
    BOOL HitCheckBox(const CPoint &pt);

  protected:
    SLayoutSize m_nHeaderHeight; /**< Height of the header */
    SLayoutSize m_nItemHeight;   /**< Height of the items */

    int m_nSelectItem;              /**< Index of the selected item (single) / keyboard cursor (multi) */
    int m_nSelAnchor;               /**< Multi-selection range anchor: fixed base for Shift ranges; follows the cursor on every non-Shift change */
    int m_nSelectColumn;            /**< Index of the selected column */
    int m_nHoverItem;               /**< Index of the item under the mouse */
    BOOL m_bHotTrack;               /**< Hot tracking flag */
    int m_iBandOldSel;              /**< Selection anchor before a rubber band started */
    SArray<BOOL> m_arrBandSnapshot; /**< Checked-state snapshot taken when the rubber band starts */

    CPoint m_ptIcon; /**< Icon position */
    CPoint m_ptText; /**< Text position */

    COLORREF m_crItemBg;    /**< Background color */
    COLORREF m_crItemBg2;   /**< Background color for even rows */
    COLORREF m_crItemSelBg; /**< Selected item background color */
    COLORREF m_crItemHotBg; /**< Hot item background color */
    COLORREF m_crText;      /**< Text color */
    COLORREF m_crSelText;   /**< Selected text color */

    SAutoRefPtr<ISkinObj> m_pItemSkin;  /**< Skin for items */
    SAutoRefPtr<ISkinObj> m_pIconSkin;  /**< Skin for icons */
    SAutoRefPtr<ISkinObj> m_pCheckSkin; /**< Skin for checkboxes */
    BOOL m_bCheckBox;                   /**< Checkbox enable flag */
    BOOL m_bMultiSelection;             /**< Multiple selection enable flag */

  protected:
    typedef SArray<DXLVITEM> ArrLvItem; /**< Array of items */

    SHeaderCtrl *m_pHeader; /**< Header control */
    ArrLvItem m_arrItems;   /**< Array of items */
    CPoint m_ptOrigin;      /**< Origin point */

  protected:
    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"headerHeight", m_nHeaderHeight, FALSE)
        ATTR_LAYOUTSIZE(L"itemHeight", m_nItemHeight, FALSE)
        ATTR_BOOL(L"checkBox", m_bCheckBox, TRUE)
        ATTR_BOOL(L"multiSel", m_bMultiSelection, TRUE)
        ATTR_SKIN(L"itemSkin", m_pItemSkin, TRUE)
        ATTR_SKIN(L"iconSkin", m_pIconSkin, TRUE)
        ATTR_SKIN(L"checkSkin", m_pCheckSkin, TRUE)
        ATTR_COLOR(L"colorItemBkgnd", m_crItemBg, FALSE)
        ATTR_COLOR(L"colorItemBkgnd2", m_crItemBg2, FALSE)
        ATTR_COLOR(L"colorItemHotBkgnd", m_crItemHotBg, FALSE)
        ATTR_COLOR(L"colorItemSelBkgnd", m_crItemSelBg, FALSE)
        ATTR_COLOR(L"colorText", m_crText, FALSE)
        ATTR_COLOR(L"colorSelText", m_crSelText, FALSE)
        ATTR_INT(L"icon-x", m_ptIcon.x, FALSE)
        ATTR_INT(L"icon-y", m_ptIcon.y, FALSE)
        ATTR_INT(L"text-x", m_ptText.x, FALSE)
        ATTR_INT(L"text-y", m_ptText.y, FALSE)
        ATTR_INT(L"hotTrack", m_bHotTrack, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
        MSG_WM_RBUTTONUP(OnRButtonUp)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()
};

SNSEND

#endif /**< __SLISTCTRL__H__ */
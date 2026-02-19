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
     * @param enable Enable flag
     */
    VOID EnableMultiSelection(BOOL enable)
    {
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
    virtual BOOL CreateChildren(SXmlNode xmlNode);

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
     */
    void NotifySelChange(int nOldSel, int nNewSel, BOOL checkBox = FALSE);

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
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);

    /**
     * @brief Handle left mouse button down event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief Handle left mouse button double-click event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonDbClick(UINT nFlags, CPoint pt);

    /**
     * @brief Handle left mouse button up event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse move event
     * @param nFlags Flags
     * @param pt Mouse coordinates
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * @brief Handle mouse leave event
     */
    void OnMouseLeave();

    /**
     * @brief Handle size change event
     * @param nType Size change type
     * @param size New size
     */
    void OnSize(UINT nType, CSize size);

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

    int m_nSelectItem;   /**< Index of the selected item */
    int m_nSelectColumn; /**< Index of the selected column */
    int m_nHoverItem;    /**< Index of the item under the mouse */
    BOOL m_bHotTrack;    /**< Hot tracking flag */

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
        ATTR_BOOL(L"multiSelection", m_bMultiSelection, TRUE)
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
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
    SOUI_MSG_MAP_END()
};

SNSEND

#endif // __SLISTCTRL__H__
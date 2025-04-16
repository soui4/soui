#ifndef __SLISTBOX__H__
#define __SLISTBOX__H__

#include <core/SPanel.h>
#include <proxy/SPanelProxy.h>
#include <interface/SCtrl-i.h>

SNSBEGIN

/**
 * @struct tagLBITEM
 * @brief List Box Item Structure
 * @details Structure representing an item in the list box.
 */
typedef struct tagLBITEM
{
    STrText strText; /**< Text of the item */
    int nImage;      /**< Icon index */
    LPARAM lParam;   /**< Additional parameter */

    /**
     * @brief Constructor
     * @param pOwner Owner window pointer
     */
    tagLBITEM(SWindow *pOwner)
        : strText(pOwner)
    {
        nImage = -1;
        lParam = 0;
    }
} LBITEM, *LPLBITEM;

/**
 * @class SListBox
 * @brief List Box Control
 * @details A control that displays a list of items from which the user can select one or more.
 */
class SOUI_EXP SListBox : public TPanelProxy<IListBox> {
  public:
    DEF_SOBJECT(SPanel, L"listbox")

    /**
     * @brief Constructor
     */
    SListBox();

    /**
     * @brief Destructor
     */
    virtual ~SListBox();

  public:
    /**
     * @brief Get the number of items in the list box
     * @return Number of items
     */
    STDMETHOD_(int, GetCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Get the index of the currently selected item
     * @return Index of the selected item
     */
    STDMETHOD_(int, GetCurSel)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the currently selected item
     * @param nIndex Index of the item to select
     * @param bNotifyChange Whether to notify of the change
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int nIndex, BOOL bNotifyChange = FALSE) OVERRIDE;

    /**
     * @brief Get the index of the top visible item
     * @return Index of the top visible item
     */
    STDMETHOD_(int, GetTopIndex)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the index of the top visible item
     * @param nIndex Index of the top visible item
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetTopIndex)(THIS_ int nIndex) OVERRIDE;

    /**
     * @brief Get the height of the items
     * @return Height of the items
     */
    STDMETHOD_(int, GetItemHeight)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the height of the items
     * @param nItemHeight Height of the items
     */
    STDMETHOD_(void, SetItemHeight)(THIS_ int nItemHeight) OVERRIDE;

    /**
     * @brief Get the data associated with an item
     * @param nIndex Index of the item
     * @return Data associated with the item
     */
    STDMETHOD_(LPARAM, GetItemData)(THIS_ int nIndex) SCONST OVERRIDE;

    /**
     * @brief Set the data associated with an item
     * @param nIndex Index of the item
     * @param lParam Data to associate with the item
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ int nIndex, LPARAM lParam) OVERRIDE;

    /**
     * @brief Get the text of an item
     * @param nIndex Index of the item
     * @param bRawText Whether to get raw text
     * @param str Output string for the item text
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, GetIText)(THIS_ int nIndex, BOOL bRawText, IStringT *str) SCONST OVERRIDE;

    /**
     * @brief Delete all items in the list box
     */
    STDMETHOD_(void, DeleteAll)(THIS) OVERRIDE;

    /**
     * @brief Delete a specific item
     * @param nIndex Index of the item to delete
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, DeleteString)(THIS_ int nIndex) OVERRIDE;

    /**
     * @brief Add a string to the list box
     * @param lpszItem Text of the item
     * @param nImage Icon index
     * @param lParam Additional parameter
     * @return Index of the added item
     */
    STDMETHOD_(int, AddString)(THIS_ LPCTSTR lpszItem, int nImage = -1, LPARAM lParam = 0) OVERRIDE;

    /**
     * @brief Insert a string into the list box
     * @param nIndex Index at which to insert the item
     * @param lpszItem Text of the item
     * @param nImage Icon index
     * @param lParam Additional parameter
     * @return Index of the inserted item
     */
    STDMETHOD_(int, InsertString)
    (THIS_ int nIndex, LPCTSTR lpszItem, int nImage = -1, LPARAM lParam = 0) OVERRIDE;

    /**
     * @brief Ensure an item is visible
     * @param nIndex Index of the item to ensure visible
     */
    STDMETHOD_(void, EnsureVisible)(THIS_ int nIndex) OVERRIDE;

    /**
     * @brief Find a string in the list box
     * @param iFindAfter Index after which to start searching
     * @param pszText String to find
     * @return Index of the found item or -1 if not found
     */
    STDMETHOD_(int, FindString)(THIS_ int iFindAfter, LPCTSTR pszText) SCONST OVERRIDE;

    /**
     * @brief Get the desired size of the control
     * @param psz Output size
     * @param nParentWid Parent container width
     * @param nParentHei Parent container height
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int nParentWid, int nParentHei) OVERRIDE;

    /**
     * @brief Set the image of an item
     * @param nIndex Index of the item
     * @param iImage Icon index
     * @return TRUE if successful, FALSE otherwise
     */
    STDMETHOD_(BOOL, SetItemImage)(THIS_ int nIndex, int iImage) OVERRIDE;

    /**
     * @brief Get the image of an item
     * @param nIndex Index of the item
     * @return Icon index
     */
    STDMETHOD_(int, GetItemImage)(THIS_ int nIndex) OVERRIDE;

  public:
    /**
     * @brief Get the text of an item
     * @param nIndex Index of the item
     * @param bRawText Whether to get raw text
     * @return Text of the item
     */
    SStringT GetText(int nIndex, BOOL bRawText = FALSE) const
    {
        SStringT strRet;
        GetIText(nIndex, bRawText, &strRet);
        return strRet;
    }

  protected:
    /**
     * @brief Redraw a specific item
     * @param iItem Index of the item to redraw
     */
    void RedrawItem(int iItem);

    /**
     * @brief Hit test to determine the item under the mouse
     * @param pt Mouse coordinates
     * @return Index of the item or -1 if no item
     */
    int HitTest(CPoint &pt);

    /**
     * @brief Update the scroll bar
     */
    void UpdateScrollBar();

    /**
     * @brief Handle language change event
     * @return HRESULT
     */
    virtual HRESULT OnLanguageChanged();

    /**
     * @brief Handle scale change event
     * @param nScale Scale factor
     */
    void OnScaleChanged(int nScale) override;

    /**
     * @brief Create child items from XML configuration
     * @param xmlNode XML node for the child items
     * @return TRUE if successful, FALSE otherwise
     */
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    /**
     * @brief Load attributes for an item from XML
     * @param xmlNode XML node for the item
     * @param pItem Pointer to the item structure
     */
    void LoadItemAttribute(SXmlNode xmlNode, LPLBITEM pItem);

    /**
     * @brief Insert a new item
     * @param nIndex Index at which to insert the item
     * @param pItem Pointer to the item structure
     * @return Index of the inserted item
     */
    int InsertItem(int nIndex, LPLBITEM pItem);

    /**
     * @brief Draw an item
     * @param pRT Rendering target handle
     * @param rc Rectangle for the item
     * @param iItem Index of the item
     */
    virtual void DrawItem(IRenderTarget *pRT, CRect &rc, int iItem);

    /**
     * @brief Notify of selection change
     * @param nOldSel Old selected index
     * @param nNewSel New selected index
     */
    void NotifySelChange(int nOldSel, int nNewSel);

    /**
     * @brief Get the dialog code
     * @return Dialog code
     */
    virtual UINT WINAPI OnGetDlgCode() const;

  protected:
    /**
     * @brief Handle size change event
     * @param nType Size change type
     * @param size New size
     */
    void OnSize(UINT nType, CSize size);

    /**
     * @brief Paint the control
     * @param pRT Rendering target handle
     */
    void OnPaint(IRenderTarget *pRT);

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
     * @brief Handle key down event
     * @param nChar Key code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle character input event
     * @param nChar Character code
     * @param nRepCnt Repeat count
     * @param nFlags Flags
     */
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * @brief Handle destroy event
     */
    void OnDestroy();

    /**
     * @brief Handle show window event
     * @param bShow Show flag
     * @param nStatus Status code
     */
    void OnShowWindow(BOOL bShow, UINT nStatus);

    /**
     * @brief Handle mouse leave event
     */
    void OnMouseLeave();

  protected:
    SArray<LPLBITEM> m_arrItems; /**< Array of items */

    SLayoutSize m_itemHeight; /**< Height of the items */
    int m_iSelItem;           /**< Index of the selected item */
    int m_iHoverItem;         /**< Index of the item under the mouse */
    int m_iScrollSpeed;       /**< Scroll speed */
    BOOL m_bHotTrack;         /**< Hot tracking flag */

    SLayoutSize m_ptIcon[2]; /**< Icon coordinates */
    SLayoutSize m_ptText[2]; /**< Text coordinates */

    COLORREF m_crItemBg;    /**< Background color */
    COLORREF m_crItemBg2;   /**< Background color for even rows */
    COLORREF m_crItemSelBg; /**< Selected item background color */
    COLORREF m_crItemHotBg; /**< Hot item background color */
    COLORREF m_crText;      /**< Text color */
    COLORREF m_crSelText;   /**< Selected text color */
    SAutoRefPtr<ISkinObj> m_pItemSkin, m_pIconSkin;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"itemHeight", m_itemHeight, FALSE)
        ATTR_SKIN(L"itemSkin", m_pItemSkin, TRUE)
        ATTR_SKIN(L"iconSkin", m_pIconSkin, TRUE)
        ATTR_COLOR(L"colorItemBkgnd", m_crItemBg, FALSE)
        ATTR_COLOR(L"colorItemBkgnd2", m_crItemBg2, FALSE)
        ATTR_COLOR(L"colorItemSelBkgnd", m_crItemSelBg, FALSE)
        ATTR_COLOR(L"colorItemHotBkgnd", m_crItemHotBg, FALSE)
        ATTR_COLOR(L"colorText", m_crText, FALSE)
        ATTR_COLOR(L"colorSelText", m_crSelText, FALSE)
        ATTR_LAYOUTSIZE(L"icon-x", m_ptIcon[0], FALSE)
        ATTR_LAYOUTSIZE(L"icon-y", m_ptIcon[1], FALSE)
        ATTR_LAYOUTSIZE(L"text-x", m_ptText[0], FALSE)
        ATTR_LAYOUTSIZE(L"text-y", m_ptText[1], FALSE)
        ATTR_INT(L"hotTrack", m_bHotTrack, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
    SOUI_MSG_MAP_END()
};

SNSEND

#endif // __SLISTBOX__H__
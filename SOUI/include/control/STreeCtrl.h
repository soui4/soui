/**
 * @file       STreeCtrl.h
 * @brief      Definition of the STreeCtrl class.
 * @version    2012.12.16 - 1.1
 * @author     soui
 * @date       2012-12-16
 *
 * @copyright  Copyright (C) 2012-2050 SOUI团队
 *             All rights reserved.
 */

#ifndef __STREECTRL__H__
#define __STREECTRL__H__

#include "core/SPanel.h"
#include "STree.h"

SNSBEGIN

/**
 * @enum STVIMask
 * @brief Masks for TVITEM structure.
 */
enum STVIMask
{
    STVIMask_Toggle = 0x00000001UL,   /**< Mask for toggle state. */
    STVIMask_CheckBox = 0x00000002UL, /**< Mask for checkbox state. */
    STVIMask_Icon = 0x00000004UL      /**< Mask for icon state. */
};

/**
 * @enum STVICheckBox
 * @brief States for checkboxes in the tree view.
 */
enum STVICheckBox
{
    STVICheckBox_UnChecked = 0, /**< Checkbox is unchecked. */
    STVICheckBox_Checked,       /**< Checkbox is checked. */
    STVICheckBox_PartChecked    /**< Checkbox is partially checked. */
};

/**
 * @enum STVIBtn
 * @brief Types of buttons in the tree view.
 */
enum STVIBtn
{
    STVIBtn_None,    /**< No button. */
    STVIBtn_Toggle,  /**< Toggle button. */
    STVIBtn_CheckBox /**< Checkbox button. */
};

/**
 * @struct TVITEM
 * @brief Structure representing a tree view item.
 */
typedef struct tagTVITEM
{
    SStringT strText;   /**< Text of the item. */
    int nImage;         /**< Index of the normal image. */
    int nSelectedImage; /**< Index of the selected image. */
    LPARAM lParam;      /**< Application-defined data. */

    HSTREEITEM hItem;      /**< Handle to the item. */
    BOOL bCollapsed;       /**< TRUE if the item is collapsed. */
    BOOL bVisible;         /**< TRUE if the item is visible. */
    BOOL bHasChildren;     /**< TRUE if the item has children. */
    int nLevel;            /**< Level of the item in the tree. */
    int nCheckBoxValue;    /**< State of the checkbox. */
    int nContentWidth;     /**< Content width of the item. */
    DWORD dwToggleState;   /**< Toggle state of the item. */
    DWORD dwCheckBoxState; /**< Checkbox state of the item. */

    /**
     * @brief Constructor for TVITEM.
     */
    tagTVITEM()
    {
        nImage = -1;
        nSelectedImage = -1;
        lParam = 0;

        hItem = 0;
        bCollapsed = FALSE;
        bVisible = TRUE;
        bHasChildren = FALSE;
        nLevel = 0;
        nCheckBoxValue = STVICheckBox_UnChecked;
        nContentWidth = 0;
        dwToggleState = WndState_Normal;
        dwCheckBoxState = WndState_Normal;
    }

} TVITEM, *LPTVITEM;

/**
 * @class STreeCtrl
 * @brief A tree control for displaying hierarchical data.
 *
 * This class represents a tree control that can display hierarchical data with various customization options.
 */
class SOUI_EXP STreeCtrl
    : public TPanelProxy<ITreeCtrl>
    , protected CSTree<LPTVITEM> {
    DEF_SOBJECT(SPanel, L"treectrl")

  public:
    /**
     * @class IListener
     * @brief Listener interface for tree view events.
     */
    struct IListener
    {
        /**
         * @brief Called when an item is inserted.
         * @param pTreeCtrl Pointer to the tree control.
         * @param hItem Handle to the inserted item.
         */
        virtual void OnInsertItem(STreeCtrl *pTreeCtrl, HSTREEITEM hItem) = 0;

        /**
         * @brief Called when an item is deleted.
         * @param pTreeCtrl Pointer to the tree control.
         * @param hItem Handle to the deleted item.
         * @param lParam Application-defined data.
         */
        virtual void OnDeleteItem(STreeCtrl *pTreeCtrl, HSTREEITEM hItem, LPARAM lParam) = 0;
    };

    /**
     * @brief Constructor for STreeCtrl.
     */
    STreeCtrl();

    /**
     * @brief Destructor for STreeCtrl.
     */
    virtual ~STreeCtrl();

  public:
    /**
     * @brief Inserts a new item into the tree.
     * @param lpszItem Text of the item.
     * @param nImage Index of the normal image.
     * @param nSelectedImage Index of the selected image.
     * @param lParam Application-defined data.
     * @param hParent Handle to the parent item.
     * @param hInsertAfter Handle to the item after which to insert.
     * @return Handle to the inserted item.
     */
    STDMETHOD_(HSTREEITEM, InsertItem)
    (THIS_ LPCTSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST) OVERRIDE;

    /**
     * @brief Inserts a new item into the tree (ANSI version).
     * @param lpszItem Text of the item.
     * @param nImage Index of the normal image.
     * @param nSelectedImage Index of the selected image.
     * @param lParam Application-defined data.
     * @param hParent Handle to the parent item.
     * @param hInsertAfter Handle to the item after which to insert.
     * @return Handle to the inserted item.
     */
    STDMETHOD_(HSTREEITEM, InsertItemA)
    (THIS_ LPCSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST) OVERRIDE
    {
        SStringT str = S_CA2T(lpszItem, CP_UTF8);
        return InsertItem(str, nImage, nSelectedImage, lParam, hParent, hInsertAfter);
    }

    /**
     * @brief Removes an item from the tree.
     * @param hItem Handle to the item to remove.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, RemoveItem)(THIS_ HSTREEITEM hItem) OVERRIDE;

    /**
     * @brief Removes all items from the tree.
     */
    STDMETHOD_(void, RemoveAllItems)(THIS) OVERRIDE;

    /**
     * @brief Gets the root item of the tree.
     * @return Handle to the root item.
     */
    STDMETHOD_(HSTREEITEM, GetRootItem)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the next sibling item.
     * @param hItem Handle to the item.
     * @return Handle to the next sibling item.
     */
    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Gets the previous sibling item.
     * @param hItem Handle to the item.
     * @return Handle to the previous sibling item.
     */
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Gets a child item.
     * @param hItem Handle to the item.
     * @param bFirst TRUE to get the first child, FALSE to get the next child.
     * @return Handle to the child item.
     */
    STDMETHOD_(HSTREEITEM, GetChildItem)
    (THIS_ HSTREEITEM hItem, BOOL bFirst = TRUE) SCONST OVERRIDE;

    /**
     * @brief Gets the parent item.
     * @param hItem Handle to the item.
     * @return Handle to the parent item.
     */
    STDMETHOD_(HSTREEITEM, GetParentItem)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Gets the selected item.
     * @return Handle to the selected item.
     */
    STDMETHOD_(HSTREEITEM, GetSelectedItem)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the next item.
     * @param hItem Handle to the item.
     * @return Handle to the next item.
     */
    STDMETHOD_(HSTREEITEM, GetNextItem)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Sorts the children of an item.
     * @param hItem Handle to the item.
     * @param sortFunc Sorting callback function.
     * @param pCtx Context for the sorting function.
     */
    STDMETHOD_(void, SortChildren)
    (THIS_ HSTREEITEM hItem, FunTreeSortCallback sortFunc, void *pCtx) OVERRIDE;

    /**
     * @brief Selects an item.
     * @param hItem Handle to the item to select.
     * @param bEnsureVisible TRUE to ensure the item is visible.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SelectItem)(THIS_ HSTREEITEM hItem, BOOL bEnsureVisible = TRUE) OVERRIDE;

    /**
     * @brief Gets the text of an item.
     * @param hItem Handle to the item.
     * @param strText String to receive the item text.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, GetItemText)(THIS_ HSTREEITEM hItem, IStringT *strText) SCONST OVERRIDE;

    /**
     * @brief Gets the text of an item (ANSI version).
     * @param hItem Handle to the item.
     * @param strText String to receive the item text.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, GetItemTextA)(THIS_ HSTREEITEM hItem, IStringA *strText) SCONST OVERRIDE
    {
        SStringT strBuf;
        BOOL bRet = GetItemText(hItem, &strBuf);
        SStringA strBufA = S_CT2A(strBuf, CP_UTF8);
        strText->Copy(&strBufA);
        return bRet;
    }

    /**
     * @brief Sets the text of an item.
     * @param hItem Handle to the item.
     * @param lpszItem New text for the item.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetItemText)(THIS_ HSTREEITEM hItem, LPCTSTR lpszItem) OVERRIDE;

    /**
     * @brief Sets the text of an item (ANSI version).
     * @param hItem Handle to the item.
     * @param lpszItem New text for the item.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetItemTextA)(THIS_ HSTREEITEM hItem, LPCSTR lpszItem) OVERRIDE
    {
        SStringT str = S_CA2T(lpszItem, CP_UTF8);
        return SetItemText(hItem, str);
    }

    /**
     * @brief Gets the images of an item.
     * @param hItem Handle to the item.
     * @param nImage Pointer to receive the index of the normal image.
     * @param nSelectedImage Pointer to receive the index of the selected image.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, GetItemImage)
    (THIS_ HSTREEITEM hItem, int *nImage, int *nSelectedImage) SCONST OVERRIDE;

    /**
     * @brief Sets the images of an item.
     * @param hItem Handle to the item.
     * @param nImage Index of the normal image.
     * @param nSelectedImage Index of the selected image.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetItemImage)(THIS_ HSTREEITEM hItem, int nImage, int nSelectedImage) OVERRIDE;

    /**
     * @brief Gets the application-defined data of an item.
     * @param hItem Handle to the item.
     * @return Application-defined data.
     */
    STDMETHOD_(LPARAM, GetItemData)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Sets the application-defined data of an item.
     * @param hItem Handle to the item.
     * @param lParam Application-defined data.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetItemData)(THIS_ HSTREEITEM hItem, LPARAM lParam) OVERRIDE;

    /**
     * @brief Checks if an item has children.
     * @param hItem Handle to the item.
     * @return TRUE if the item has children, otherwise FALSE.
     */
    STDMETHOD_(BOOL, ItemHasChildren)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Gets the check state of an item.
     * @param hItem Handle to the item.
     * @return Check state of the item.
     */
    STDMETHOD_(int, GetCheckState)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    /**
     * @brief Sets the check state of an item.
     * @param hItem Handle to the item.
     * @param bCheck TRUE to check the item, FALSE to uncheck.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, SetCheckState)(THIS_ HSTREEITEM hItem, BOOL bCheck) OVERRIDE;

    /**
     * @brief Expands or collapses an item.
     * @param hItem Handle to the item.
     * @param nCode Code specifying the action (e.g., TVE_EXPAND).
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, Expand)(THIS_ HSTREEITEM hItem, UINT nCode = TVE_EXPAND) OVERRIDE;

    /**
     * @brief Ensures an item is visible.
     * @param hItem Handle to the item.
     * @return TRUE if successful, otherwise FALSE.
     */
    STDMETHOD_(BOOL, EnsureVisible)(THIS_ HSTREEITEM hItem) OVERRIDE;

  public:
    /**
     * @brief Sets the listener for tree view events.
     * @param pListener Pointer to the listener.
     */
    void SetListener(IListener *pListener);

    /**
     * @brief Inserts a new item into the tree.
     * @param lpszItem Text of the item.
     * @param hParent Handle to the parent item.
     * @param hInsertAfter Handle to the item after which to insert.
     * @return Handle to the inserted item.
     */
    HSTREEITEM InsertItem(LPCTSTR lpszItem, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST);

    /**
     * @brief Inserts a new item into the tree.
     * @param lpszItem Text of the item.
     * @param nImage Index of the normal image.
     * @param nSelectedImage Index of the selected image.
     * @param hParent Handle to the parent item.
     * @param hInsertAfter Handle to the item after which to insert.
     * @return Handle to the inserted item.
     */
    HSTREEITEM InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST);

    /**
     * @brief Performs a hit test on the tree view.
     * @param pt Mouse position.
     * @return Handle to the item at the specified position.
     */
    HSTREEITEM HitTest(CPoint &pt);

    /**
     * @brief Gets the text of an item.
     * @param hItem Handle to the item.
     * @param strText String to receive the item text.
     * @return TRUE if successful, otherwise FALSE.
     */
    BOOL GetItemText(HSTREEITEM hItem, SStringT &strText) const
    {
        return GetItemText(hItem, &strText);
    }

  protected:
    void CalcItemWidth(IRenderTarget *pRT, HSTREEITEM hItem, DWORD dwFlags);
    void RecalcItemsWidth();
    /**
     * @brief Handles the page up action.
     */
    void PageUp();

    /**
     * @brief Handles the page down action.
     */
    void PageDown();

    /**
     * @brief Updates the scroll bars.
     */
    void UpdateScrollBar();

    /**
     * @brief Creates child windows from an XML node.
     * @param xmlNode XML node containing the child window definitions.
     * @return TRUE if successful, otherwise FALSE.
     */
    BOOL CreateChildren(SXmlNode xmlNode) override;

    void OnScaleChanged(int scale) override;

    /**
     * @brief Loads a branch of items from an XML node.
     * @param hParent Handle to the parent item.
     * @param xmlNode XML node containing the branch definitions.
     */
    virtual void LoadBranch(HSTREEITEM hParent, SXmlNode xmlNode);

    /**
     * @brief Loads attributes for an item from an XML node.
     * @param xmlNode XML node containing the item attributes.
     * @param pItem Pointer to the item.
     */
    virtual void LoadItemAttribute(SXmlNode xmlNode, LPTVITEM pItem);

    /**
     * @brief Inserts a new item into the tree.
     * @param pItemObj Pointer to the item data.
     * @param hParent Handle to the parent item.
     * @param hInsertAfter Handle to the item after which to insert.
     * @return Handle to the inserted item.
     */
    HSTREEITEM InsertItem(LPTVITEM pItemObj, HSTREEITEM hParent, HSTREEITEM hInsertAfter);

    /**
     * @brief Inserts a new item into the tree from an XML node.
     * @param xmlNode XML node containing the item definition.
     * @param hParent Handle to the parent item.
     * @param hInsertAfter Handle to the item after which to insert.
     * @return Handle to the inserted item.
     */
    HSTREEITEM InsertItem(SXmlNode xmlNode, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST);

    /**
     * @brief Checks if one item is an ancestor of another.
     * @param hItem1 Handle to the potential ancestor item.
     * @param hItem2 Handle to the potential descendant item.
     * @return TRUE if hItem1 is an ancestor of hItem2, otherwise FALSE.
     */
    BOOL IsAncestor(HSTREEITEM hItem1, HSTREEITEM hItem2);

    /**
     * @brief Verifies if an item is valid.
     * @param hItem Handle to the item to verify.
     * @return TRUE if the item is valid, otherwise FALSE.
     */
    BOOL VerifyItem(HSTREEITEM hItem) const;

    /**
     * @brief Sets the visibility of an item's children.
     * @param hItem Handle to the item.
     * @param bVisible TRUE to make the children visible, FALSE to hide them.
     */
    void SetChildrenVisible(HSTREEITEM hItem, BOOL bVisible);

    /**
     * @brief Sets the check state of an item's children.
     * @param hItem Handle to the item.
     * @param nCheckValue New check state value.
     */
    void SetChildrenState(HSTREEITEM hItem, int nCheckValue);

    /**
     * @brief Checks the state of an item's children.
     * @param hItem Handle to the item.
     * @param bCheck TRUE to check the children, FALSE to uncheck them.
     * @return TRUE if the state was successfully updated, otherwise FALSE.
     */
    BOOL CheckChildrenState(HSTREEITEM hItem, BOOL bCheck);

    /**
     * @brief Updates the check state of an item.
     * @param hItem Handle to the item.
     */
    void CheckState(HSTREEITEM hItem);

    /**
     * @brief Performs layout calculations for items.
     */
    virtual void ItemLayout();

    /**
     * @brief Calculates the content width of an item.
     * @param pItem Pointer to the item data.
     */
    virtual void CalcItemContentWidth(LPTVITEM pItem);

    /**
     * @brief Calculates the width of an item.
     * @param pItem Pointer to the item data.
     * @return Calculated width of the item.
     */
    virtual int CalcItemWidth(const LPTVITEM pItem);

    /**
     * @brief Calculates the maximum width of an item and its children.
     * @param hItem Handle to the item.
     * @return Maximum width of the item and its children.
     */
    virtual int CalcMaxItemWidth(HSTREEITEM hItem);

    /**
     * @brief Updates the content width of the tree.
     */
    virtual void UpdateContentWidth();

    /**
     * @brief Gets the index of an item in the visible list.
     * @param hItemObj Handle to the item.
     * @return Index of the item in the visible list.
     */
    int GetItemShowIndex(HSTREEITEM hItemObj);

    /**
     * @brief Gets the rectangle of an item.
     * @param pItem Pointer to the item data.
     * @param rcItem Rectangle to receive the item position.
     * @return TRUE if successful, otherwise FALSE.
     */
    BOOL GetItemRect(LPTVITEM pItem, CRect &rcItem);

    /**
     * @brief Redraws an item.
     * @param hItem Handle to the item.
     */
    void RedrawItem(HSTREEITEM hItem);

    /**
     * @brief Draws an item.
     * @param pRT Pointer to the rendering target.
     * @param rc Rectangle to draw the item.
     * @param hItem Handle to the item.
     */
    virtual void DrawItem(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem);

    /**
     * @brief Draws lines for an item.
     * @param pRT Pointer to the rendering target.
     * @param rc Rectangle to draw the lines.
     * @param hItem Handle to the item.
     */
    virtual void DrawLines(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem);

    /**
     * @brief Performs a hit test on an item.
     * @param hItem Handle to the item.
     * @param pt Mouse position.
     * @return Index of the hit test result.
     */
    int ItemHitTest(HSTREEITEM hItem, CPoint &pt) const;

    /**
     * @brief Modifies the toggle state of an item.
     * @param hItem Handle to the item.
     * @param dwStateAdd State to add.
     * @param dwStateRemove State to remove.
     */
    void ModifyToggleState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove);

    /**
     * @brief Modifies the checkbox state of an item.
     * @param hItem Handle to the item.
     * @param dwStateAdd State to add.
     * @param dwStateRemove State to remove.
     */
    void ModifyChekcBoxState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove);

    /**
     * @brief Handles the left mouse button down event for an item.
     * @param hItem Handle to the item.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void ItemLButtonDown(HSTREEITEM hItem, UINT nFlags, CPoint pt);

    /**
     * @brief Handles the left mouse button up event for an item.
     * @param hItem Handle to the item.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void ItemLButtonUp(HSTREEITEM hItem, UINT nFlags, CPoint pt);

    /**
     * @brief Handles the left mouse button double-click event for an item.
     * @param hItem Handle to the item.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void ItemLButtonDbClick(HSTREEITEM hItem, UINT nFlags, CPoint pt);

    /**
     * @brief Handles the mouse move event for an item.
     * @param hItem Handle to the item.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void ItemMouseMove(HSTREEITEM hItem, UINT nFlags, CPoint pt);

    /**
     * @brief Handles the mouse leave event for an item.
     * @param hItem Handle to the item.
     */
    void ItemMouseLeave(HSTREEITEM hItem);

  protected:
    /**
     * @brief Frees the memory associated with an item.
     * @param pItemData Pointer to the item data.
     */
    virtual void OnNodeFree(LPTVITEM &pItemData);

    /**
     * @brief Handles the insertion of an item.
     * @param pItemData Pointer to the item data.
     */
    virtual void OnInsertItem(LPTVITEM &pItemData);

  protected:
    /**
     * @brief Handle to the selected item.
     */
    HSTREEITEM m_hSelItem;

    /**
     * @brief Handle to the item under the hover state.
     */
    HSTREEITEM m_hHoverItem;

    /**
     * @brief Handle to the item that has capture.
     */
    HSTREEITEM m_hCaptureItem;

    /**
     * @brief Number of visible items.
     */
    int m_nVisibleItems;

    /**
     * @brief Total content width of the tree.
     */
    int m_nContentWidth;

    /**
     * @brief Mask for item attributes.
     */
    UINT m_uItemMask;

    /**
     * @brief Offset for item positioning.
     */
    int m_nItemOffset;

    /**
     * @brief Rectangle for the toggle button.
     */
    CRect m_rcToggle;

    /**
     * @brief Rectangle for the checkbox.
     */
    CRect m_rcCheckBox;

    /**
     * @brief Rectangle for the icon.
     */
    CRect m_rcIcon;

    /**
     * @brief Button currently hovered.
     */
    int m_nItemHoverBtn;

    /**
     * @brief Button currently pushed down.
     */
    int m_nItemPushDownBtn;

    /**
     * @brief Height of each item.
     */
    SLayoutSize m_nItemHei;

    /**
     * @brief Indentation between levels.
     */
    SLayoutSize m_nIndent;

    /**
     * @brief Margin around each item.
     */
    SLayoutSize m_nItemMargin;

    /**
     * @brief Flag indicating if checkboxes are enabled.
     */
    BOOL m_bCheckBox;

    /**
     * @brief Flag indicating if right-click selection is enabled.
     */
    BOOL m_bRightClickSel;

    /**
     * @brief Listener for tree view events.
     */
    IListener *m_pListener;

    /**
     * @brief Skin for the background of items.
     */
    SAutoRefPtr<ISkinObj> m_pItemBgSkin;

    /**
     * @brief Skin for the selected background of items.
     */
    SAutoRefPtr<ISkinObj> m_pItemSelSkin;

    /**
     * @brief Skin for the icons.
     */
    SAutoRefPtr<ISkinObj> m_pIconSkin;

    /**
     * @brief Skin for the toggle buttons.
     */
    SAutoRefPtr<ISkinObj> m_pToggleSkin;

    /**
     * @brief Skin for the checkboxes.
     */
    SAutoRefPtr<ISkinObj> m_pCheckSkin;

    /**
     * @brief Skin for the lines.
     */
    SAutoRefPtr<ISkinObj> m_pLineSkin;

    /**
     * @brief Background color of items.
     */
    COLORREF m_crItemBg;

    /**
     * @brief Background color of selected items.
     */
    COLORREF m_crItemSelBg;

    /**
     * @brief Text color of items.
     */
    COLORREF m_crItemText;

    /**
     * @brief Text color of selected items.
     */
    COLORREF m_crItemSelText;

    /**
     * @brief Flag indicating if lines are drawn between items.
     */
    BOOL m_bHasLines; /**< has lines*/

    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"indent", m_nIndent, TRUE)
        ATTR_LAYOUTSIZE(L"itemHeight", m_nItemHei, TRUE)
        ATTR_LAYOUTSIZE(L"itemMargin", m_nItemMargin, TRUE)
        ATTR_BOOL(L"checkBox", m_bCheckBox, TRUE)
        ATTR_BOOL(L"rightClickSel", m_bRightClickSel, TRUE)
        ATTR_SKIN(L"itemBkgndSkin", m_pItemBgSkin, TRUE)
        ATTR_SKIN(L"itemSelSkin", m_pItemSelSkin, TRUE)
        ATTR_SKIN(L"lineSkin", m_pLineSkin, TRUE)
        ATTR_SKIN(L"toggleSkin", m_pToggleSkin, TRUE)
        ATTR_SKIN(L"iconSkin", m_pIconSkin, TRUE)
        ATTR_SKIN(L"checkSkin", m_pCheckSkin, TRUE)
        ATTR_COLOR(L"colorItemBkgnd", m_crItemBg, FALSE)
        ATTR_COLOR(L"colorItemSelBkgnd", m_crItemSelBg, FALSE)
        ATTR_COLOR(L"colorItemText", m_crItemText, FALSE)
        ATTR_COLOR(L"colorItemSelText", m_crItemSelText, FALSE)
        ATTR_BOOL(L"hasLines", m_bHasLines, TRUE)
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief Handles the destruction of the tree control.
     */
    void OnDestroy();

    /**
     * @brief Handles the paint event.
     * @param pRT Pointer to the rendering target.
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * @brief Handles the left mouse button down event.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void OnLButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief Handles the left mouse button up event.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void OnLButtonUp(UINT nFlags, CPoint pt);

    /**
     * @brief Handles the left mouse button double-click event.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void OnLButtonDbClick(UINT nFlags, CPoint pt);

    /**
     * @brief Handles the right mouse button down event.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void OnRButtonDown(UINT nFlags, CPoint pt);

    /**
     * @brief Handles the mouse move event.
     * @param nFlags Flags associated with the mouse event.
     * @param pt Mouse position.
     */
    void OnMouseMove(UINT nFlags, CPoint pt);

    /**
     * @brief Handles the mouse leave event.
     */
    void OnMouseLeave();

    /**
     * @brief Handles the size event.
     * @param nType Type of size change.
     * @param size New size.
     */
    void OnSize(UINT nType, CSize size);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_RBUTTONDOWN(OnRButtonDown);
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()
};

SNSEND

#endif // __STREECTRL__H__
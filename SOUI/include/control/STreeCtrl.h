//////////////////////////////////////////////////////////////////////////
//  Class Name: STreeCtrl
//     Version: 2012.12.16 - 1.1 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "core/SPanel.h"
#include "STree.h"

SNSBEGIN

enum
{
    STVIMask_Toggle = 0x00000001UL,
    STVIMask_CheckBox = 0x00000002UL,
    STVIMask_Icon = 0x00000004UL
};

enum
{
    STVICheckBox_UnChecked = 0, //未选中
    STVICheckBox_Checked,       //选中
    STVICheckBox_PartChecked    //部分选中
};

enum
{
    STVIBtn_None,
    STVIBtn_Toggle,
    STVIBtn_CheckBox
};

typedef struct tagTVITEM
{
    SStringT strText;
    int nImage;
    int nSelectedImage;
    LPARAM lParam;

    HSTREEITEM hItem;
    BOOL bCollapsed;
    BOOL bVisible;
    BOOL bHasChildren;
    int nLevel;
    int nCheckBoxValue;
    int nContentWidth;
    DWORD dwToggleState;
    DWORD dwCheckBoxState;

    tagTVITEM()
    {
        nImage = -1;
        nSelectedImage = -1;
        lParam = NULL;

        hItem = NULL;
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

class SOUI_EXP STreeCtrl
    : public TPanelProxy<ITreeCtrl>
    , protected CSTree<LPTVITEM> {
    DEF_SOBJECT(SPanel, L"treectrl")
  public:
    struct IListener
    {
        virtual void OnInsertItem(STreeCtrl *pTreeCtrl, HSTREEITEM hItem) = 0;
        virtual void OnDeleteItem(STreeCtrl *pTreeCtrl, HSTREEITEM hItem, LPARAM lParam) = 0;
    };

    STreeCtrl();

    virtual ~STreeCtrl();

  public:
    STDMETHOD_(HSTREEITEM, InsertItem)
    (THIS_ LPCTSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST) OVERRIDE;
	STDMETHOD_(HSTREEITEM, InsertItemA)
		(THIS_ LPCSTR lpszItem, int nImage, int nSelectedImage, LPARAM lParam, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST) OVERRIDE{
			SStringT str = S_CA2T(lpszItem,CP_UTF8);
			return InsertItem(str,nImage,nSelectedImage,lParam,hParent,hInsertAfter);
	}

    STDMETHOD_(BOOL, RemoveItem)(THIS_ HSTREEITEM hItem) OVERRIDE;
    STDMETHOD_(void, RemoveAllItems)(THIS) OVERRIDE;

    STDMETHOD_(HSTREEITEM, GetRootItem)(THIS) SCONST OVERRIDE;

    STDMETHOD_(HSTREEITEM, GetNextSiblingItem)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
    STDMETHOD_(HSTREEITEM, GetPrevSiblingItem)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
    STDMETHOD_(HSTREEITEM, GetChildItem)
    (THIS_ HSTREEITEM hItem, BOOL bFirst = TRUE) SCONST OVERRIDE;
    STDMETHOD_(HSTREEITEM, GetParentItem)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
    STDMETHOD_(HSTREEITEM, GetSelectedItem)(THIS) SCONST OVERRIDE;
    STDMETHOD_(HSTREEITEM, GetNextItem)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    STDMETHOD_(void, SortChildren)
    (THIS_ HSTREEITEM hItem, FunTreeSortCallback sortFunc, void *pCtx) OVERRIDE;

    STDMETHOD_(BOOL, SelectItem)(THIS_ HSTREEITEM hItem, BOOL bEnsureVisible = TRUE) OVERRIDE;

    STDMETHOD_(BOOL, GetItemText)(THIS_ HSTREEITEM hItem, IStringT *strText) SCONST OVERRIDE;
	STDMETHOD_(BOOL, GetItemTextA)(THIS_ HSTREEITEM hItem, IStringA *strText) SCONST OVERRIDE{
		SStringT strBuf;
		BOOL bRet = GetItemText(hItem,&strBuf);
		SStringA strBufA = S_CT2A(strBuf,CP_UTF8);
		strText->Copy(&strBufA);
		return bRet;
	}
    STDMETHOD_(BOOL, SetItemText)(THIS_ HSTREEITEM hItem, LPCTSTR lpszItem) OVERRIDE;
	STDMETHOD_(BOOL, SetItemTextA)(THIS_ HSTREEITEM hItem, LPCSTR lpszItem) OVERRIDE
	{
		SStringT str = S_CA2T(lpszItem,CP_UTF8);
		return SetItemText(hItem,str);
	}
    STDMETHOD_(BOOL, GetItemImage)
    (THIS_ HSTREEITEM hItem, int *nImage, int *nSelectedImage) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetItemImage)(THIS_ HSTREEITEM hItem, int nImage, int nSelectedImage) OVERRIDE;
    STDMETHOD_(LPARAM, GetItemData)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetItemData)(THIS_ HSTREEITEM hItem, LPARAM lParam) OVERRIDE;
    STDMETHOD_(BOOL, ItemHasChildren)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;

    STDMETHOD_(int, GetCheckState)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
    STDMETHOD_(BOOL, SetCheckState)(THIS_ HSTREEITEM hItem, BOOL bCheck) OVERRIDE;

    STDMETHOD_(BOOL, Expand)(THIS_ HSTREEITEM hItem, UINT nCode = TVE_EXPAND) OVERRIDE;
    STDMETHOD_(BOOL, EnsureVisible)(THIS_ HSTREEITEM hItem) OVERRIDE;

  public:
    void SetListener(IListener *pListener);

    HSTREEITEM InsertItem(LPCTSTR lpszItem, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST);
    HSTREEITEM InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST);

    HSTREEITEM HitTest(CPoint &pt);

    BOOL GetItemText(HSTREEITEM hItem, SStringT &strText) const
    {
        return GetItemText(hItem, &strText);
    }

  protected:
    void PageUp();
    void PageDown();
    void UpdateScrollBar();

    virtual BOOL CreateChildren(SXmlNode xmlNode);
    virtual void LoadBranch(HSTREEITEM hParent, SXmlNode xmlNode);
    virtual void LoadItemAttribute(SXmlNode xmlNode, LPTVITEM pItem);

    HSTREEITEM InsertItem(LPTVITEM pItemObj, HSTREEITEM hParent, HSTREEITEM hInsertAfter);
    HSTREEITEM InsertItem(SXmlNode xmlNode, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST);

    BOOL IsAncestor(HSTREEITEM hItem1, HSTREEITEM hItem2);
    BOOL VerifyItem(HSTREEITEM hItem) const;

    void SetChildrenVisible(HSTREEITEM hItem, BOOL bVisible);

    void SetChildrenState(HSTREEITEM hItem, int nCheckValue);
    BOOL CheckChildrenState(HSTREEITEM hItem, BOOL bCheck);
    void CheckState(HSTREEITEM hItem);

    virtual void ItemLayout();
    virtual void CalcItemContentWidth(LPTVITEM pItem);
    virtual int CalcItemWidth(const LPTVITEM pItem);
    virtual int CalcMaxItemWidth(HSTREEITEM hItem);

    virtual void UpdateContentWidth();

    int GetItemShowIndex(HSTREEITEM hItemObj);
    BOOL GetItemRect(LPTVITEM pItem, CRect &rcItem);

    void RedrawItem(HSTREEITEM hItem);
    virtual void DrawItem(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem);
    virtual void DrawLines(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem);

    int ItemHitTest(HSTREEITEM hItem, CPoint &pt) const;
    void ModifyToggleState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove);
    void ModifyChekcBoxState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove);

    void ItemLButtonDown(HSTREEITEM hItem, UINT nFlags, CPoint pt);
    void ItemLButtonUp(HSTREEITEM hItem, UINT nFlags, CPoint pt);
    void ItemLButtonDbClick(HSTREEITEM hItem, UINT nFlags, CPoint pt);

    void ItemMouseMove(HSTREEITEM hItem, UINT nFlags, CPoint pt);
    void ItemMouseLeave(HSTREEITEM hItem);

  protected:
    virtual void OnNodeFree(LPTVITEM &pItemData);
    virtual void OnInsertItem(LPTVITEM &pItemData);

  protected:
    HSTREEITEM m_hSelItem;
    HSTREEITEM m_hHoverItem;
    HSTREEITEM m_hCaptureItem;

    int m_nVisibleItems;
    int m_nContentWidth;

    UINT m_uItemMask;
    int m_nItemOffset;
    CRect m_rcToggle;
    CRect m_rcCheckBox;
    CRect m_rcIcon;
    int m_nItemHoverBtn;
    int m_nItemPushDownBtn;

    int m_nItemHei, m_nIndent, m_nItemMargin;
    BOOL m_bCheckBox;
    BOOL m_bRightClickSel;
    IListener *m_pListener;
    SAutoRefPtr<ISkinObj> m_pItemBgSkin, m_pItemSelSkin;
    SAutoRefPtr<ISkinObj> m_pIconSkin, m_pToggleSkin, m_pCheckSkin;
    SAutoRefPtr<ISkinObj> m_pLineSkin;

    COLORREF m_crItemBg, m_crItemSelBg;
    COLORREF m_crItemText, m_crItemSelText;
    BOOL m_bHasLines; /**< has lines*/

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"indent", m_nIndent, TRUE)
        ATTR_INT(L"itemHeight", m_nItemHei, TRUE)
        ATTR_INT(L"itemMargin", m_nItemMargin, TRUE)
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
    void OnDestroy();

    void OnPaint(IRenderTarget *pRT);

    void OnLButtonDown(UINT nFlags, CPoint pt);
    void OnLButtonUp(UINT nFlags, CPoint pt);
    void OnLButtonDbClick(UINT nFlags, CPoint pt);
    void OnRButtonDown(UINT nFlags, CPoint pt);

    void OnMouseMove(UINT nFlags, CPoint pt);
    void OnMouseLeave();
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

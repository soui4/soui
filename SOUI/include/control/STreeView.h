#pragma once

#include "core/Swnd.h"
#include "core/SItemPanel.h"
#include "interface/SAdapter-i.h"
#include "interface/STreeViewItemLocator-i.h"

SNSBEGIN
//////////////////////////////////////////////////////////////////////////
class SOUI_EXP STreeViewItemLocator : public TObjRefImpl<ITreeViewItemLocator> {
  public:
    STreeViewItemLocator(int nIndent = 16);

    ~STreeViewItemLocator();

  public:
    STDMETHOD_(void, SetAdapter)(THIS_ ITvAdapter *pAdapter) OVERRIDE;
    STDMETHOD_(void, OnBranchChanged)(THIS_ HSTREEITEM hItem) OVERRIDE;
    STDMETHOD_(void, OnBranchExpandedChanged)
    (THIS_ HSTREEITEM hItem, BOOL bExpandedOld, BOOL bExpandedNew) OVERRIDE;

    STDMETHOD_(int, GetTotalHeight)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetTotalWidth)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, Item2Position)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
    STDMETHOD_(HSTREEITEM, Position2Item)(THIS_ int position) SCONST OVERRIDE;
    STDMETHOD_(int, GetScrollLineSize)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetItemWidth)(THIS_ HSTREEITEM hItem, int nWidth) OVERRIDE;
    STDMETHOD_(int, GetItemWidth)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
    STDMETHOD_(void, SetItemHeight)(THIS_ HSTREEITEM hItem, int nHeight) OVERRIDE;
    STDMETHOD_(int, GetItemHeight)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
    STDMETHOD_(int, GetItemIndent)(THIS_ HSTREEITEM hItem) SCONST OVERRIDE;
	STDMETHOD_(int, GetIndent)(CTHIS) SCONST OVERRIDE;
	STDMETHOD_(void, SetIndent)(THIS_ int nIndent) OVERRIDE;
  protected:
    BOOL IsItemExpanded(HSTREEITEM hItem) const;

    //更新hItem所在的父窗口中分枝宽度数据
    // hItem:显示宽度发生变化的节点，可以是节点本身宽度变化，也可能是子节点宽度发生了变化
    // nOldWidth：原显示宽度
    // nNewWidth: 新显示宽度
    void _UpdateBranchWidth(HSTREEITEM hItem, int nOldWidth, int nNewWidth);

    int _GetBranchWidth(HSTREEITEM hBranch) const;

    void _SetBranchWidth(HSTREEITEM hBranch, int nWidth);

    void _SetItemWidth(HSTREEITEM hItem, int nWidth);

    int _GetBranchHeight(HSTREEITEM hItem) const;

    void _SetBranchHeight(HSTREEITEM hItem, int nHeight);

    void _UpdateBranchHeight(HSTREEITEM hItem, int nDiff);

    //向后更新兄弟结点的偏移量
    void _UpdateSiblingsOffset(HSTREEITEM hItem);

    int _GetItemOffset(HSTREEITEM hItem) const;

    void _SetItemOffset(HSTREEITEM hItem, int nOffset);

    void _SetItemHeight(HSTREEITEM hItem, int nHeight);

    int _GetItemVisibleHeight(HSTREEITEM hItem) const;

    int _GetItemVisibleWidth(HSTREEITEM hItem) const;

    HSTREEITEM _Position2Item(int position, HSTREEITEM hParent, int nParentPosition) const;

    BOOL _IsItemVisible(HSTREEITEM hItem) const;

    void _InitBranch(HSTREEITEM hItem);

    SAutoRefPtr<ITvAdapter> m_adapter;
    int m_nLineHeight;
    int m_nIndent;
    CSize m_szDef;
};

class SOUI_EXP STreeView
    : public TPanelProxy<ITreeView>
    , protected SHostProxy
    , protected IItemContainer {
    DEF_SOBJECT(SPanel, L"treeview")
    friend class STreeViewDataSetObserver;

  public:
    STreeView();
    ~STreeView();

  public:
    STDMETHOD_(BOOL, SetAdapter)(THIS_ ITvAdapter *adapter) OVERRIDE;

    STDMETHOD_(ITvAdapter *, GetAdapter)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetItemLocator)(THIS_ ITreeViewItemLocator *pItemLocator) OVERRIDE;

    STDMETHOD_(ITreeViewItemLocator *, GetItemLocator)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetSel)(THIS_ HSTREEITEM hItem, BOOL bNotify = FALSE) OVERRIDE;

    STDMETHOD_(HSTREEITEM, GetSel)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, EnsureVisible)(THIS_ HSTREEITEM hItem) OVERRIDE;

    STDMETHOD_(IItemPanel *, HitTest)(THIS_ const POINT *pt) SCONST OVERRIDE;

  public:
    SItemPanel *HitTest(CPoint &pt) const;

  protected:
    BOOL OnItemClick(IEvtArgs *pEvt);
    BOOL OnItemDblClick(IEvtArgs *pEvt);

    void onBranchChanged(HSTREEITEM hBranch);
    void onBranchInvalidated(HSTREEITEM hBranch, BOOL bInvalidParents, BOOL bInvalidChildren);
    void onBranchExpandedChanged(HSTREEITEM hBranch, BOOL bExpandedOld, BOOL bExpandedNew);
	void onItemBeforeRemove(HSTREEITEM hItem);
  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnSize(UINT nType, CSize size);
    void OnDestroy();
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags); //实现键盘选择
    LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OnMouseLeave();
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OnKillFocus(SWND wndFocus);
    void OnSetFocus(SWND wndOld);
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
    HRESULT OnAttrIndent(const SStringW &strValue, BOOL bLoading);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"indent", OnAttrIndent)
        ATTR_INT(L"wantTab", m_bWantTab, FALSE)
		ATTR_BOOL(L"hasLines", m_bHasLines, TRUE)
		ATTR_SKIN(L"lineSkin", m_pLineSkin, TRUE)
    SOUI_ATTRS_END()
  protected:
    virtual UINT WINAPI OnGetDlgCode() const;
    virtual BOOL OnSetCursor(const CPoint &pt);
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);
    virtual void OnColorize(COLORREF cr);
    virtual void OnScaleChanged(int nScale);
    virtual HRESULT OnLanguageChanged();
    virtual void OnRebuildFont();

    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);
    virtual int GetScrollLineSize(BOOL bVertical);
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture);          //设置or释放鼠标捕获
    virtual BOOL OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const; //获得表项的显示位置
    virtual BOOL IsItemRedrawDelay() const;                                  //指示表项的更新方式

	virtual void DrawLines(IRenderTarget *pRT, const CRect &rc, HSTREEITEM hItem);
  protected:
    void UpdateScrollBar();
    void UpdateVisibleItems();

    void RedrawItem(SItemPanel *pItem);
    SItemPanel *GetItemPanel(HSTREEITEM hItem);

    void DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam);

  protected:
    SAutoRefPtr<ITvAdapter> m_adapter;
    SAutoRefPtr<ITvDataSetObserver> m_observer;
    SAutoRefPtr<ITreeViewItemLocator> m_tvItemLocator;

    struct ItemInfo
    {
        SItemPanel *pItem;
        int nType;
    };
    SArray<SList<SItemPanel *> *> m_itemRecycle; // item回收站,每一种样式在回收站中保持一个列表，以便重复利用
    SList<ItemInfo> m_visible_items;             //可见元素

    typedef SMap<HSTREEITEM, ItemInfo> VISIBLEITEMSMAP;
    VISIBLEITEMSMAP *m_pVisibleMap;

    SXmlDoc m_xmlTemplate;

    SOsrPanel *m_itemCapture;
    SOsrPanel *m_pHoverItem;

    HSTREEITEM m_hSelected; /**< 当前选择项 */

    BOOL m_bWantTab; /**< want tab */
	BOOL m_bHasLines; /**< has lines*/
	SAutoRefPtr<ISkinObj> m_pLineSkin;
	SLayoutSize	m_indent;
};
SNSEND
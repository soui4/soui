#pragma once

#include "core/SItemPanel.h"
#include "interface/SAdapter-i.h"
#include "interface/SListViewItemLocator-i.h"
SNSBEGIN

class SOUI_EXP SListView
    : public TPanelProxy<IListView>
    , protected SHostProxy
    , protected IItemContainer {
    DEF_SOBJECT(SPanel, L"listview")

    friend class SListViewDataSetObserver;

  public:
    SListView();
    ~SListView();

    STDMETHOD_(BOOL, SetAdapter)(THIS_ ILvAdapter *adapter) OVERRIDE;

    STDMETHOD_(ILvAdapter *, GetAdapter)(THIS) SCONST OVERRIDE;

    STDMETHOD_(IListViewItemLocator *, GetItemLocator)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetItemLocator)(THIS_ IListViewItemLocator *pItemLocator) OVERRIDE;

    STDMETHOD_(void, EnsureVisible)(THIS_ int iItem) OVERRIDE;

    STDMETHOD_(void, SetSel)(THIS_ int iItem, BOOL bNotify = FALSE) OVERRIDE;

    STDMETHOD_(int, GetSel)(THIS) SCONST OVERRIDE;

    STDMETHOD_(IItemPanel *, HitTest)(THIS_ const POINT *pt) SCONST OVERRIDE;

    SItemPanel *HitTest(CPoint &pt) const;

	STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz,int nParentWid, int nParentHei) OVERRIDE;

  protected:
    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture);
    virtual BOOL OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const;
    virtual BOOL IsItemRedrawDelay() const;

  protected:
    void onDataSetChanged();
    void onDataSetInvalidated();
    void onItemDataChanged(int iItem);

  protected:
    BOOL OnItemClick(IEvtArgs *pEvt);

  protected:
    virtual BOOL OnScroll(BOOL bVertical, UINT uCode, int nPos);
    virtual int GetScrollLineSize(BOOL bVertical);
    virtual BOOL CreateChildren(SXmlNode xmlNode);

    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);
    virtual UINT WINAPI OnGetDlgCode() const;
    virtual BOOL OnSetCursor(const CPoint &pt);

    virtual void OnColorize(COLORREF cr);
    virtual void OnScaleChanged(int nScale);
    virtual HRESULT OnLanguageChanged();
    virtual void OnRebuildFont();

  protected:
    void DispatchMessage2Items(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void UpdateScrollBar();
    void RedrawItem(SOsrPanel *pItem);
    SItemPanel *GetItemPanel(int iItem);

    void UpdateVisibleItems();
    void UpdateVisibleItem(int iItem);

    void OnPaint(IRenderTarget *pRT);
    void OnSize(UINT nType, CSize size);
    void OnDestroy();

    LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);

    void OnMouseLeave();

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    void OnKillFocus(SWND wndFocus);

    void OnSetFocus(SWND wndOld);

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
        ATTR_BOOL(L"wantTab", m_bWantTab, FALSE)
        ATTR_BOOL(L"vertical", m_bVertical, FALSE)
    SOUI_ATTRS_END()
  protected:
    SAutoRefPtr<ILvAdapter> m_adapter;
    SAutoRefPtr<ILvDataSetObserver> m_observer;
    SAutoRefPtr<IListViewItemLocator> m_lvItemLocator; //列表项定位接口
    struct ItemInfo
    {
        SItemPanel *pItem;
        int nType;
    };

    bool m_bPendingUpdate;    // response for data set changed in OnShowWindow.
    int m_iPendingUpdateItem; //-1 for all. -2 for nothing
    int m_iPendingViewItem;   //-1 for init

    int m_iFirstVisible;        //第一个显示项索引
    SList<ItemInfo> m_lstItems; //当前正在显示的项
    SOsrPanel *m_itemCapture;   // The item panel that has been set capture.

    int m_iSelItem;
    SOsrPanel *m_pHoverItem;
    BOOL m_bDataSetInvalidated;

    SArray<SList<SItemPanel *> *> m_itemRecycle; // item回收站,每一种样式在回收站中保持一个列表，以便重复利用

    SXmlDoc m_xmlTemplate;
    SAutoRefPtr<ISkinObj> m_pSkinDivider;
    SLayoutSize m_nDividerSize;
    BOOL m_bWantTab;
    BOOL m_bVertical;
};
SNSEND
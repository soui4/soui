#pragma once
#include <sobject/Sobject.hpp>
#include <interface/SNcPainter-i.h>
#include <core/SItemPanel.h>

SNSBEGIN

class SNcPanel;
class SHostWnd;
class SNcPainter
    : public TObjRefImpl<SObjectImpl<INcPainter>>
    , IItemContainer
    , IHostProxy {
    DEF_SOBJECT_EX(TObjRefImpl<SObjectImpl<INcPainter>>, L"ncpainter", Undef)
    friend class SHostWnd;
    friend class SNcPanel;
    enum
    {
        IDC_SYS_ICON = 100,
        IDC_SYS_TITLE = 101,
        IDC_SYS_CLOSE = 102,
        IDC_SYS_MIN = 103,
        IDC_SYS_MAX = 104,
        IDC_SYS_RESTORE = 105,
    };

  public:
    SNcPainter(SHostWnd *pHost);
    ~SNcPainter(void);

    static int toNcBuiltinID(const SStringW &str);
	static void updateSystemButton(SWindow *pRoot,UINT nResizeMode);
  public:
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pXmlNode) OVERRIDE;

  public:
    STDMETHOD_(IWindow *, GetRoot)(THIS) OVERRIDE;

  protected:
    virtual void OnItemSetCapture(SOsrPanel *pItem, BOOL bCapture);

    virtual BOOL OnItemGetRect(const SOsrPanel *pItem, CRect &rcItem) const;

    virtual BOOL IsItemRedrawDelay() const;

    virtual IObject *GetHost();

  protected:
    virtual BOOL OnHostFireEvent(IEvtArgs *e);

    virtual BOOL IsHostUpdateLocked() const;

    virtual BOOL IsHostVisible() const;

    virtual CRect GetHostRect() const;

    virtual void InvalidateHostRect(LPCRECT pRc,BOOL bClip);

    virtual ISwndContainer *GetHostContainer();

    virtual IRenderTarget *OnGetHostRenderTarget(LPCRECT rc, GrtFlag gdcFlags);

    virtual void OnReleaseHostRenderTarget(IRenderTarget *pRT, LPCRECT rc, GrtFlag gdcFlags);

	virtual void OnLayoutDirty();

  protected:
    int GetScale() const;
    CSize GetNcSize() const;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE(L"titleHeight", m_titleHeight, FALSE)
        ATTR_LAYOUTSIZE(L"borderWidth", m_borderWidth, FALSE)
        ATTR_SKIN(L"skinBorder", m_skinBorder, FALSE)
        ATTR_BOOL(L"system", m_bSysNcPainter, FALSE)
    SOUI_ATTRS_END()

  protected:
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);
    BOOL OnNcActivate(BOOL bActive);
    UINT OnNcHitTest(CPoint point);
    void OnNcPaint(HRGN hRgn);
    LRESULT OnSetText(LPCTSTR pszText);
    LRESULT OnRepaint(UINT msg, WPARAM wp, LPARAM lp);
    void OnNcDestroy();
    void OnNcLButtonDown(UINT flag, CPoint pt);
    void OnLButtonUp(WPARAM wp, LPARAM lp);
    void OnMouseMove(WPARAM wp, LPARAM lp);
    LRESULT OnNcMouseEvent(UINT msg, WPARAM wp, LPARAM lp);
    LRESULT OnNcMouseLeave(UINT msg, WPARAM wp, LPARAM lp);
    void OnSize(UINT nType, CSize size);
	void OnTimer(UINT_PTR tid);
    BEGIN_MSG_MAP_EX(SNcPainter)
        if (m_bSysNcPainter)
            return FALSE;
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_NCACTIVATE(OnNcActivate)
        MSG_WM_NCHITTEST(OnNcHitTest)
        MSG_WM_NCPAINT(OnNcPaint)
        MSG_WM_NCDESTROY(OnNcDestroy)
        MSG_WM_SETTEXT(OnSetText)
        MSG_WM_SIZE(OnSize)
        MSG_WM_NCLBUTTONDOWN(OnNcLButtonDown)
        MESSAGE_HANDLER_EX(WM_NCMOUSELEAVE, OnNcMouseLeave)
        MESSAGE_RANGE_HANDLER_EX(WM_NCMOUSEMOVE, WM_NCMBUTTONDBLCLK, OnNcMouseEvent)
        MESSAGE_HANDLER_EX(WM_SETTINGCHANGE, OnRepaint)
        MESSAGE_HANDLER_EX(WM_SYSCOLORCHANGE, OnRepaint)
		MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()

  protected:
    void Reset();
    BOOL IsDrawNc() const;
    void PaintCaption();
    void UpdateToolTip();

  private:
    SHostWnd *m_pHost;

    SLayoutSize m_titleHeight;
    SLayoutSize m_borderWidth;
    SAutoRefPtr<ISkinObj> m_skinBorder;
    BOOL m_bSysNcPainter;

    SAutoRefPtr<IRenderTarget> m_memRT;
    SAutoRefPtr<IRenderTarget> m_memLeft;
    SAutoRefPtr<IRenderTarget> m_memRight;
    SAutoRefPtr<IRenderTarget> m_memTop;
    SAutoRefPtr<IRenderTarget> m_memBottom;
    UINT m_htPart;
    BOOL m_bInPaint;
    BOOL m_bLButtonDown;
    BOOL m_bMouseHover;
    SNcPanel *m_root;
};

class SNcPanel : public SOsrPanel {
    DEF_SOBJECT(SOsrPanel, L"ncpanel")
  public:
    SNcPanel(IHostProxy *pFrameHost, IItemContainer *pItemContainer);

    void SetActive(BOOL bActive);

  protected:
    BOOL OnEraseBkgnd(IRenderTarget *pRT);
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
    SOUI_MSG_MAP_END()

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skinActive", m_skinActive, FALSE)
        ATTR_SKIN(L"skinInactive", m_skinInactive, FALSE)
        ATTR_COLOR(L"colorActiveTitle", m_crActiveTitle, TRUE)
        ATTR_COLOR(L"colorInactiveTitle", m_crInactiveTitle, TRUE)
    SOUI_ATTRS_END()

  protected:
    SAutoRefPtr<ISkinObj> m_skinActive;
    SAutoRefPtr<ISkinObj> m_skinInactive;
    COLORREF m_crActiveTitle;
    COLORREF m_crInactiveTitle;

    BOOL m_bActive;
};

SNSEND

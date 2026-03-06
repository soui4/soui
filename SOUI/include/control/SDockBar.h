#ifndef __SDOCKBAR__H__
#define __SDOCKBAR__H__

#include <core/SWnd.h>

SNSBEGIN

class SOUI_EXP SDockBar : public SWindow {
    DEF_SOBJECT(SWindow, L"dockbar")

  public:
    SDockBar(void);
    virtual ~SDockBar(void);

  public:
    STDMETHOD_(BOOL, IsDisplay)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;

  protected:
    void OnDecendantFocusChanged(SWND swnd, BOOL bSet) override;
    BOOL OnSetCursor(const CPoint &pt) override;
    UINT OnNcHitTest(const CPoint &pt) override;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"captionSkin", m_skinCaption, TRUE)
        ATTR_SKIN(L"closeBtnSkin", m_skinCloseBtn, TRUE)
        ATTR_LAYOUTSIZE(L"captionHeight", m_nCaptionHeight, TRUE)
        ATTR_BOOL(L"resizable", m_bResizable, TRUE)
    SOUI_ATTRS_END()

  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnMouseMove(UINT nFlags, CPoint point);
    void OnMouseLeave();
    void OnShowWindow(BOOL bShow, UINT nStatus);
    void OnNcLButtonDown(UINT nHitTest, CPoint point);
    void OnNcLButtonUp(UINT nHitTest, CPoint point);
    void OnNcMouseMove(UINT nHitTest, CPoint point);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MSG_WM_NCLBUTTONDOWN(OnNcLButtonDown)
        MSG_WM_NCLBUTTONUP(OnNcLButtonUp)
        MSG_WM_NCMOUSEMOVE(OnNcMouseMove)
    SOUI_MSG_MAP_END()

  private:
    void OnCloseBtnClick();
    CRect GetCaptionRect() const;
    CRect GetCloseBtnRect() const;
    BOOL IsPointOnCloseBtn(CPoint point) const;
    void UpdateResizeCursor(int nHitTest);

  protected:
    SAutoRefPtr<ISkinObj> m_skinCaption;
    SAutoRefPtr<ISkinObj> m_skinCloseBtn;
    BOOL m_bCloseBtnHover;
    BOOL m_bCloseBtnPressed;
    BOOL m_bActive;
    BOOL m_bResizable;
    SLayoutSize m_nCaptionHeight;

    int m_nResizeHitTest;
    BOOL m_bIsResizing;
    CPoint m_ptResizeStart;
    CSize m_szResizeStart;
};

SNSEND

#endif // __SDOCKBAR__H__

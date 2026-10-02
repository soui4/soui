#ifndef __SDOCKBAR__H__
#define __SDOCKBAR__H__

#include <core/SWnd.h>
#include <layout/SFrameLayoutParamStruct.h>

SNSBEGIN

class SDockFloatWnd;

class SOUI_EXP SDockBar : public SWindow {
    DEF_SOBJECT(SWindow, L"dockbar")

  public:
    SDockBar(void);
    virtual ~SDockBar(void);

  public:
    STDMETHOD_(BOOL, IsDisplay)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT *prc) SCONST OVERRIDE;

    /**
     * @brief Check whether the dock bar is currently floating
     */
    BOOL IsFloating() const
    {
        return m_bFloating;
    }

    /**
     * @brief Float the dock bar into a separate host window
     * @param ptScreen desired screen top-left of the float window
     * @return TRUE on success
     */
    BOOL Float(const CPoint &ptScreen);

    /**
     * @brief Dock the floating dock bar back into its dock parent
     */
    void Dock();

  protected:
    void OnDecendantFocusChanged(SWND swnd, BOOL bSet) override;
    BOOL OnSetCursor(const CPoint &pt) override;
    UINT OnNcHitTest(const CPoint &pt) override;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"captionSkin", m_skinCaption, TRUE)
        ATTR_SKIN(L"closeBtnSkin", m_skinCloseBtn, TRUE)
        ATTR_SKIN(L"dockBtnSkin", m_skinDockBtn, TRUE)
        ATTR_LAYOUTSIZE(L"captionHeight", m_nCaptionHeight, TRUE)
        ATTR_BOOL(L"resizable", m_bResizable, TRUE)
        ATTR_BOOL(L"floatable", m_bFloatable, TRUE)
    SOUI_ATTRS_END()

  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnLButtonDblClk(UINT nFlags, CPoint point);
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
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
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
    CRect GetDockBtnRect() const;
    BOOL IsPointOnCloseBtn(CPoint point) const;
    BOOL IsPointOnDockBtn(CPoint point) const;
    BOOL IsPointOnCaption(CPoint point) const;
    CPoint GetWindowScreenTopLeft() const;
    void UpdateResizeCursor(int nHitTest);

  protected:
    SAutoRefPtr<ISkinObj> m_skinCaption;
    SAutoRefPtr<ISkinObj> m_skinCloseBtn;
    SAutoRefPtr<ISkinObj> m_skinDockBtn;
    BOOL m_bCloseBtnHover;
    BOOL m_bCloseBtnPressed;
    BOOL m_bDockBtnHover;
    BOOL m_bDockBtnPressed;
    BOOL m_bActive;
    BOOL m_bResizable;
    BOOL m_bFloatable;
    SLayoutSize m_nCaptionHeight;

    int m_nResizeHitTest;
    BOOL m_bIsResizing;
    CPoint m_ptResizeStart;
    CSize m_szResizeStart;

    BOOL m_bFloating;
    BOOL m_bDragFloating;
    CPoint m_ptDragStart;
    CPoint m_ptFloatDragScreen; /**< 拖动中继锚点：浮动窗口左上角屏幕坐标 */
    CPoint m_ptMouseDragScreen; /**< 拖动中继锚点：按下时鼠标屏幕坐标 */
    SWindow *m_pDockParent;
    SWindow *m_pDockPrevSibling;
    SFrameLayoutParamStruct m_paramDocked;
    SDockFloatWnd *m_pFloatWnd;
};

SNSEND

#endif /**< __SDOCKBAR__H__ */

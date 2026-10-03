#include "souistd.h"
#include "control/SDockBar.h"
#include "control/SDockFloatWnd.h"
#include "layout/SFrameLayout.h"

SNSBEGIN

SDockBar::SDockBar(void)
    : m_bCloseBtnHover(FALSE)
    , m_bCloseBtnPressed(FALSE)
    , m_bDockBtnHover(FALSE)
    , m_bDockBtnPressed(FALSE)
    , m_nCaptionHeight(24, dp)
    , m_bActive(FALSE)
    , m_bResizable(FALSE)
    , m_bFloatable(TRUE)
    , m_nResizeHitTest(0)
    , m_bIsResizing(FALSE)
    , m_bFloating(FALSE)
    , m_bDragFloating(FALSE)
    , m_pDockParent(NULL)
    , m_pDockPrevSibling(NULL)
    , m_bInitFloating(FALSE)
    , m_bInitFloatPending(FALSE)
    , m_pFloatWnd(NULL)
    , m_skinCloseBtn(GETBUILTINSKIN(SKIN_SYS_BTN_MINI_CLOSE))
{
    m_bFocusable = TRUE;
}

SDockBar::~SDockBar(void)
{
    // If the dock bar is destroyed while floating (e.g. the float host is torn
    // down), unregister from the dock parent so FindChildByID/ByName no longer
    // reports a dangling window. The dock parent may already be destroyed when
    // the whole application shuts down, so validate it through the window map.
    if (m_bFloating && m_pDockParent &&
        SWindowMgr::GetWindow(m_pDockParent->GetSwnd()) == m_pDockParent)
    {
        m_pDockParent->RemoveDetachedChild(this);
    }
}

void SDockBar::OnDecendantFocusChanged(SWND swnd, BOOL bSet)
{
    m_bActive = bSet;
    InvalidateRect(GetCaptionRect());
    __baseCls::OnDecendantFocusChanged(swnd, bSet);
}

CRect SDockBar::GetCaptionRect() const
{
    CRect rcClient;
    GetClientRect(&rcClient);
    rcClient.bottom = rcClient.top + m_nCaptionHeight.toPixelSize(GetScale());
    return rcClient;
}

CRect SDockBar::GetCloseBtnRect() const
{
    CRect rcCaption = GetCaptionRect();
    int nBtnSize = smin(rcCaption.Height(), 24);
    CRect rcCloseBtn;
    rcCloseBtn.right = rcCaption.right - 4;
    rcCloseBtn.left = rcCloseBtn.right - nBtnSize;
    rcCloseBtn.top = rcCaption.top + (rcCaption.Height() - nBtnSize) / 2;
    rcCloseBtn.bottom = rcCloseBtn.top + nBtnSize;
    return rcCloseBtn;
}

BOOL SDockBar::IsPointOnCloseBtn(CPoint point) const
{
    CRect rcCloseBtn = GetCloseBtnRect();
    return rcCloseBtn.PtInRect(point);
}

CRect SDockBar::GetDockBtnRect() const
{
    CRect rcCaption = GetCaptionRect();
    CRect rcCloseBtn = GetCloseBtnRect();
    // Keep the dock button the same size as the close button; the dock button
    // skin is stretched into this rect when drawn.
    int nBtnSize = rcCloseBtn.Width();
    CRect rcDockBtn;
    rcDockBtn.right = rcCloseBtn.left - 4;
    rcDockBtn.left = rcDockBtn.right - nBtnSize;
    rcDockBtn.top = rcCaption.top + (rcCaption.Height() - nBtnSize) / 2;
    rcDockBtn.bottom = rcDockBtn.top + nBtnSize;
    return rcDockBtn;
}

BOOL SDockBar::IsPointOnDockBtn(CPoint point) const
{
    if (!m_bFloating)
        return FALSE;
    CRect rcDockBtn = GetDockBtnRect();
    return rcDockBtn.PtInRect(point);
}

BOOL SDockBar::IsPointOnCaption(CPoint point) const
{
    return GetCaptionRect().PtInRect(point);
}

CPoint SDockBar::GetWindowScreenTopLeft() const
{
    CPoint pt;
    const ISwndContainer *pContainer = GetContainer();
    if (!pContainer)
        return pt;
    CRect rcWnd = GetWindowRect();
    pContainer->FrameToHost(&rcWnd);
    pt = rcWnd.TopLeft();
    ::ClientToScreen(pContainer->GetHostHwnd(), &pt);
    return pt;
}

UINT SDockBar::OnNcHitTest(const CPoint &point)
{
    if (!m_bResizable)
        return HTNOWHERE;

    CRect rcWnd, rcClient;
    GetWindowRect(&rcWnd);
    GetClientRect(&rcClient);

    SFrameLayoutParam *pParam = sobj_cast<SFrameLayoutParam>(GetLayoutParam());
    if (!pParam)
        return HTNOWHERE;
    if (rcClient.PtInRect(point))
        return HTCLIENT;
    DockPosition dockPos = pParam->dockPos;

    if (dockPos == DockLeft)
    {
        if (point.x >= rcClient.right && point.x <= rcWnd.right)
            return HTRIGHT;
    }
    else if (dockPos == DockRight)
    {
        if (point.x >= rcWnd.left && point.x <= rcClient.left)
            return HTLEFT;
    }
    else if (dockPos == DockTop)
    {
        if (point.y >= rcClient.bottom && point.y <= rcWnd.bottom)
            return HTBOTTOM;
    }
    else if (dockPos == DockBottom)
    {
        if (point.y >= rcWnd.top && point.y <= rcClient.top)
            return HTTOP;
    }

    return HTNOWHERE;
}

void SDockBar::UpdateResizeCursor(int nHitTest)
{
    HCURSOR hCursor = NULL;
    switch (nHitTest)
    {
    case HTLEFT:
    case HTRIGHT:
        hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
        break;
    case HTTOP:
    case HTBOTTOM:
        hCursor = ::LoadCursor(NULL, IDC_SIZENS);
        break;
    default:
        hCursor = ::LoadCursor(NULL, IDC_ARROW);
        break;
    }
    if (hCursor)
        ::SetCursor(hCursor);
}

BOOL SDockBar::OnSetCursor(const CPoint &pt)
{
    if (m_bIsResizing)
    {
        UpdateResizeCursor(m_nResizeHitTest);
        return TRUE;
    }

    int nHitTest = OnNcHitTest(pt);
    if (nHitTest != HTNOWHERE)
    {
        UpdateResizeCursor(nHitTest);
        return TRUE;
    }

    return FALSE;
}

void SDockBar::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);
    CRect rcCaption = GetCaptionRect();

    if (m_skinCaption)
    {
        int nSkinState = m_bActive ? 1 : 0;
        m_skinCaption->DrawByIndex(pRT, rcCaption, nSkinState);
    }
    else
    {
        COLORREF clrCaption = m_bActive ? RGBA(100, 150, 255, 255) : RGBA(200, 200, 200, 255);
        pRT->FillSolidRect(&rcCaption, clrCaption);
    }

    CRect rcText = rcCaption;
    CRect rcCloseBtn = GetCloseBtnRect();
    rcText.right = rcCloseBtn.left - 8;
    SStringT strTitle = GetWindowText();
    if (!strTitle.IsEmpty())
    {
        COLORREF oldTextColor = 0;
        if (!m_skinCaption)
        {
            COLORREF clrText = m_bActive ? RGBA(255, 255, 255, 255) : RGBA(0, 0, 0, 255);
            oldTextColor = pRT->SetTextColor(clrText);
        }
        pRT->DrawText(strTitle, strTitle.GetLength(), &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        if (!m_skinCaption)
        {
            pRT->SetTextColor(oldTextColor);
        }
    }

    DWORD dwState = 0;
    if (m_bCloseBtnPressed)
        dwState |= WndState_PushDown;
    else if (m_bCloseBtnHover)
        dwState |= WndState_Hover;

    if (m_skinCloseBtn)
    {
        m_skinCloseBtn->DrawByState(pRT, rcCloseBtn, dwState);
    }
    else
    {
        COLORREF clrBtn = RGBA(255, 100, 100, 255);
        if (m_bCloseBtnPressed)
            clrBtn = RGBA(200, 50, 50, 255);
        else if (m_bCloseBtnHover)
            clrBtn = RGBA(255, 150, 150, 255);

        pRT->FillSolidRect(&rcCloseBtn, clrBtn);

        COLORREF oldColor = pRT->SetTextColor(RGBA(255, 255, 255, 255));
        pRT->DrawText(_T("x"), 1, &rcCloseBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        pRT->SetTextColor(oldColor);
    }

    // Draw the dock button while floating (clicking it docks the bar back).
    if (m_bFloating)
    {
        CRect rcDockBtn = GetDockBtnRect();
        DWORD dwDockState = 0;
        if (m_bDockBtnPressed)
            dwDockState |= WndState_PushDown;
        else if (m_bDockBtnHover)
            dwDockState |= WndState_Hover;

        if (m_skinDockBtn)
        {
            // Clamp the state index: single-frame skins (e.g. a 1-frame SVG icon) have
            // no dedicated hover/pressed frames, so never select a frame beyond the
            // available states or the button would draw nothing.
            int nMaxState = m_skinDockBtn->GetStates();
            int iState = SState2Index::GetDefIndex(dwDockState, true);
            if (iState >= nMaxState)
                iState = nMaxState - 1;
            m_skinDockBtn->DrawByState(pRT, rcDockBtn, 1 << iState);

            // For single-frame skins only, overlay an explicit feedback tint since
            // the clamped index loses the hover/pressed distinction.
            if (nMaxState < 3)
            {
                if (m_bDockBtnPressed)
                    pRT->FillSolidRect(&rcDockBtn, RGBA(0, 0, 0, 60));
                else if (m_bDockBtnHover)
                    pRT->FillSolidRect(&rcDockBtn, RGBA(255, 255, 255, 50));
            }
        }
        else
        {
            // No skin configured: draw a simple button with the render target API.
            // The background follows the button state (normal/hover/pressed).
            COLORREF clrBg = RGBA(200, 208, 220, 255);
            if (m_bDockBtnPressed)
                clrBg = RGBA(110, 140, 180, 255);
            else if (m_bDockBtnHover)
                clrBg = RGBA(226, 232, 240, 255);

            pRT->FillSolidRect(&rcDockBtn, clrBg);

            // A simple "dock" glyph: a downward triangle arrow, drawn centered
            // with the render target API.
            int nGlyph = smin(rcDockBtn.Width(), rcDockBtn.Height()) / 3;
            if (nGlyph > 2)
            {
                CPoint cpt = rcDockBtn.CenterPoint();
                POINT pts[3] = {
                    {cpt.x - nGlyph / 2, cpt.y - nGlyph / 2},
                    {cpt.x + nGlyph / 2, cpt.y - nGlyph / 2},
                    {cpt.x, cpt.y + nGlyph / 2},
                };
                SAutoRefPtr<IBrushS> pBrush, pOldBrush;
                pRT->CreateSolidColorBrush(RGBA(255, 255, 255, 255), &pBrush);
                pRT->SelectObject(pBrush, (IRenderObj **)&pOldBrush);
                pRT->FillPolygon(pts, 3);
                pRT->SelectObject(pOldBrush, NULL);
            }
        }
    }
    AfterPaint(pRT, painter);
}

void SDockBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (IsPointOnCloseBtn(point))
    {
        m_bCloseBtnPressed = TRUE;
        Invalidate();
        return;
    }
    if (IsPointOnDockBtn(point))
    {
        m_bDockBtnPressed = TRUE;
        Invalidate();
        return;
    }
    if (IsPointOnCaption(point) && (m_bFloating || m_bFloatable))
    {
        // Dragging the caption: float the dock bar (docked) or move the float window (floating).
        m_bDragFloating = TRUE;
        m_ptDragStart = point;
        m_ptMouseDragScreen = GetWindowScreenTopLeft();
        m_ptMouseDragScreen.Offset(point);
        if (m_bFloating && m_pFloatWnd)
        {
            CRect rcFloat;
            ::GetWindowRect(m_pFloatWnd->GetHwnd(), &rcFloat);
            m_ptFloatDragScreen = rcFloat.TopLeft();
        }
        SetCapture();
        return;
    }
    if (IsFocusable())
        SetFocus();
}

void SDockBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_bCloseBtnPressed && IsPointOnCloseBtn(point))
    {
        OnCloseBtnClick();
    }
    m_bCloseBtnPressed = FALSE;
    Invalidate();

    if (m_bDockBtnPressed && IsPointOnDockBtn(point))
    {
        Dock();
    }
    m_bDockBtnPressed = FALSE;
    Invalidate();

    if (m_bDragFloating)
    {
        m_bDragFloating = FALSE;
        ReleaseCapture();
    }
}

void SDockBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (IsPointOnCaption(point) && !IsPointOnCloseBtn(point) && !IsPointOnDockBtn(point))
    {
        if (m_bFloating)
        {
            Dock();
        }
        else if (m_bFloatable)
        {
            Float(GetWindowScreenTopLeft());
        }
    }
}

void SDockBar::OnMouseMove(UINT nFlags, CPoint point)
{
    BOOL bNewHover = IsPointOnCloseBtn(point);
    if (bNewHover != m_bCloseBtnHover)
    {
        m_bCloseBtnHover = bNewHover;
        Invalidate();
    }

    BOOL bNewDockHover = IsPointOnDockBtn(point);
    if (bNewDockHover != m_bDockBtnHover)
    {
        m_bDockBtnHover = bNewDockHover;
        Invalidate();
    }

    if (m_bDragFloating)
    {
        if (m_bFloating)
        {
            // Move the float window keeping the grab point under the cursor.
            if (m_pFloatWnd)
            {
                CPoint ptMouseNow;
                ::GetCursorPos(&ptMouseNow);
                CPoint ptTopLeft = m_ptFloatDragScreen;
                ptTopLeft.Offset(ptMouseNow - m_ptMouseDragScreen);
                m_pFloatWnd->MoveTo(ptTopLeft);
            }
        }
        else if (abs(point.x - m_ptDragStart.x) > 8 || abs(point.y - m_ptDragStart.y) > 8)
        {
            // Float the dock bar at its current position, then relay the drag to
            // the new float window so the user can keep dragging without
            // releasing the mouse button.
            CPoint ptFloatPos = GetWindowScreenTopLeft();

            // Release the capture held by the docked container first so it does
            // not keep a dangling reference to this window.
            m_bDragFloating = FALSE;
            ReleaseCapture();

            if (Float(ptFloatPos))
            {
                // Re-anchor the drag to the *actual* float window screen position:
                // the position laid out by the float host may differ from
                // ptFloatPos (container coordinate conversions). The current
                // message was dispatched by the docked container, so its local
                // `point` is not comparable across the switch; use the cursor's
                // screen position instead. This keeps the grab point under the
                // cursor and synchronizes coordinates.
                CPoint ptMouseNow;
                ::GetCursorPos(&ptMouseNow);
                CRect rcFloat;
                ::GetWindowRect(m_pFloatWnd->GetHwnd(), &rcFloat);
                m_ptFloatDragScreen = rcFloat.TopLeft();
                m_ptMouseDragScreen = ptMouseNow;

                // Resume dragging inside the float host.
                m_bDragFloating = TRUE;
                SetCapture();
            }
        }
    }
}

void SDockBar::OnMouseLeave()
{
    SetMsgHandled(FALSE);

    if (m_bCloseBtnHover)
    {
        m_bCloseBtnHover = FALSE;
        Invalidate();
    }
    if (m_bDockBtnHover)
    {
        m_bDockBtnHover = FALSE;
        Invalidate();
    }
}

void SDockBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __baseCls::OnShowWindow(bShow, nStatus);
    // Init-time float: the "floating" attribute asks to start the dock bar in
    // float mode. Defer through a one-shot timer so the first layout has run
    // and the float window can be placed at the dock bar's real size/position.
    if (bShow && m_bInitFloating && !m_bFloating && !m_bInitFloatPending)
    {
        m_bInitFloatPending = TRUE;
        SetTimer(kTimerIdInitFloat, 0);
    }
    // Keep the float host in sync with this dock bar's visibility: hiding a
    // floating dock bar also hides its host window instead of leaving an empty
    // window on screen; a later Show()/SetVisible(TRUE) restores the bar and
    // keeps it floating.
    if (m_bFloating && m_pFloatWnd)
        ::ShowWindow(m_pFloatWnd->GetHwnd(), bShow ? SW_SHOW : SW_HIDE);
    RequestRelayout();
}

void SDockBar::OnTimer(char cTimerID)
{
    if (cTimerID == kTimerIdInitFloat)
    {
        KillTimer(kTimerIdInitFloat);
        m_bInitFloatPending = FALSE;
        if (m_bInitFloating && !m_bFloating)
        {
            m_bInitFloating = FALSE; // consume the request
            Float(GetWindowScreenTopLeft());
        }
    }
}
void SDockBar::OnCloseBtnClick()
{
    // Hiding the dock bar in both docked and floating states; re-showing it
    // later via Show()/SetVisible(TRUE) restores it in its current state.
    SetVisible(FALSE, TRUE);
}

void SDockBar::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
    if (m_bResizable && (nHitTest == HTLEFT || nHitTest == HTRIGHT || nHitTest == HTTOP || nHitTest == HTBOTTOM))
    {
        m_bIsResizing = TRUE;
        m_nResizeHitTest = nHitTest;
        m_ptResizeStart = point;
        CRect rcWnd;
        GetWindowRect(&rcWnd);
        m_szResizeStart.cx = rcWnd.Width();
        m_szResizeStart.cy = rcWnd.Height();
        SetCapture();
        SetMsgHandled(TRUE);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void SDockBar::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
    if (m_bIsResizing)
    {
        m_bIsResizing = FALSE;
        m_nResizeHitTest = 0;
        ReleaseCapture();
        SetMsgHandled(TRUE);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void SDockBar::OnNcMouseMove(UINT nHitTest, CPoint point)
{
    if (m_bIsResizing)
    {
        SFrameLayoutParam *pParam = sobj_cast<SFrameLayoutParam>(GetLayoutParam());
        if (pParam)
        {
            DockPosition dockPos = pParam->dockPos;
            int nDeltaX = point.x - m_ptResizeStart.x;
            int nDeltaY = point.y - m_ptResizeStart.y;
            int nNewWidth = m_szResizeStart.cx;
            int nNewHeight = m_szResizeStart.cy;

            if (dockPos == DockLeft)
            {
                nNewWidth = m_szResizeStart.cx + nDeltaX;
            }
            else if (dockPos == DockRight)
            {
                nNewWidth = m_szResizeStart.cx - nDeltaX;
            }
            else if (dockPos == DockTop)
            {
                nNewHeight = m_szResizeStart.cy + nDeltaY;
            }
            else if (dockPos == DockBottom)
            {
                nNewHeight = m_szResizeStart.cy - nDeltaY;
            }

            nNewWidth = smax(nNewWidth, 30);
            nNewHeight = smax(nNewHeight, 30);

            if (dockPos == DockLeft || dockPos == DockRight)
            {
                pParam->width.setSize(nNewWidth, px);
            }
            else
            {
                pParam->height.setSize(nNewHeight, px);
            }

            RequestRelayout();
        }
    }
    else if (m_bResizable && (nHitTest == HTLEFT || nHitTest == HTRIGHT || nHitTest == HTTOP || nHitTest == HTBOTTOM))
    {
        UpdateResizeCursor(nHitTest);
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

BOOL SDockBar::Float(const CPoint &ptScreen, const CSize &szFloat /* = CSize(0,0) */)
{
    if (!m_bFloatable || m_bFloating)
        return FALSE;
    SWindow *pParent = GetParent();
    ISwndContainer *pContainer = GetContainer();
    if (!pParent || !pContainer)
        return FALSE;
    SFrameLayoutParam *pParam = sobj_cast<SFrameLayoutParam>(GetLayoutParam());
    if (!pParam)
        return FALSE;
    HWND hHost = pContainer->GetHostHwnd();

    // Snapshot the docked state (dock parent, sibling order and layout param).
    m_paramDocked = *(SFrameLayoutParamStruct *)pParam;
    m_pDockParent = pParent;
    m_pDockPrevSibling = GetWindow(GSW_PREVSIBLING);

    CRect rcWnd = GetWindowRect();
    CSize szFloatOut = szFloat;
    if (szFloatOut.cx <= 0 || szFloatOut.cy <= 0)
        szFloatOut = rcWnd.Size();
    if (szFloatOut.cx <= 0 || szFloatOut.cy <= 0)
        szFloatOut = CSize(300, 300); // not laid out yet: use a default size

    // Detach from the dock parent.
    pParent->RemoveChild(this);

    // While floating, the dock bar fills the float host.
    pParam->dockPos = DockMainView;
    pParam->width.setMatchParent();
    pParam->height.setMatchParent();

    SDockFloatWnd *pFloatWnd = new SDockFloatWnd();
    if (!pFloatWnd->Create(this, hHost, ptScreen, szFloatOut))
    {
        // Create() already released pFloatWnd on failure; roll back the docked state.
        *(SFrameLayoutParamStruct *)pParam = m_paramDocked;
        pParent->InsertChild(this, m_pDockPrevSibling);
        return FALSE;
    }

    m_pFloatWnd = pFloatWnd;
    m_bFloating = TRUE;
    // Keep the dock bar discoverable from the dock parent (FindChildByID/ByName)
    // even though it is physically hosted in the float window now.
    pParent->AddDetachedChild(this);
    pParent->RequestRelayout();
    return TRUE;
}

void SDockBar::Dock()
{
    if (!m_bFloating)
        return;
    SWindow *pFloatParent = GetParent();
    if (!pFloatParent)
        return;
    SDockFloatWnd *pFloatWnd = m_pFloatWnd;
    SWindow *pDockParent = m_pDockParent;

    m_bFloating = FALSE;
    m_pFloatWnd = NULL;

    // Detach from the float host.
    pFloatParent->RemoveChild(this);

    // Restore the docked layout param.
    SFrameLayoutParam *pParam = sobj_cast<SFrameLayoutParam>(GetLayoutParam());
    if (pParam)
        *(SFrameLayoutParamStruct *)pParam = m_paramDocked;

    // The dock parent may already be destroyed when the application shuts down
    // (the owned float window is torn down after its owner); in that case there
    // is nowhere to re-dock, the float host is just closed.
    if (pDockParent && SWindowMgr::GetWindow(pDockParent->GetSwnd()) != pDockParent)
        pDockParent = NULL;

    // Unregister from the dock parent's detached list before re-inserting.
    if (pDockParent)
        pDockParent->RemoveDetachedChild(this);

    // Re-insert into the dock parent (after the previous sibling if it is still valid).
    if (pDockParent)
    {
        SWindow *pInsertAfter = ICWND_LAST;
        if (m_pDockPrevSibling && m_pDockPrevSibling->GetParent() == pDockParent)
            pInsertAfter = m_pDockPrevSibling;
        pDockParent->InsertChild(this, pInsertAfter);
        pDockParent->RequestRelayout();
    }
    else
    {
        // Dock parent is gone: nothing to restore, mark the bar as detached for
        // good so the float host teardown destroys it with its own tree.
        m_pDockParent = NULL;
    }

    // Close the float window asynchronously.
    if (pFloatWnd)
        pFloatWnd->RequestClose();
}

BOOL SDockBar::IsDisplay() const
{
    return FALSE;
}

void SDockBar::GetChildrenLayoutRect(RECT *prc) const
{
    GetClientRect(prc);
    prc->top += m_nCaptionHeight.toPixelSize(GetScale());
}

SNSEND
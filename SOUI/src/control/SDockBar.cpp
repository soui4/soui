#include "souistd.h"
#include "control/SDockBar.h"
#include "layout/SFrameLayout.h"

SNSBEGIN

SDockBar::SDockBar(void)
    : m_bCloseBtnHover(FALSE)
    , m_bCloseBtnPressed(FALSE)
    , m_nCaptionHeight(24, dp)
    , m_bActive(FALSE)
    , m_bResizable(FALSE)
    , m_nResizeHitTest(0)
    , m_bIsResizing(FALSE)
{
    m_bFocusable = TRUE;
}

SDockBar::~SDockBar(void)
{
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
}

void SDockBar::OnMouseMove(UINT nFlags, CPoint point)
{
    BOOL bNewHover = IsPointOnCloseBtn(point);
    if (bNewHover != m_bCloseBtnHover)
    {
        m_bCloseBtnHover = bNewHover;
        Invalidate();
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
}

void SDockBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __baseCls::OnShowWindow(bShow, nStatus);
    RequestRelayout();
}
void SDockBar::OnCloseBtnClick()
{
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
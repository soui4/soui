///////////////////////////////////////////////////////////////////////
/** Class Name: SwndContainerImpl */
///////////////////////////////////////////////////////////////////////
#include "souistd.h"
#include <core/SWndContainerImpl.h>

SNSBEGIN

#define WM_NCMOUSEFIRST WM_NCMOUSEMOVE
#define WM_NCMOUSELAST  WM_NCMBUTTONDBLCLK

///////////////////////////////////////////////////////////////////////
SwndContainerImpl::SwndContainerImpl()
    : m_hCapture(0)
    , m_hHover(0)
    , m_hTouchTarget(0)
    , m_hTouchInterceptor(0)
    , m_ptTouchDown(0, 0)
    , m_uNcHitTest(HTCLIENT)
    , m_bZorderDirty(TRUE)
    , m_pRoot(NULL)
{
    m_swndFinder.Attach(new SWindowFinder());
}

SwndContainerImpl::~SwndContainerImpl()
{
}
void SwndContainerImpl::SetRoot(SWindow *pRoot)
{
    m_pRoot = pRoot;
    m_dropTarget.SetOwner(pRoot);
    m_focusMgr.SetOwner(pRoot);
}

LRESULT SwndContainerImpl::DoFrameEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;
    SWindow *pRoot = m_pRoot; // use a local value to avoid m_pRoot changed while handle message
    pRoot->AddRef();
    pRoot->SetMsgHandled(TRUE);

    if (DispatchTouchEvent(uMsg, wParam, lParam))
    {
        pRoot->Release();
        return lRet;
    }

    switch (uMsg)
    {
    case WM_MOUSEMOVE:
        OnFrameMouseMove((UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
        break;
    case WM_MOUSEHOVER:
        OnFrameMouseEvent(uMsg, wParam, lParam);
        break;
    case WM_MOUSELEAVE:
        OnFrameMouseLeave();
        break;
    case WM_CANCELMODE:
        // DispatchTouchEvent() already delivered ACTION_CANCEL to the touch owner.
        break;
    case WM_SETCURSOR:
        lRet = OnFrameSetCursor(CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
        if (!lRet)
        {
            HCURSOR hCursor = GETRESPROVIDER->LoadCursor(IDC_ARROW);
            SetCursor(hCursor);
        }
        break;
    case WM_KEYDOWN:
        OnFrameKeyDown((UINT)wParam, (UINT)lParam & 0xFFFF, (UINT)((lParam & 0xFFFF0000) >> 16));
        break;
    case WM_SETFOCUS:
        OnActivate(WA_ACTIVE);
        break;
    case WM_KILLFOCUS:
        OnActivate(WA_INACTIVE);
        break;
    case WM_ACTIVATE:
        OnActivate(LOWORD(wParam));
        break;
    case WM_ACTIVATEAPP:
        OnActivateApp((BOOL)wParam, (DWORD)lParam);
        break;
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_COMPOSITION:
    case WM_IME_CHAR:
    case WM_IME_REQUEST:
        lRet = OnFrameKeyEvent(uMsg, wParam, lParam);
        break;
    case WM_MOUSEWHEEL:
    case 0x20E: // WM_MOUSEHWHEEL
        lRet = OnFrameMouseWheel(uMsg, wParam, lParam);
        break;
    default:
        if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
            OnFrameKeyEvent(uMsg, wParam, lParam);
        else if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
            OnFrameMouseEvent(uMsg, wParam, lParam);
        else
            pRoot->SetMsgHandled(FALSE);
        break;
    }

    pRoot->Release();
    return lRet;
}

BOOL SwndContainerImpl::OnReleaseSwndCapture()
{
    SWindow *pWnd = SWindowMgr::GetWindow(m_hCapture);
    if (pWnd)
    {
        pWnd->OnCaptureChanged(FALSE);
    }
    m_hCapture = 0;
    return TRUE;
}

SWND SwndContainerImpl::OnSetSwndCapture(SWND swnd)
{
    if (m_hCapture == swnd)
        return swnd;
    SWindow *pWnd = SWindowMgr::GetWindow(swnd);
    SASSERT(pWnd);
    if (pWnd->IsDisabled(TRUE))
        return 0;

    SWND hRet = m_hCapture;
    m_hCapture = swnd;
    pWnd->OnCaptureChanged(TRUE);
    return hRet;
}

void SwndContainerImpl::OnSetSwndFocus(SWND swnd)
{
    if (swnd && ::GetFocus() != GetHostHwnd() && !(::GetWindowLong(GetHostHwnd(), GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
    {
        ::SetFocus(GetHostHwnd());
    }
    m_focusMgr.SetFocusedHwnd(swnd);
}

SWND SwndContainerImpl::OnGetSwndCapture() const
{
    return m_hCapture;
}

SWND SwndContainerImpl::GetFocus() const
{
    return m_focusMgr.GetFocusedHwnd();
}

SWND SwndContainerImpl::GetHover() const
{
    return m_hHover;
}

void SwndContainerImpl::ResetTouchState()
{
    if (m_hTouchInterceptor && m_hCapture == m_hTouchInterceptor)
    {
        OnReleaseSwndCapture();
    }
    m_hTouchInterceptor = 0;
    m_hTouchTarget = 0;
    m_ptTouchDown = CPoint(0, 0);
}

BOOL SwndContainerImpl::IsDisallowIntercept(SWindow *pTarget) const
{
    // Equivalent of walking up ViewParent.requestDisallowInterceptTouchEvent().
    for (SWindow *p = pTarget; p; p = p->GetParent())
    {
        if (p->IsDisallowIntercept())
            return TRUE;
    }
    return FALSE;
}

BOOL SwndContainerImpl::DispatchTouchEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_pRoot)
        return FALSE;
    if (uMsg != WM_LBUTTONDOWN && uMsg != WM_MOUSEMOVE && uMsg != WM_LBUTTONUP && uMsg != WM_CANCELMODE)
        return FALSE;

    // Once a container has intercepted, it owns the whole remaining event stream.
    if (m_hTouchInterceptor)
    {
        SWindow *pInterceptor = SWindowMgr::GetWindow(m_hTouchInterceptor);
        if (!pInterceptor)
        {
            ResetTouchState();
            return FALSE;
        }
        pInterceptor->OnTouchEvent(uMsg, wParam, lParam);
        if (uMsg == WM_LBUTTONUP || uMsg == WM_CANCELMODE)
            ResetTouchState();
        return TRUE;
    }

    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    SWindow *pTarget = SWindowMgr::GetWindow(m_hCapture);

    if (uMsg == WM_LBUTTONDOWN)
    {
        // ACTION_DOWN is never intercepted: the child keeps its complete click stream.
        ResetTouchState();
        pTarget = SWindowMgr::GetWindow(m_pRoot->SwndFromPoint(pt));
        m_hTouchTarget = pTarget ? pTarget->GetSwnd() : 0;
        m_ptTouchDown = pt;
        // ViewGroup clears FLAG_DISALLOW_INTERCEPT on ACTION_DOWN.
        for (SWindow *p = pTarget; p; p = p->GetParent())
            p->RequestDisallowInterceptTouchEvent(FALSE);
        // Let every ancestor initialize its gesture detector, outermost first.
        SList<SWindow *> chain;
        for (SWindow *p = pTarget ? pTarget->GetParent() : NULL; p; p = p->GetParent())
            chain.AddHead(p);
        SPOSITION pos = chain.GetHeadPosition();
        while (pos)
            chain.GetNext(pos)->OnInterceptTouchEvent(uMsg, wParam, lParam);
        return FALSE;
    }

    if (!pTarget)
        pTarget = SWindowMgr::GetWindow(m_pRoot->SwndFromPoint(pt));

    if (uMsg == WM_LBUTTONUP || uMsg == WM_CANCELMODE)
    {
        if (uMsg == WM_CANCELMODE && pTarget)
            pTarget->SSendMessage(WM_CANCELMODE);
        ResetTouchState();
        return FALSE;
    }

    // WM_MOUSEMOVE: only while the button is held and a DOWN target exists.
    if (!(wParam & MK_LBUTTON) || !m_hTouchTarget || !pTarget)
        return FALSE;
    if (IsDisallowIntercept(pTarget))
        return FALSE;

    // Ask the innermost candidate first. Android reaches the same result by
    // letting an inner ViewPager call requestDisallowInterceptTouchEvent(true)
    // so that its parents stop asking; in SOUI the container owns the decision,
    // so walking outwards from the hit window gives the most local handler
    // priority. With the old outermost-first walk, two nested STabCtrls that
    // share an axis both cross touchSlop on the same MOVE and the outer one
    // always won (reproduced by the two tabAlign=top tabs in page_skia.xml).
    // pTarget itself is included so a tabctrl hit directly can also claim it.
    SList<SWindow *> chain;
    for (SWindow *p = pTarget; p; p = p->GetParent())
        chain.AddTail(p);

    SPOSITION pos = chain.GetHeadPosition();
    while (pos)
    {
        SWindow *pCandidate = chain.GetNext(pos);
        if (!pCandidate->OnInterceptTouchEvent(uMsg, wParam, lParam))
            continue;

        // Ask the child to give up its gesture. A FALSE reply means it refuses
        // (SPanel does this while it scrolls along the drag axis or is doing a
        // rubber-band selection), so the parent must not steal the event stream.
        // The reason carries the axis declared by the interceptor: a vertically
        // scrolling list must not veto a horizontal page switch of an outer tab.
        SWindow *pCancel = SWindowMgr::GetWindow(m_hCapture ? m_hCapture : m_hTouchTarget);
        if (pCancel && pCancel != pCandidate)
        {
            // Prefer the axis declared by the interceptor: a 45 degree or
            // zigzag drag has no reliable "dominant" direction, while the
            // candidate knows exactly which axis it will consume.
            int nReason = pCandidate->GetTouchInterceptAxis();
            if (!CANCEL_REASON_IS_SCROLL(nReason))
            {
                // Legacy fallback for interceptors that do not declare an axis.
                int nDx = pt.x - m_ptTouchDown.x;
                int nDy = pt.y - m_ptTouchDown.y;
                if (nDx < 0)
                    nDx = -nDx;
                if (nDy < 0)
                    nDy = -nDy;
                nReason = (nDx >= nDy) ? CANCEL_REASON_SCROLL_H : CANCEL_REASON_SCROLL_V;
            }
            if (!pCancel->CancelCaptureMode(nReason))
                return FALSE;
            pCancel->SSendMessage(WM_CANCELMODE);
        }

        OnFrameMouseLeave();
        pCandidate->SetCapture();
        m_hTouchInterceptor = pCandidate->GetSwnd();
        pCandidate->OnTouchEvent(uMsg, wParam, lParam);
        return TRUE;
    }
    return FALSE;
}

void SwndContainerImpl::OnFrameMouseMove(UINT uFlag, CPoint pt)
{
    // Handle the trackMouseEvent attribute
    SPOSITION pos = m_lstTrackMouseEvtWnd.GetHeadPosition();
    while (pos)
    {
        SWND swnd = m_lstTrackMouseEvtWnd.GetNext(pos);
        SWindow *pWnd = SWindowMgr::GetWindow(swnd);
        if (!pWnd)
        {
            UnregisterTrackMouseEvent(swnd);
        }
        else if (pWnd->IsVisible(TRUE))
        {
            CPoint pt4(pt);
            pWnd->TransformPointEx(pt4);
            BOOL bInWnd = pWnd->IsContainPoint(pt4, FALSE);
            if (bInWnd && !(pWnd->GetState() & WndState_Hover))
            {
                pWnd->SSendMessage(WM_MOUSEHOVER);
            }
            else if (!bInWnd && (pWnd->GetState() & WndState_Hover))
            {
                pWnd->SSendMessage(WM_MOUSELEAVE);
            }
        }
    }
    SWindow *pCapture = SWindowMgr::GetWindow(m_hCapture);
    if (pCapture)
    { // A window has set mouse capture; no need to check for the TrackMouseEvent attribute, nor to check changes between client and non-client areas
        pCapture->TransformPointEx(pt);
        SWindow *pHover = pCapture->IsContainPoint(pt, FALSE) ? pCapture : NULL;
        SWND hHover = pHover ? pHover->GetSwnd() : 0;
        if (hHover != m_hHover)
        { // Detect whether the mouse is moving between captured windows
            SWindow *pOldHover = SWindowMgr::GetWindow(m_hHover);
            m_hHover = hHover;
            if (pOldHover)
            {
                if (m_uNcHitTest != HTCLIENT)
                    pOldHover->SSendMessage(WM_NCMOUSELEAVE, m_uNcHitTest, 0);
                pOldHover->SSendMessage(WM_MOUSELEAVE);
            }
            if (pHover && !(pHover->GetState() & WndState_Hover))
            {
                if (m_uNcHitTest != HTCLIENT)
                    pHover->SSendMessage(WM_NCMOUSEHOVER, m_uNcHitTest, MAKELPARAM(pt.x, pt.y));
                pHover->SSendMessage(WM_MOUSEHOVER, uFlag, MAKELPARAM(pt.x, pt.y));
            }
        }
        pCapture->SSendMessage(m_uNcHitTest != HTCLIENT ? WM_NCMOUSEMOVE : WM_MOUSEMOVE, m_uNcHitTest != HTCLIENT ? m_uNcHitTest : uFlag, MAKELPARAM(pt.x, pt.y));
    }
    else
    { // No mouse capture is set
        CPoint pt2 = pt;
        SWND hHover = m_pRoot->SwndFromPoint(pt2);
        SWindow *pHover = SWindowMgr::GetWindow(hHover);
        if (m_hHover != hHover)
        { // The hover window has changed
            SWindow *pOldHover = SWindowMgr::GetWindow(m_hHover);
            m_hHover = hHover;
            if (pOldHover)
            {
                BOOL bLeave = TRUE;
                if (pOldHover->GetStyle().m_bTrackMouseEvent)
                { // Perform special handling for windows that monitor mouse events
                    CPoint pt3 = pt;
                    pOldHover->TransformPointEx(pt3);
                    bLeave = !pOldHover->IsContainPoint(pt3, FALSE);
                }
                if (bLeave && !pOldHover->IsDisabled(TRUE))
                {
                    if (m_uNcHitTest != HTCLIENT)
                        pOldHover->SSendMessage(WM_NCMOUSELEAVE, m_uNcHitTest, 0);
                    pOldHover->SSendMessage(WM_MOUSELEAVE);
                }
            }
            if (pHover && !pHover->IsDisabled(TRUE) && !(pHover->GetState() & WndState_Hover))
            {
                m_uNcHitTest = pHover->OnNcHitTest(pt2);
                if (m_uNcHitTest != HTCLIENT)
                    pHover->SSendMessage(WM_NCMOUSEHOVER, m_uNcHitTest, MAKELPARAM(pt2.x, pt2.y));
                pHover->SSendMessage(WM_MOUSEHOVER, uFlag, MAKELPARAM(pt2.x, pt2.y));
            }
        }
        else if (pHover && !pHover->IsDisabled(TRUE))
        { // Movement within the window; detect changes between client and non-client areas
            UINT uNcHitTest = pHover->OnNcHitTest(pt2);
            if (uNcHitTest != m_uNcHitTest)
            {
                m_uNcHitTest = uNcHitTest;
                if (m_uNcHitTest != HTCLIENT)
                {
                    pHover->SSendMessage(WM_NCMOUSEHOVER, m_uNcHitTest, MAKELPARAM(pt2.x, pt2.y));
                }
                else
                {
                    pHover->SSendMessage(WM_NCMOUSELEAVE, m_uNcHitTest, 0);
                }
            }
        }
        if (pHover && !pHover->IsDisabled(TRUE))
            pHover->SSendMessage(m_uNcHitTest != HTCLIENT ? WM_NCMOUSEMOVE : WM_MOUSEMOVE, m_uNcHitTest != HTCLIENT ? m_uNcHitTest : uFlag, MAKELPARAM(pt2.x, pt2.y));
    }
}

void SwndContainerImpl::OnFrameMouseLeave()
{
    SWindow *pCapture = SWindowMgr::GetWindow(m_hCapture);
    if (pCapture)
    {
        pCapture->SSendMessage(WM_MOUSELEAVE);
    }
    else if (m_hHover)
    {
        SWindow *pHover = SWindowMgr::GetWindow(m_hHover);
        if (pHover && !pHover->IsDisabled(TRUE))
        {
            pHover->AddRef();
            pHover->SSendMessage(WM_MOUSELEAVE);
            if (m_uNcHitTest != HTCLIENT)
                pHover->SSendMessage(WM_NCMOUSELEAVE, m_uNcHitTest, 0);
            pHover->Release();
        }
    }

    // Handle the trackMouseEvent attribute
    SPOSITION pos = m_lstTrackMouseEvtWnd.GetHeadPosition();
    while (pos)
    {
        SWND swnd = m_lstTrackMouseEvtWnd.GetNext(pos);
        SWindow *pWnd = SWindowMgr::GetWindow(swnd);
        if (!pWnd)
        {
            UnregisterTrackMouseEvent(swnd);
        }
        else if (pWnd->IsVisible(TRUE) && pWnd->GetState() & WndState_Hover)
        {
            pWnd->SSendMessage(WM_MOUSELEAVE);
        }
    }
    m_hHover = 0;
}

BOOL SwndContainerImpl::OnFrameSetCursor(const CPoint &pt)
{
    SWindow *pCapture = SWindowMgr::GetWindow(m_hCapture);
    if (pCapture)
        return pCapture->OnSetCursor(pt);
    else
    {
        SWindow *pHover = SWindowMgr::GetWindow(m_hHover);
        if (pHover && !pHover->IsDisabled(TRUE))
            return pHover->OnSetCursor(pt);
    }
    return FALSE;
}

void SwndContainerImpl::OnFrameMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SWindow *pCapture = SWindowMgr::GetWindow(m_hCapture);
    if (pCapture)
    {
        if (m_uNcHitTest != HTCLIENT)
        {
            uMsg += (UINT)WM_NCMOUSEFIRST - WM_MOUSEFIRST; // Convert to the corresponding NC message
            wParam = m_uNcHitTest;                         // Use m_uNcHitTest as wparam
        }
        BOOL bMsgHandled = FALSE;
        CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        pCapture->TransformPointEx(pt);
        lParam = MAKELPARAM(pt.x, pt.y);
        pCapture->SSendMessage(uMsg, wParam, lParam, &bMsgHandled);
        m_pRoot->SetMsgHandled(bMsgHandled);
    }
    else
    {
        CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        m_hHover = m_pRoot->SwndFromPoint(pt);
        SWindow *pHover = SWindowMgr::GetWindow(m_hHover);
        if (pHover && !pHover->IsDisabled(TRUE))
        {
            BOOL bMsgHandled = FALSE;
            if (m_uNcHitTest != HTCLIENT)
            {
                uMsg += (UINT)WM_NCMOUSEFIRST - WM_MOUSEFIRST; // Convert to the corresponding NC message
                wParam = m_uNcHitTest;                         // Use m_uNcHitTest as wparam
            }
            lParam = MAKELPARAM(pt.x, pt.y);
            pHover->SSendMessage(uMsg, wParam, lParam, &bMsgHandled);
            m_pRoot->SetMsgHandled(bMsgHandled);
        }
        else
        {
            m_pRoot->SetMsgHandled(FALSE);
        }
    }
}

LRESULT SwndContainerImpl::OnFrameMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 返回值 = 目标窗口的 OnMouseWheel 是否真的消费了这一格滚轮。宿主（如 wasm 页面）
    // 用它决定要不要把滚轮继续交给外部环境处理，所以不能像以前那样固定返回 0。
    LRESULT lRet = 0;
    SWindow *pWndTarget = SWindowMgr::GetWindow(m_hCapture);
    if (!pWndTarget)
    {
        if (IsSendWheel2Hover())
        {
            CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            m_hHover = m_pRoot->SwndFromPoint(pt);
            pWndTarget = SWindowMgr::GetWindow(m_hHover);
        }
        else
        {
            pWndTarget = SWindowMgr::GetWindow(m_focusMgr.GetFocusedHwnd());
        }
    }

    if (pWndTarget && !pWndTarget->IsDisabled(TRUE))
    {
        BOOL bMsgHandled = FALSE;
        lRet = pWndTarget->SSendMessage(uMsg, wParam, lParam, &bMsgHandled);
        m_pRoot->SetMsgHandled(bMsgHandled);
    }
    else
    {
        m_pRoot->SetMsgHandled(FALSE);
    }
    return lRet;
}

LRESULT SwndContainerImpl::OnFrameKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;
    if ((uMsg == WM_KEYDOWN || uMsg == WM_KEYUP) && GetKeyState(VK_MENU) & 0x80)
    {
        UINT vKey = (UINT)wParam;
        if (vKey >= 'a' && vKey <= 'z')
            vKey -= 0x20; // Convert to VK
        if (m_focusMgr.OnKeyDown(vKey))
            return lRet; // Handle focus switching first
    }

    SWindow *pFocus = SWindowMgr::GetWindow(m_focusMgr.GetFocusedHwnd());
    if (pFocus)
    {
        BOOL bMsgHandled = FALSE;
        lRet = pFocus->SSendMessage(uMsg, wParam, lParam, &bMsgHandled);
        m_pRoot->SetMsgHandled(bMsgHandled);
    }
    else
    {
        m_pRoot->SetMsgHandled(FALSE);
    }
    return lRet;
}

void SwndContainerImpl::OnFrameKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (m_focusMgr.OnKeyDown(nChar))
        return; // Handle focus switching first

    SWindow *pFocus = SWindowMgr::GetWindow(m_focusMgr.GetFocusedHwnd());
    if (pFocus)
    {
        BOOL bMsgHandled = FALSE;
        pFocus->SSendMessage(WM_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags), &bMsgHandled);
        m_pRoot->SetMsgHandled(bMsgHandled);
    }
    else
    {
        m_pRoot->SetMsgHandled(FALSE);
    }
}

BOOL SwndContainerImpl::RegisterDragDrop(SWND swnd, IDropTarget *pDropTarget)
{
    return m_dropTarget.RegisterDragDrop(swnd, pDropTarget);
}

BOOL SwndContainerImpl::UnregisterDragDrop(SWND swnd)
{
    return m_dropTarget.UnregisterDragDrop(swnd);
}

void SwndContainerImpl::OnActivate(UINT nState)
{
    if (nState == WA_INACTIVE)
    {
        m_focusMgr.StoreFocusedView();
    }
    else if (nState == WA_ACTIVE)
    {
        m_focusMgr.RestoreFocusedView();
    }
}

void SwndContainerImpl::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
    m_pRoot->SDispatchMessage(WM_ACTIVATEAPP, (WPARAM)bActive, (LPARAM)dwThreadID);
}

BOOL SwndContainerImpl::RegisterTrackMouseEvent(SWND swnd)
{
    if (m_lstTrackMouseEvtWnd.Find(swnd))
        return FALSE;
    m_lstTrackMouseEvtWnd.AddTail(swnd);
    return TRUE;
}

BOOL SwndContainerImpl::UnregisterTrackMouseEvent(SWND swnd)
{
    SPOSITION pos = m_lstTrackMouseEvtWnd.Find(swnd);
    if (!pos)
        return FALSE;
    m_lstTrackMouseEvtWnd.RemoveAt(pos);
    return TRUE;
}

void SwndContainerImpl::MarkWndTreeZorderDirty()
{
    m_bZorderDirty = TRUE;
}

void SwndContainerImpl::BuildWndTreeZorder()
{
    if (m_bZorderDirty)
    {
        if (m_pRoot)
            m_pRoot->OnBuildTreeZorder(0);
        m_bZorderDirty = FALSE;
    }
}

BOOL SwndContainerImpl::RegisterTimelineHandler(ITimelineHandler *pHandler)
{
    bool bEmpty1 = m_timelineHandlerMgr.IsEmpty();
    BOOL bRet = m_timelineHandlerMgr.RegisterTimelineHandler(pHandler);
    bool bEmpty2 = m_timelineHandlerMgr.IsEmpty();
    if (bEmpty1 && !bEmpty2)
        OnTimelineRequestChanged(TRUE);
    return bRet;
}

BOOL SwndContainerImpl::UnregisterTimelineHandler(ITimelineHandler *pHandler)
{
    bool bEmpty1 = m_timelineHandlerMgr.IsEmpty();
    BOOL bRet = m_timelineHandlerMgr.UnregisterTimelineHandler(pHandler);
    bool bEmpty2 = m_timelineHandlerMgr.IsEmpty();
    if (!bEmpty1 && bEmpty2)
        OnTimelineRequestChanged(FALSE);
    return bRet;
}

BOOL SwndContainerImpl::RegisterValueAnimator(IValueAnimator *pAnimator)
{
    bool bEmpty1 = m_timelineHandlerMgr.IsEmpty();
    BOOL bRet = m_timelineHandlerMgr.RegisterValueAnimator(pAnimator);
    bool bEmpty2 = m_timelineHandlerMgr.IsEmpty();
    if (bEmpty1 && !bEmpty2)
        OnTimelineRequestChanged(TRUE);
    return bRet;
}

BOOL SwndContainerImpl::UnregisterValueAnimator(IValueAnimator *pAnimator)
{
    bool bEmpty1 = m_timelineHandlerMgr.IsEmpty();
    BOOL bRet = m_timelineHandlerMgr.UnregisterValueAnimator(pAnimator);
    bool bEmpty2 = m_timelineHandlerMgr.IsEmpty();
    if (!bEmpty1 && bEmpty2)
        OnTimelineRequestChanged(FALSE);
    return bRet;
}

bool SwndContainerImpl::IsTimeLineMgrEmpty() const
{
    return m_timelineHandlerMgr.IsEmpty();
}

void SwndContainerImpl::OnNextFrame()
{
    if (!m_pRoot->IsVisible(FALSE))
        return;
    if (!IsTimelineEnabled())
        return;
    m_timelineHandlerMgr.OnNextFrame();
}

BOOL SwndContainerImpl::RegisterVideoCanvas(SWND swnd)
{
    SPOSITION pos = m_lstVideoCanvas.Find(swnd);
    if (pos)
        return FALSE;
    m_lstVideoCanvas.AddTail(swnd);
    return TRUE;
}

BOOL SwndContainerImpl::UnregisterVideoCanvas(SWND swnd)
{
    SPOSITION pos = m_lstVideoCanvas.Find(swnd);
    if (!pos)
        return FALSE;

    m_lstVideoCanvas.RemoveAt(pos);
    return TRUE;
}

BOOL SwndContainerImpl::IsTimelineEnabled() const
{
    return TRUE;
}

BOOL SwndContainerImpl::IsDesignerMode() const
{
    return FALSE;
}

SNSEND

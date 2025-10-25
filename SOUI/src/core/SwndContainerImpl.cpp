﻿//////////////////////////////////////////////////////////////////////////
//  Class Name: SwndContainerImpl
//////////////////////////////////////////////////////////////////////////
#include "souistd.h"
#include <core/SWndContainerImpl.h>

SNSBEGIN

#define WM_NCMOUSEFIRST WM_NCMOUSEMOVE
#define WM_NCMOUSELAST  WM_NCMBUTTONDBLCLK

//////////////////////////////////////////////////////////////////////////
SwndContainerImpl::SwndContainerImpl()
    : m_hCapture(0)
    , m_hHover(0)
    , m_bNcHover(FALSE)
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
        OnFrameMouseWheel(uMsg, wParam, lParam);
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

void SwndContainerImpl::OnFrameMouseMove(UINT uFlag, CPoint pt)
{
    SWindow *pCapture = SWindowMgr::GetWindow(m_hCapture);
    if (pCapture)
    { //有窗口设置了鼠标捕获,不需要判断是否有TrackMouseEvent属性,也不需要判断客户区与非客户区的变化
        pCapture->TransformPointEx(pt);
        SWindow *pHover = pCapture->IsContainPoint(pt, FALSE) ? pCapture : NULL;
        SWND hHover = pHover ? pHover->GetSwnd() : 0;
        if (hHover != m_hHover)
        { //检测鼠标是否在捕获窗口间移动
            SWindow *pOldHover = SWindowMgr::GetWindow(m_hHover);
            m_hHover = hHover;
            if (pOldHover)
            {
                if (m_bNcHover)
                    pOldHover->SSendMessage(WM_NCMOUSELEAVE);
                pOldHover->SSendMessage(WM_MOUSELEAVE);
            }
            if (pHover && !(pHover->GetState() & WndState_Hover))
            {
                if (m_bNcHover)
                    pHover->SSendMessage(WM_NCMOUSEHOVER, uFlag, MAKELPARAM(pt.x, pt.y));
                pHover->SSendMessage(WM_MOUSEHOVER, uFlag, MAKELPARAM(pt.x, pt.y));
            }
        }
        pCapture->SSendMessage(m_bNcHover ? WM_NCMOUSEMOVE : WM_MOUSEMOVE, uFlag, MAKELPARAM(pt.x, pt.y));
    }
    else
    { //没有设置鼠标捕获
        CPoint pt2 = pt;
        SWND hHover = m_pRoot->SwndFromPoint(pt2);
        SWindow *pHover = SWindowMgr::GetWindow(hHover);
        if (m_hHover != hHover)
        { // hover窗口发生了变化
            SWindow *pOldHover = SWindowMgr::GetWindow(m_hHover);
            m_hHover = hHover;
            if (pOldHover)
            {
                BOOL bLeave = TRUE;
                if (pOldHover->GetStyle().m_bTrackMouseEvent)
                { //对于有监视鼠标事件的窗口做特殊处理
                    CPoint pt3 = pt;
                    pOldHover->TransformPointEx(pt3);
                    bLeave = !pOldHover->IsContainPoint(pt3, FALSE);
                }
                if (bLeave)
                {
                    if (m_bNcHover)
                        pOldHover->SSendMessage(WM_NCMOUSELEAVE);
                    pOldHover->SSendMessage(WM_MOUSELEAVE);
                }
            }
            if (pHover && !pHover->IsDisabled(TRUE) && !(pHover->GetState() & WndState_Hover))
            {
                m_bNcHover = pHover->OnNcHitTest(pt2);
                if (m_bNcHover)
                    pHover->SSendMessage(WM_NCMOUSEHOVER, uFlag, MAKELPARAM(pt2.x, pt2.y));
                pHover->SSendMessage(WM_MOUSEHOVER, uFlag, MAKELPARAM(pt2.x, pt2.y));
            }
        }
        else if (pHover && !pHover->IsDisabled(TRUE))
        { //窗口内移动，检测客户区和非客户区的变化
            BOOL bNcHover = pHover->OnNcHitTest(pt2);
            if (bNcHover != m_bNcHover)
            {
                m_bNcHover = bNcHover;
                if (m_bNcHover)
                {
                    pHover->SSendMessage(WM_NCMOUSEHOVER, uFlag, MAKELPARAM(pt2.x, pt2.y));
                }
                else
                {
                    pHover->SSendMessage(WM_NCMOUSELEAVE);
                }
            }
        }
        if (pHover && !pHover->IsDisabled(TRUE))
            pHover->SSendMessage(m_bNcHover ? WM_NCMOUSEMOVE : WM_MOUSEMOVE, uFlag, MAKELPARAM(pt2.x, pt2.y));
    }

    //处理trackMouseEvent属性
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
            if (m_bNcHover)
                pHover->SSendMessage(WM_NCMOUSELEAVE);
            pHover->Release();
        }
    }

    //处理trackMouseEvent属性
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
        if (m_bNcHover)
            uMsg += (UINT)WM_NCMOUSEFIRST - WM_MOUSEFIRST; //转换成NC对应的消息
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
            if (m_bNcHover)
                uMsg += (UINT)WM_NCMOUSEFIRST - WM_MOUSEFIRST; //转换成NC对应的消息
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

void SwndContainerImpl::OnFrameMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
        pWndTarget->SSendMessage(uMsg, wParam, lParam, &bMsgHandled);
        m_pRoot->SetMsgHandled(bMsgHandled);
    }
    else
    {
        m_pRoot->SetMsgHandled(FALSE);
    }
}

LRESULT SwndContainerImpl::OnFrameKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;
    if ((uMsg == WM_KEYDOWN || uMsg == WM_KEYUP) && GetKeyState(VK_MENU) & 0x80)
    {
        UINT vKey = (UINT)wParam;
        if (vKey >= 'a' && vKey <= 'z')
            vKey -= 0x20; //转换成VK
        if (m_focusMgr.OnKeyDown(vKey))
            return lRet; //首先处理焦点切换
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
        return; //首先处理焦点切换

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
        UINT uInitZorder = 0;
        _BuildWndTreeZorder(m_pRoot, uInitZorder);
        m_bZorderDirty = FALSE;
    }
}

void SwndContainerImpl::_BuildWndTreeZorder(IWindow *pWnd, UINT &iOrder)
{
    ((SWindow *)pWnd)->m_uZorder = iOrder++;
    IWindow *pChild = pWnd->GetIWindow(GSW_FIRSTCHILD);
    while (pChild)
    {
        _BuildWndTreeZorder(pChild, iOrder);
        pChild = pChild->GetIWindow(GSW_NEXTSIBLING);
    }
}

BOOL SwndContainerImpl::RegisterTimelineHandler(ITimelineHandler *pHandler)
{
    return m_timelineHandlerMgr.RegisterTimelineHandler(pHandler);
}

BOOL SwndContainerImpl::UnregisterTimelineHandler(ITimelineHandler *pHandler)
{
    return m_timelineHandlerMgr.UnregisterTimelineHandler(pHandler);
}

void SwndContainerImpl::OnNextFrame()
{
    if (!m_pRoot->IsVisible(FALSE))
        return;
    if(!IsTimelineEnabled())
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

SNSEND
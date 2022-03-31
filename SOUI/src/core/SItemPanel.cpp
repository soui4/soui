//////////////////////////////////////////////////////////////////////////
//  Class Name: SOsrPanel
//     Creator: Huang Jianxiong
//     Version: 2011.10.20 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "souistd.h"
#include "core/SItemPanel.h"

#pragma warning(disable : 4018)

SNSBEGIN

SOsrPanel::SOsrPanel(IHostProxy *pFrameHost, IItemContainer *pItemContainer)
    : m_pHostProxy(pFrameHost)
    , m_pItemContainer(pItemContainer)
	, m_dwData(0)
	, m_lpItemIndex(-1)
{
    SetContainer(this);
    SwndContainerImpl::SetRoot(this);
    SASSERT(m_pHostProxy);
    SASSERT(m_pItemContainer);
    m_evtSet.addEvent(EVENTID(EventItemPanelClick));
    m_evtSet.addEvent(EVENTID(EventItemPanelRclick));
    m_evtSet.addEvent(EVENTID(EventItemPanelClickUp));
    m_evtSet.addEvent(EVENTID(EventItemPanelRclickUp));
    m_evtSet.addEvent(EVENTID(EventItemPanelDbclick));
    m_evtSet.addEvent(EVENTID(EventItemPanelHover));
    m_evtSet.addEvent(EVENTID(EventItemPanelLeave));
}

void SOsrPanel::OnFinalRelease()
{
    AddRef(); //防止重复进入该函数
    SSendMessage(WM_DESTROY);
    __baseCls::OnFinalRelease();
}

BOOL SOsrPanel::InitFromXml(THIS_ IXmlNode *pNode)
{
	BOOL bRet = __baseCls::InitFromXml(pNode);
	if(bRet)
	{
		BuildWndTreeZorder();
	}
	return bRet;
}

LRESULT SOsrPanel::DoFrameEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    AddRef();

    if (!IsDisabled())
    {
        switch (uMsg)
        {
        case WM_MOUSEHOVER:
        {
            ModifyState(WndState_Hover, 0, TRUE);
            EventItemPanelHover evt(this);
            evt.wParam = wParam;
            evt.lParam = lParam;
            FireEvent(evt);
            break;
        }
        case WM_MOUSELEAVE:
        {
            ModifyState(0, WndState_Hover, TRUE);
            EventItemPanelLeave evt(this);
            FireEvent(evt);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            EventItemPanelClick evt(this);
            evt.wParam = wParam;
            evt.lParam = lParam;
            FireEvent(evt);
            break;
        }
        case WM_RBUTTONDOWN:
        {
            EventItemPanelRclick evt(this);
            evt.wParam = wParam;
            evt.lParam = lParam;
            FireEvent(evt);
            break;
        }
        case WM_LBUTTONDBLCLK:
        {
            EventItemPanelDbclick evt(this);
            evt.wParam = wParam;
            evt.lParam = lParam;
            FireEvent(evt);
            break;
        }
        case WM_LBUTTONUP:
        {
            EventItemPanelClickUp evt(this);
            evt.wParam = wParam;
            evt.lParam = lParam;
            FireEvent(evt);
            break;
        }
        case WM_RBUTTONUP:
        {
            EventItemPanelRclickUp evt(this);
            evt.wParam = wParam;
            evt.lParam = lParam;
            FireEvent(evt);
            break;
        }
        }
    }

    SetMsgHandled(FALSE);

    BOOL isMute = GetEventSet()->isMuted();
    if (uMsg == WM_LBUTTONUP && !isMute)
    {
        GetEventSet()->setMutedState(true);
    }
    LRESULT lRet = SwndContainerImpl::DoFrameEvent(uMsg, wParam, lParam);
    if (uMsg == WM_LBUTTONUP && !isMute)
    {
        GetEventSet()->setMutedState(false);
        FireCommand();
    }

    Release();
    return lRet;
}

BOOL SOsrPanel::OnFireEvent(IEvtArgs *evt)
{
    return m_pHostProxy->OnHostFireEvent(evt);
}

RECT SOsrPanel::GetContainerRect() const
{
    CRect rcItem;
    m_pItemContainer->OnItemGetRect(this, rcItem);
    return rcItem;
}

IRenderTarget *SOsrPanel::OnGetRenderTarget(LPCRECT rc, GrtFlag gdcFlags)
{
	IRenderTarget *pRT = NULL;
	if (m_pItemContainer->IsItemRedrawDelay() || gdcFlags == GRT_NODRAW)
	{
		GETRENDERFACTORY->CreateRenderTarget(&pRT, 0,0);
		return pRT;
	}

	CRect rcHost(rc);
	FrameToHost(&rcHost);
	pRT = m_pHostProxy->OnGetHostRenderTarget(&rcHost,GRT_PAINTBKGND);
	CRect rcItem=GetItemRect();
	pRT->OffsetViewportOrg(rcItem.left,rcItem.top,NULL);
	pRT->PushClipRect(rc,RGN_AND);

    return pRT;
}

void SOsrPanel::OnReleaseRenderTarget(IRenderTarget *pRT, LPCRECT rc, GrtFlag gdcFlags)
{
	if (m_pItemContainer->IsItemRedrawDelay()||gdcFlags == GRT_NODRAW)
	{
		pRT->Release();
		return;
	}
	pRT->PopClip();

	CRect rcHost(rc);
	FrameToHost(&rcHost);
	CRect rcItem=GetItemRect();
	pRT->OffsetViewportOrg(-rcItem.left,-rcItem.top,NULL);
	m_pHostProxy->OnReleaseHostRenderTarget(pRT,&rcHost,GRT_PAINTBKGND);
}

void SOsrPanel::OnRedraw(LPCRECT rc)
{
    if (m_pHostProxy->IsHostUpdateLocked())
        return;

    CRect rcItem = GetItemRect();
    if (!rcItem.IsRectNull() && m_pHostProxy->IsHostVisible())
    {
        if (m_pItemContainer->IsItemRedrawDelay())
        {
            CRect rc2(rc);
            rc2.OffsetRect(rcItem.TopLeft());
            rc2.IntersectRect(rc2, rcItem);
            CRect rcHostClient = m_pHostProxy->GetHostRect();
            rc2.IntersectRect(rc2, rcHostClient);
            m_pHostProxy->InvalidateHostRect(&rc2);
        }
        else
        {
            IRenderTarget *pRT = OnGetRenderTarget(rc, GRT_PAINTBKGND);
            SAutoRefPtr<IRegionS> rgn;
            GETRENDERFACTORY->CreateRegion(&rgn);
            rgn->CombineRect(rc, RGN_COPY);
            RedrawRegion(pRT, rgn);
            OnReleaseRenderTarget(pRT, rc, GRT_PAINTBKGND);
        }
    }
}

BOOL SOsrPanel::OnReleaseSwndCapture()
{
    if (!SwndContainerImpl::OnReleaseSwndCapture())
        return FALSE;
    m_pItemContainer->OnItemSetCapture(this, FALSE);
    return TRUE;
}

SWND SOsrPanel::OnSetSwndCapture(SWND swnd)
{
    m_pItemContainer->OnItemSetCapture(this, TRUE);
    return SwndContainerImpl::OnSetSwndCapture(swnd);
}

HWND SOsrPanel::GetHostHwnd()
{
    return m_pHostProxy->GetHostContainer()->GetHostHwnd();
}

LPCWSTR SOsrPanel::GetTranslatorContext() const
{
    return m_pHostProxy->GetHostContainer()->GetTranslatorContext();
}

BOOL SOsrPanel::IsTranslucent() const
{
    return m_pHostProxy->GetHostContainer()->IsTranslucent();
}

BOOL SOsrPanel::IsSendWheel2Hover() const
{
    return m_pHostProxy->GetHostContainer()->IsSendWheel2Hover();
}

BOOL SOsrPanel::UpdateWindow()
{
    return m_pHostProxy->GetHostContainer()->UpdateWindow();
}

void SOsrPanel::UpdateTooltip()
{
    return m_pHostProxy->GetHostContainer()->UpdateTooltip();
}

void SOsrPanel::ModifyItemState(DWORD dwStateAdd, DWORD dwStateRemove)
{
    ModifyState(dwStateAdd, dwStateRemove, FALSE);
}

SWND SOsrPanel::SwndFromPoint(CPoint &pt, bool bIncludeMsgTransparent)
{
    SWND hRet = SWindow::SwndFromPoint(pt, bIncludeMsgTransparent);
    if (hRet == m_swnd)
        hRet = NULL;
    return hRet;
}

void SOsrPanel::Draw(IRenderTarget *pRT, const CRect &rc)
{
    UpdateLayout();
    BuildWndTreeZorder();

    float fMat[9];
    pRT->GetTransform(fMat);
    SMatrix mtx(fMat);
    if (mtx.isIdentity())
    {
        SPainter painter;
        BeforePaint(pRT, painter);
        pRT->OffsetViewportOrg(rc.left, rc.top, NULL);
        SAutoRefPtr<IRegionS> rgn;
        pRT->GetClipRegion(&rgn);
        RedrawRegion(pRT, rgn);
        pRT->OffsetViewportOrg(-rc.left, -rc.top, NULL);
        AfterPaint(pRT, painter);
    }
    else
    { // draw to cache
        IRenderTarget *pMemRT = NULL;
        CRect rcMem = rc;
        rcMem.MoveToXY(0, 0);
        GETRENDERFACTORY->CreateRenderTarget(&pMemRT, rc.Width(), rc.Height());
        pMemRT->AlphaBlend(rcMem, pRT, rc, 255);
        SPainter painter;
        BeforePaint(pMemRT, painter);
        RedrawRegion(pMemRT, NULL);
        AfterPaint(pMemRT, painter);
        pRT->AlphaBlend(rc, pMemRT, rcMem, 255);
        pMemRT->Release();
    }
}


BOOL SOsrPanel::NeedRedrawWhenStateChange()
{
    return TRUE;
}

CRect SOsrPanel::GetItemRect() const
{
    CRect rcItem;
    m_pItemContainer->OnItemGetRect(this, rcItem);
    return rcItem;
}
void SOsrPanel::SetItemCapture(BOOL bCapture)
{
    m_pItemContainer->OnItemSetCapture(this, bCapture);
}


BOOL SOsrPanel::UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    CRect rcItem = GetItemRect();
    if (m_hHover == m_swnd)
    {
        tipInfo.swnd = m_swnd;
        tipInfo.dwCookie = 0;
        tipInfo.rcTarget = rcItem;
        tipInfo.strTip = GetToolTipText();
        return TRUE;
    }

    SWindow *pHover = SWindowMgr::GetWindow(m_hHover);
    if (!pHover)
    { // disabled窗口也可以有tooltip
        tipInfo.swnd = 0;
        return TRUE;
    }

    pt -= rcItem.TopLeft();
    BOOL bRet = pHover->UpdateToolTip(pt, tipInfo);
    if (bRet)
    {
        tipInfo.rcTarget.OffsetRect(rcItem.TopLeft());
    }
    return bRet;
}

IMessageLoop *SOsrPanel::GetMsgLoop()
{
    return m_pHostProxy->GetHostContainer()->GetMsgLoop();
}

IScriptModule *SOsrPanel::GetScriptModule()
{
    return m_pHostProxy->GetHostContainer()->GetScriptModule();
}

void SOsrPanel::FrameToHost(RECT *rc) const
{
    CRect rcItem = GetItemRect();
    ::OffsetRect(rc, rcItem.left, rcItem.top);
}


//不继承宿主的字体，从指定的字体或者系统字体开始，避免在GetRenderTarget时还需要从宿主窗口到获取当前的文字属性。
void SOsrPanel::BeforePaint(IRenderTarget *pRT, SPainter &painter)
{
    int iState = SState2Index::GetDefIndex(GetState(), true);
    IFontPtr fontText = GetStyle().GetTextFont(iState);
    COLORREF crText = GetStyle().GetTextColor(iState);
    if (fontText == NULL)
        fontText = SFontPool::getSingleton().GetFont(FF_DEFAULTFONT, GetScale());
    if (crText == CR_INVALID)
        crText = RGBA(0, 0, 0, 255);
    pRT->SelectObject(fontText, (IRenderObj **)&painter.oldFont);
    painter.oldTextColor = pRT->SetTextColor(crText);
}

void SOsrPanel::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __baseCls::OnShowWindow(bShow, nStatus);
    if (IsVisible(TRUE))
        m_pHostProxy->GetHostContainer()->RegisterTimelineHandler(this);
    else
        m_pHostProxy->GetHostContainer()->UnregisterTimelineHandler(this);
}

void SOsrPanel::OnDestroy()
{
    m_pHostProxy->GetHostContainer()->UnregisterTimelineHandler(this);
    __baseCls::OnDestroy();
}

int SOsrPanel::GetScale() const
{
    return m_pHostProxy->GetHostContainer()->GetScale();
}

void SOsrPanel::EnableIME(BOOL bEnable)
{
    m_pHostProxy->GetHostContainer()->EnableIME(bEnable);
}

void SOsrPanel::OnUpdateCursor()
{
    m_pHostProxy->GetHostContainer()->OnUpdateCursor();
}


BOOL SOsrPanel::IsItemInClip(const SMatrix &mtx,
                              const CRect &rcClip,
                              const IRegionS *clipRgn,
                              const CRect &rcItem)
{
    if (!mtx.isIdentity()) // don't clip any item if matrix is not identify.
        return TRUE;
    CRect rc;
    rc.IntersectRect(rcClip, rcItem);
    return !rc.IsRectEmpty() && (!clipRgn || clipRgn->RectInRegion(rcItem));
}

LPARAM SOsrPanel::GetItemIndex() const
{
	return m_lpItemIndex;
}

void SOsrPanel::SetItemIndex(LPARAM lp)
{
	m_lpItemIndex = lp;
}

void SOsrPanel::SetItemData(LPARAM dwData)
{
	m_dwData = dwData;
}

LPARAM SOsrPanel::GetItemData() const
{
	return m_dwData;
}




//////////////////////////////////////////////////////////////////////////
SItemPanel *SItemPanel::Create(IHostProxy  *pFrameHost,
							 SXmlNode xmlNode,
							 IItemContainer *pItemContainer)
{
	SItemPanel *pItem = new SItemPanel(pFrameHost, pItemContainer);
	pItem->InitFromXml(&xmlNode);
	SApplication::getSingletonPtr()->SetSwndDefAttr(pItem);
	return pItem;
}


SItemPanel::SItemPanel(IHostProxy *pFrameHost, IItemContainer *pItemContainer)
:TOsrPanelProxy<IItemPanel>(pFrameHost,pItemContainer)
, m_crBk(CR_INVALID)
, m_crHover(CR_INVALID)
, m_crSelBk(RGBA(0, 0, 128, 255))
{

}

void SItemPanel::SetSkin(ISkinObj *pSkin)
{
	m_pBgSkin = pSkin;
}

void SItemPanel::SetColor(COLORREF crBk, COLORREF crSelBk)
{
	m_crBk = crBk;
	m_crSelBk = crSelBk;
}

COLORREF SItemPanel::GetBkgndColor() const
{
	if ((m_dwState & WndState_Check) && m_crSelBk != CR_INVALID)
	{
		return m_crSelBk;
	}
	if ((m_dwState & WndState_Hover) && m_crHover != CR_INVALID)
	{
		return m_crHover;
	}
	return m_crBk;
}

BOOL SItemPanel::OnFireEvent(IEvtArgs *evt)
{
	EventOfPanel evt2(m_pHostProxy->GetHost());
	evt2.pPanel = this;
	evt2.pOrgEvt = evt;
	return m_pHostProxy->OnHostFireEvent(&evt2);
}


SNSEND
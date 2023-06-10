#include <souistd.h>
#include <Control/SStackView.h>

SNSBEGIN


SStackView::SStackView(void):m_iSel(-2),m_animator(this),m_aniStyle(kAniNone),m_bVertAni(FALSE)
{
	m_bClipClient = true;
	m_animator.addListener(this);
}

SStackView::~SStackView(void)
{
}

BOOL SStackView::SelectPage(int iSel,BOOL enableAnimate)
{
	if(iSel<-2 || iSel>=(int)GetChildrenCount())
	{
		return FALSE;
	}
	if(m_animator.isStarted())
		m_animator.end();
	if(iSel == m_iSel)
		return TRUE;
	if(!IsVisible(TRUE))
		enableAnimate=FALSE;
	if(m_iSel == -2){
		for(UINT i=0;i<GetChildrenCount();i++){
			IWindow *pChild = GetPage(i);
			pChild->SetVisible(FALSE,FALSE);
		}
		Invalidate();
	}
	m_animator.SetFrom(m_iSel);
	m_animator.SetTo(iSel);
	m_iSel = iSel;
	if(m_iSel!=-2){
		m_animator.start(GetContainer());
		if(!enableAnimate){
			m_animator.end();
		}
	}
	return TRUE;
}

IWindow* SStackView::GetSelPage(CTHIS) const
{
	if(m_iSel<0)
		return NULL;
	return GetIChild(m_iSel);
}

IWindow* SStackView::GetPage(CTHIS_ int iPage) const
{
	if(iPage<0 || iPage>=(int)m_childs.GetCount())
		return NULL;
	return m_childs[iPage];
}

void SStackView::UpdateChildrenPosition(THIS)
{
	if(m_animator.isStarted())
		m_animator.end();
	//note: childs array is not ready now!
	RECT rc;
	GetChildrenLayoutRect(&rc);
	SWindow* pChild = GetWindow(GSW_FIRSTCHILD);
	while (pChild) {
		pChild->Move(&rc);
		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
	}
}

void SStackView::OnDestroy()
{
	m_animator.end();
	__baseCls::OnDestroy();
}

void SStackView::OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer)
{
	m_animator.end();
	__baseCls::OnContainerChanged(pOldContainer,pNewContainer);
}

void SStackView::OnSize(UINT nType, CSize size)
{
	__baseCls::OnSize(nType,size);
	m_animator.OnOwnerResize();
}

void SStackView::onAnimationEnd(THIS_ IValueAnimator * pAnimator)
{
	SASSERT(m_iSel!=-2);
	IWindow * pFrom = GetPage(m_animator.GetFrom());
	if(pFrom) pFrom->SetVisible(FALSE,TRUE);
}

void SStackView::SetAniStyle(THIS_ StackViewAniStyle aniStyle)
{
	m_aniStyle = aniStyle;
}

void SStackView::SetAniDir(THIS_ BOOL bVert)
{
	m_bVertAni = bVert;
}

SIZE SStackView::MeasureChildren(int nParentWid, int nParentHei)
{
	SIZE ret={0};
	SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
	BOOL getSize = FALSE;
	while(pChild){
		if(pChild->GetLayoutParam()->IsWrapContent(Any))
		{
			SIZE szChild;
			pChild->GetDesiredSize(&szChild,nParentWid,nParentHei);
			if(pChild->GetLayoutParam()->IsWrapContent(Horz) && ret.cx<szChild.cx)
				ret.cx = szChild.cx;
			if(pChild->GetLayoutParam()->IsWrapContent(Vert) && ret.cy<szChild.cy) 
				ret.cy = szChild.cy;
			getSize = TRUE;
		}
		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
	}
	if(!getSize){
		SWindow * pChild = GetWindow(GSW_FIRSTCHILD);
		if(pChild){
			pChild->GetDesiredSize(&ret,nParentWid,nParentHei);
		}
	}
	return ret;
}

BOOL SStackView::CreateChildren(SXmlNode xmlNode)
{
	BOOL bRet = __baseCls::CreateChildren(xmlNode);
	if(!bRet) return FALSE;
	if(m_iSel!=-2){
		int idx = 0;
		SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
		while(pChild){
			pChild->SetVisible(idx==m_iSel);
			pChild = pChild->GetWindow(GSW_NEXTSIBLING);
			idx ++;
		}
	}
	BuildChildsArray();
	return TRUE;
}

void SStackView::OnAfterInsertChild(SWindow *pChild)
{
	if(m_isLoading)
		return;

	BuildChildsArray();
}

void SStackView::OnAfterRemoveChild(SWindow *pChild)
{
	BuildChildsArray();
}

void SStackView::BuildChildsArray()
{
	if(m_animator.isStarted())
		m_animator.end();
	int nChilds = GetChildrenCount();
	m_childs.SetCount(nChilds);
	SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
	for(int i=0;i<nChilds;i++){
		SASSERT(pChild);
		m_childs[i]=pChild;
		pChild=pChild->GetWindow(GSW_NEXTSIBLING);
	}
}

IWindow * SStackView::GetIChild(THIS_ int iChild) const
{
	if(iChild<0 || iChild>(int)m_childs.GetCount())
		return NULL;
	if(iChild == 0)
		return (IWindow*)this;
	return m_childs[iChild-1];
}

StackViewAniStyle SStackView::GetChildAnimateStyle(int iChild) const
{
	IWindow *pChild = GetPage(iChild);
	if(!pChild) return kAniNone;
	SStackPage *pPage = sobj_cast<SStackPage>(pChild);
	if(pPage)
		return pPage->GetAnimateStyle();
	else
		return m_aniStyle;
}

BOOL SStackView::IsVertChildAnimate(int iChild) const
{
	IWindow *pChild = GetPage(iChild);
	if(!pChild) return FALSE;
	SStackPage *pPage = sobj_cast<SStackPage>(pChild);
	if(pPage)
		return pPage->IsVertAnimate();
	else
		return m_bVertAni;
}

//////////////////////////////////////////////////////////////////////////
void SViewSwitchAnimator::start(THIS_ ITimelineHandlersMgr *pTimerlineMgr)
{
	if(isStarted())
		end();
	CRect rc;
	m_pOwner->GetChildrenLayoutRect(&rc);
	m_pFrom = m_pOwner->GetPage(m_iFrom);
	m_pTo = m_pOwner->GetPage(m_iTo);
	if(m_pTo){
		m_pTo->Move(rc);
		m_pTo->SetVisible(TRUE,FALSE);
	}
	m_aniStyle = m_pOwner->GetChildAnimateStyle(m_iTo);
	m_bVertAni = m_pOwner->IsVertChildAnimate(m_iTo);

	if(m_aniStyle!= kFadeInOut){
		if(m_pTo) m_pTo->SetAlpha(255);
		if(m_pFrom) m_pFrom->SetAlpha(255);
	}
	initEvaluator();
	__baseCls::start(pTimerlineMgr);
	if(m_aniStyle == kAniNone){
		//end it right now.
		__baseCls::end();
	}
}

void SViewSwitchAnimator::onEvaluateValue(THIS_ float fraction)
{
	if(m_aniStyle == kAniNone)
		return;
	if(m_aniStyle == kFadeInOut){
		BYTE a = m_evalAlphaFrom.evaluate(fraction);
		if(m_pFrom) m_pFrom->SetAlpha(a);
		BYTE a2 = m_evalAlphaTo.evaluate(fraction);
		if(m_pTo) m_pTo->SetAlpha(a2);
	}else{
		RECT rc1 = m_evalRcFrom.evaluate(fraction);
		if(m_pFrom) m_pFrom->Move(&rc1);
		RECT rc2 = m_evalRcTo.evaluate(fraction);
		if(m_pTo) m_pTo->Move(&rc2);
	}
}

void SViewSwitchAnimator::OnOwnerResize()
{
	if(!isStarted())
		return;
	initEvaluator();
}

void SViewSwitchAnimator::initEvaluator()
{
	CRect rc;
	m_pOwner->GetChildrenLayoutRect(&rc);

	if(m_aniStyle == kFadeInOut){
		m_evalAlphaFrom.setRange(255,0);
		m_evalAlphaTo.setRange(0,255);
	}else if(m_aniStyle == kMoveInOut){
		if(m_bVertAni){
			if(m_iFrom<m_iTo){
				//to => from => hide
				CRect rc2 = rc;
				rc2.top = rc2.bottom;
				m_evalRcFrom.setRange(rc,rc2);
				CRect rc3 = rc;
				rc3.bottom = rc3.top;
				m_evalRcTo.setRange(rc3,rc);
			}else{
				//hide <= from <= to
				CRect rc2 = rc;
				rc2.bottom = rc2.top;
				m_evalRcFrom.setRange(rc,rc2);
				CRect rc3 = rc;
				rc3.top = rc3.bottom;
				m_evalRcTo.setRange(rc3,rc);
			}
		}else{
			if(m_iFrom<m_iTo){
				//to => from => hide
				CRect rc2 = rc;
				rc2.left = rc2.right;
				m_evalRcFrom.setRange(rc,rc2);
				CRect rc3 = rc;
				rc3.right = rc3.left;
				m_evalRcTo.setRange(rc3,rc);
			}else{
				//hide <= from <= to
				CRect rc2 = rc;
				rc2.right = rc2.left;
				m_evalRcFrom.setRange(rc,rc2);
				CRect rc3 = rc;
				rc3.left = rc3.right;
				m_evalRcTo.setRange(rc3,rc);
			}
		}
	}else if(m_aniStyle == kPushInOut)
	{
		if(m_bVertAni){
			if(m_iFrom < m_iTo){
				CRect rc2 = rc;
				rc2.OffsetRect(0,rc.Height());
				m_evalRcFrom.setRange(rc,rc2);
				CRect rc3 = rc;
				rc3.OffsetRect(0,-rc.Height());
				m_evalRcTo.setRange(rc3,rc);
			}else{
				CRect rc2 = rc;
				rc2.OffsetRect(0,-rc.Height());
				m_evalRcFrom.setRange(rc,rc2);
				CRect rc3 = rc;
				rc3.OffsetRect(0,rc.Height());
				m_evalRcTo.setRange(rc3,rc);
			}
		}else {
			if(m_iFrom < m_iTo){
				CRect rc2 = rc;
				rc2.OffsetRect(rc.Width(),0);
				m_evalRcFrom.setRange(rc,rc2);
				CRect rc3 = rc;
				rc3.OffsetRect(-rc.Width(),0);
				m_evalRcTo.setRange(rc3,rc);
			}else{
				CRect rc2 = rc;
				rc2.OffsetRect(-rc.Width(),0);
				m_evalRcFrom.setRange(rc,rc2);
				CRect rc3 = rc;
				rc3.OffsetRect(rc.Width(),0);
				m_evalRcTo.setRange(rc3,rc);
			}
		}
	}
}


SNSEND

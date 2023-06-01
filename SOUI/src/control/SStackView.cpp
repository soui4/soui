#include <souistd.h>
#include <Control/SStackView.h>

SNSBEGIN


SStackView::SStackView(void):m_iSel(-1),m_animator(this)
{
	m_bClipClient = true;
	m_animator.addListener(this);
}

SStackView::~SStackView(void)
{
}

BOOL SStackView::SelectView(int iSel,BOOL enableAnimate)
{
	if(iSel<0 || iSel>=(int)GetChildrenCount())
		iSel = -1;
	if(m_animator.isStarted())
		m_animator.end();
	if(iSel == m_iSel)
		return TRUE;
	m_animator.SetFrom(m_iSel);
	m_animator.SetTo(iSel);
	m_animator.start(GetContainer());
	if(!enableAnimate){
		m_animator.end();
	}
	return TRUE;
}

IWindow* SStackView::GetCurView(CTHIS) const
{
	SWindow * pRet = m_animator.GetToWindow();
	if(!pRet && m_iSel!=-1){
		pRet = GetChild(m_iSel+1);
	}
	return pRet;
}

void SStackView::OnInitFinished(THIS_ IXmlNode * xmlNode)
{
	int idx = 0;
	SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
	while(pChild){
		pChild->SetVisible(idx==m_iSel);
		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
		idx ++;
	}
}

void SStackView::UpdateChildrenPosition(THIS)
{
	if(m_iSel !=-1)
	{
		if(m_animator.isStarted())
			return;
		SWindow *pChild=GetChild(m_iSel + 1);
		if(pChild){
			RECT rc;
			GetChildrenLayoutRect(&rc);
			pChild->Move(&rc);
		}
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
	m_iSel = m_animator.GetTo();
	SWindow * pFrom = m_animator.GetFromWindow();
	if(pFrom) pFrom->SetVisible(FALSE,TRUE);
}

void SStackView::SetAniStyle(THIS_ StackViewAniStyle aniStyle)
{
	m_animator.SetAniStyle(aniStyle);
}

void SStackView::SetAniDir(THIS_ BOOL bVert)
{
	m_animator.SetAniDir(bVert);
}

//////////////////////////////////////////////////////////////////////////
void SViewSwitchAnimator::start(THIS_ ITimelineHandlersMgr *pTimerlineMgr)
{
	if(isStarted())
		end();
	CRect rc;
	m_pOwner->GetChildrenLayoutRect(&rc);
	m_pFrom = m_iFrom==-1?NULL:m_pOwner->GetChild(m_iFrom+1);
	m_pTo = m_iTo==-1?NULL:m_pOwner->GetChild(m_iTo + 1);
	if(m_pTo){
		m_pTo->Move(rc);
		m_pTo->SetVisible(TRUE,FALSE);
	}
	if(m_aniType!= kFadeInOut){
		if(m_pTo) m_pTo->SetAlpha(255);
		if(m_pFrom) m_pFrom->SetAlpha(255);
	}
	__baseCls::start(pTimerlineMgr);
	if(m_aniType == kAniNone){
		//end  right now.
		__baseCls::end();
	}else{
		initEvaluator();
	}
}

void SViewSwitchAnimator::onEvaluateValue(THIS_ float fraction)
{
	if(m_aniType == kAniNone)
		return;
	if(m_aniType == kFadeInOut){
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

	if(m_aniType == kFadeInOut){
		m_evalAlphaFrom.setRange(255,0);
		m_evalAlphaTo.setRange(0,255);
	}else if(m_aniType == kMoveInOut){
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
	}else if(m_aniType == kPushInOut)
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

void SViewSwitchAnimator::SetAniStyle(StackViewAniStyle aniStyle)
{
	if(isStarted())
		end();
	m_aniType = aniStyle;
}

void SViewSwitchAnimator::SetAniDir(BOOL bVert)
{
	if(isStarted())
		end();
	m_bVertAni = bVert;
}

SNSEND

#include "stdafx.h"
#include "SPathView.h"

namespace SOUI
{
	SPathView::SPathView(void)
		:m_nLineWidth(2)
		,m_nLineStyle(PS_DASH)
		,m_nFrameSize(9)
		,m_crLine(RGBA(255,0,0,255))
		,m_fCornerRadius(5.0f)
	{
		GetEventSet()->addEvent(EVENTID(EventPath));
		GETRENDERFACTORY->CreatePath(&m_path);
	}

	SPathView::~SPathView(void)
	{
	}

	void SPathView::AddPoint(const POINT *pts,int nPts)
	{
		if(!m_path) return;
		for(int i=0;i<nPts;i++)
		{
			m_lstPoints.Add(pts[i]);
		}
		updatePath();
		OnPathChanged();
	}

	void SPathView::OnLButtonDown(UINT nFlags,CPoint pt)
	{
		if(!m_path)
			return;
		CRect rc = GetClientRect();
		pt -= rc.TopLeft();
		m_lstPoints.Add(pt);
		updatePath();
		OnPathChanged();
	}

	void SPathView::OnRButtonDown(UINT nFlags, CPoint point)
	{
		if(!m_path)
			return;

		CRect rc=GetClientRect();
		point-=rc.TopLeft();
		for(int i=0;i<m_lstPoints.GetCount();i++)
		{
			POINT pt = m_lstPoints[i];
			CRect rcFrame(pt,CSize(m_nFrameSize,m_nFrameSize));
			rcFrame.OffsetRect(-(m_nFrameSize-1)/2,-(m_nFrameSize-1)/2);
			if(rcFrame.PtInRect(point))
			{
				m_lstPoints.RemoveAt(i);
				updatePath();
				OnPathChanged();
				break;
			}
		}
	}

	void SPathView::OnPaint(IRenderTarget *pRT)
	{
		if(!m_path)
		{
			__baseCls::OnPaint(pRT);
			return;
		}

		int count = m_lstPoints.GetCount();
		CRect rc=GetClientRect();

		if(count>1)
		{
			SAutoRefPtr<ICornerPathEffect> pathEffect;
			GETRENDERFACTORY->CreatePathEffect(__suidof(ICornerPathEffect),(IPathEffect**)&pathEffect);
			if(pathEffect)
				pathEffect->Init(m_fCornerRadius);

			SAutoRefPtr<IPenS> pen,oldPen;
			pRT->CreatePen(m_nLineStyle,m_crLine,m_nLineWidth,&pen);
			pRT->SelectObject(pen,(IRenderObj**)&oldPen);

			m_path->offset(rc.left,rc.top);

			pRT->DrawPath(m_path,pathEffect);

			m_path->offset(-rc.left,-rc.top);

			pRT->SelectObject(oldPen,NULL);
		}

		for(int i=0;i<count;i++)
		{
			POINT pt = m_lstPoints[i];
			CRect rcFrame(pt,CSize(m_nFrameSize,m_nFrameSize));
			rcFrame.OffsetRect(-(m_nFrameSize-1)/2,-(m_nFrameSize-1)/2);
			rcFrame.OffsetRect(rc.TopLeft());
			pRT->DrawRectangle(&rcFrame);
		}
	}

	void SPathView::OnPathChanged()
	{
		EventPath evt(this);
		evt.fLength = m_path->getLength();
		FireEvent(evt);
		Invalidate();
	}

	void SPathView::updatePath()
	{
		m_path->reset();
		m_path->beginFigure(0,0);
		m_path->addPoly(m_lstPoints.GetData(),m_lstPoints.GetCount(),false);
		m_path->endFigure(FALSE);
		m_path->close();
	}

}

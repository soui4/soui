#include "stdafx.h"
#include "SizingFrame.h"
#include <helper/slog.h>

#define kLogTag "sizingFrame"

SNSBEGIN

SSizingFrame::SSizingFrame()
	: m_bDragging(FALSE)
	, m_nHitTest(HT_UNKNOWN)
	, m_nBorderSize(1)
	, m_nCornerSize(6)
	, m_crFrame(RGBA(255, 0, 0, 255))
	, m_hTarget(-1)
{
	m_rcDragStart.SetRectEmpty();
	m_ptDragStart.SetPoint(0, 0);
	AddEvent(EVENTID(EventFrameMoved));
}

SSizingFrame::~SSizingFrame()
{
}

void SSizingFrame::SetTarget(SWND hTarget)
{
	m_hTarget = hTarget;
}

void SSizingFrame::OnPaint(IRenderTarget *pRT)
{
	SPainter state;
	BeforePaint(pRT, state);
	CRect rcClient = GetClientRect();
	rcClient.DeflateRect(m_nCornerSize/2, m_nCornerSize/2);
	SAutoRefPtr<IPenS> pen,oldPen;
	pRT->CreatePen(PS_SOLID, m_crFrame, m_nBorderSize, &pen);
	pRT->SelectObject(pen, (IRenderObj**)&oldPen);
	pRT->DrawRectangle(rcClient);
	
	for (int i = 0; i < 4; i++) {
		CRect rcCorner = GetCornerRect(i);
		if (!rcCorner.IsRectEmpty()) {
			pRT->DrawRectangle(rcCorner);
		}
	}
	for (int i = 0; i < 4; i++) {
		CRect rcBorder = GetBorderRect(i);
		if (!rcBorder.IsRectEmpty()) {
			pRT->DrawRectangle(rcBorder);
		}
	}
	CRect rcCenter = GetCenterRect();
	if (!rcCenter.IsRectEmpty()) {
		pRT->DrawRectangle(rcCenter);
	}
	pRT->SelectObject(oldPen);	
	AfterPaint(pRT, state);
}

void SSizingFrame::OnLButtonDown(UINT nFlags, CPoint point)
{
	__baseCls::OnLButtonDown(nFlags, point);
	m_nHitTest = HitTest(point);
	if (m_nHitTest != HT_UNKNOWN) {
		m_bDragging = TRUE;
		m_ptDragStart = point;
		GetWindowRect(&m_rcDragStart);
	}
}

void SSizingFrame::OnLButtonUp(UINT nFlags, CPoint point)
{
	__baseCls::OnLButtonUp(nFlags, point);
    if (m_bDragging)
    {
		m_bDragging = FALSE;
		
		// 计算新的窗口位置和大小
		CRect rcWnd = m_rcDragStart;
		int dx = point.x - m_ptDragStart.x;
		int dy = point.y - m_ptDragStart.y;
		
		switch (m_nHitTest) {
		case HT_LEFT:
			rcWnd.left += dx;
			break;
		case HT_TOP:
			rcWnd.top += dy;
			break;
		case HT_RIGHT:
			rcWnd.right += dx;
			break;
		case HT_BOTTOM:
			rcWnd.bottom += dy;
			break;
		case HT_TOPLEFT:
			rcWnd.left += dx;
			rcWnd.top += dy;
			break;
		case HT_TOPRIGHT:
			rcWnd.right += dx;
			rcWnd.top += dy;
			break;
		case HT_BOTTOMRIGHT:
			rcWnd.right += dx;
			rcWnd.bottom += dy;
			break;
		case HT_BOTTOMLEFT:
			rcWnd.left += dx;
			rcWnd.bottom += dy;
			break;
		case HT_CENTER:
			rcWnd.OffsetRect(dx, dy);
			break;
		}
		if (rcWnd.Width() < 0)
			rcWnd.right = rcWnd.left + 0;
		if (rcWnd.Height() < 0)
			rcWnd.bottom = rcWnd.top + 0;
			
		// 更新窗口位置和大小
		Move(rcWnd);
		if(rcWnd !=m_rcDragStart)
		{
			EventFrameMoved evt(this);
			evt.hTarget = m_hTarget;
			evt.rcOld = m_rcDragStart;
			evt.rcNew = rcWnd;
			FireEvent(evt);
		}
	}
	
	m_nHitTest = HT_UNKNOWN;
	Invalidate();
}

void SSizingFrame::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging) {
		// 计算新的窗口位置和大小
		CRect rcWnd = m_rcDragStart;
		int dx = point.x - m_ptDragStart.x;
		int dy = point.y - m_ptDragStart.y;
		
		switch (m_nHitTest) {
		case HT_LEFT:
			rcWnd.left += dx;
			break;
		case HT_TOP:
			rcWnd.top += dy;
			break;
		case HT_RIGHT:
			rcWnd.right += dx;
			break;
		case HT_BOTTOM:
			rcWnd.bottom += dy;
			break;
		case HT_TOPLEFT:
			rcWnd.left += dx;
			rcWnd.top += dy;
			break;
		case HT_TOPRIGHT:
			rcWnd.right += dx;
			rcWnd.top += dy;
			break;
		case HT_BOTTOMRIGHT:
			rcWnd.right += dx;
			rcWnd.bottom += dy;
			break;
		case HT_BOTTOMLEFT:
			rcWnd.left += dx;
			rcWnd.bottom += dy;
			break;
		case HT_CENTER:
			rcWnd.OffsetRect(dx, dy);
			break;
		}
		
		// 确保窗口大小不小于最小值
		if (rcWnd.Width() < 20)
			rcWnd.right = rcWnd.left + 20;
		if (rcWnd.Height() < 20)
			rcWnd.bottom = rcWnd.top + 20;
			
		// 更新窗口位置和大小
		Move(rcWnd);
    }
    else
    {
		m_nHitTest = HitTest(point);
	}
	
	__baseCls::OnMouseMove(nFlags, point);
}

void SSizingFrame::OnMouseLeave()
{
	Invalidate();
	__baseCls::OnMouseLeave();
}

int SSizingFrame::HitTest(CPoint pt)
{
	for (int i = 0; i < 4; i++) {
		CRect rcCorner = GetCornerRect(i);
		if (rcCorner.PtInRect(pt)) {
			return HT_TOPLEFT + i;
		}
	}
	for (int i = 0; i < 4; i++) {
		CRect rcBorder = GetBorderRect(i);
		if (rcBorder.PtInRect(pt)) {
			return HT_LEFT + i;
		}
	}
	
	// 检查中心区域
	CRect rcCenter = GetCenterRect();
	if (rcCenter.PtInRect(pt)) {
		return HT_CENTER;
	}
	
	return HT_UNKNOWN;
}

BOOL SSizingFrame::OnSetCursor(const CPoint &pt)
{
	LPCTSTR lpCursor = IDC_ARROW;
	
	switch (m_nHitTest) {
	case HT_LEFT:
		lpCursor = IDC_SIZEWE;
		break;
	case HT_TOP:
		lpCursor = IDC_SIZENS;
		break;
	case HT_RIGHT:
		lpCursor = IDC_SIZEWE;
		break;
	case HT_BOTTOM:
		lpCursor = IDC_SIZENS;
		break;
	case HT_TOPLEFT:
		lpCursor = IDC_SIZENWSE;
		break;
	case HT_TOPRIGHT:
		lpCursor = IDC_SIZENESW;
		break;
	case HT_BOTTOMRIGHT:
		lpCursor = IDC_SIZENWSE;
		break;
	case HT_BOTTOMLEFT:
		lpCursor = IDC_SIZENESW;
		break;
	case HT_CENTER:
		lpCursor = IDC_SIZEALL;
		break;
	case HT_UNKNOWN:
		lpCursor = IDC_ARROW;
		break;
	}
	HCURSOR hCursor = GETRESPROVIDER->LoadCursor(lpCursor);
	if (hCursor)
		::SetCursor(hCursor);	
	return TRUE;
}

CRect SSizingFrame::GetBorderRect(int nBorder) const
{
	CRect rcFrame = GetWindowRect();
	rcFrame.DeflateRect(m_nCornerSize/2, m_nCornerSize/2);
	CPoint ptCenter;
	switch (nBorder) {
	case 0: // 左边
		ptCenter.SetPoint(rcFrame.left, rcFrame.CenterPoint().y);
		break;
	case 1: // 上边
		ptCenter.SetPoint(rcFrame.CenterPoint().x, rcFrame.top);
		break;
	case 2: // 右边
		ptCenter.SetPoint(rcFrame.right, rcFrame.CenterPoint().y);
		break;
	case 3: // 下边
		ptCenter.SetPoint(rcFrame.CenterPoint().x, rcFrame.bottom);
		break;
	default:
		break;
	}
	CRect rcRet(ptCenter,CSize(0,0));
	rcRet.InflateRect(m_nCornerSize/2, m_nCornerSize/2);
	return rcRet;
}

CRect SSizingFrame::GetCornerRect(int nCorner) const
{
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	CRect rcFrame = GetWindowRect();
	
	switch (nCorner) {
	case 0: // 左上角
		return CRect(rcFrame.left, rcFrame.top, 
					rcFrame.left + m_nCornerSize, rcFrame.top + m_nCornerSize);
	case 1: // 右上角
		return CRect(rcFrame.right - m_nCornerSize, rcFrame.top,
					rcFrame.right, rcFrame.top + m_nCornerSize);
	case 2: // 右下角
		return CRect(rcFrame.right - m_nCornerSize, rcFrame.bottom - m_nCornerSize,
					rcFrame.right, rcFrame.bottom);
	case 3: // 左下角
		return CRect(rcFrame.left, rcFrame.bottom - m_nCornerSize,
					rcFrame.left + m_nCornerSize, rcFrame.bottom);
	default:
		return CRect();
	}
}

CRect SSizingFrame::GetCenterRect() const
{
	CRect rcWnd= GetWindowRect();
	CRect rcCenter(rcWnd.CenterPoint(), CSize(0, 0));
	rcCenter.InflateRect(m_nCornerSize/2, m_nCornerSize/2);
	return rcCenter;
}

BOOL SSizingFrame::IsContainPoint(THIS_ POINT pt, BOOL bClientOnly) SCONST{
	for(int i=0; i<4; i++){
		if(GetCornerRect(i).PtInRect(pt)) return TRUE;
		if(GetBorderRect(i).PtInRect(pt)) return TRUE;
	}
	return GetCenterRect().PtInRect(pt);
}
SNSEND
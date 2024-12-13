#include "stdafx.h"
#include "SSnapshotCtrl.h"

SSnapshotCtrl::SSnapshotCtrl(void)
{
	m_bCapture = false;

	m_emPosType = Null;
}

SSnapshotCtrl::~SSnapshotCtrl(void)
{
}

void SSnapshotCtrl::OnPaint(IRenderTarget* pRT)
{
	HDC hDC = pRT->GetDC(0);
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(hDC);
	dcCompatible.SelectBitmap(*m_pBitmap);

	SIZE szBMP;
	m_pBitmap->GetSize(szBMP);
	CRect rcWindow(0, 0, szBMP.cx, szBMP.cy);

	BitBlt(hDC, 0, 0, szBMP.cx, szBMP.cy, dcCompatible, 0, 0, SRCCOPY);

	//绘制灰色遮罩
	{
		CAutoRefPtr<IPath> path;
		GETRENDERFACTORY->CreatePath(&path);
		path->addRect(rcWindow);

		//CRect rcCapture(200, 200, 1000, 1000);
		pRT->PushClipRect(m_rcCaptureArea, RGN_XOR);

		CAutoRefPtr<IBrush> brush, oldbrush;
		pRT->CreateSolidColorBrush(RGBA(0, 0, 0, 100), &brush);
		pRT->SelectObject(brush, (IRenderObj**)&oldbrush);
		pRT->FillPath(path);
		pRT->SelectObject(oldbrush, NULL);
	}
}

void SSnapshotCtrl::OnMouseMove(UINT nFlags, SOUI::CPoint point)
{
	SetMsgHandled(FALSE);
	SOUI::CRect rcWnd = GetWindowRect();

	if ((nFlags & MK_LBUTTON))
	{
		if (m_ptDown == point)
			return;

		switch (m_emPosType)
		{
		case Null:
		{
			if (m_bCapture) return;
			if (m_ptDown == point)
				return;

			m_rcCaptureArea.SetRect(m_ptDown, point);
			m_rcCaptureArea.NormalizeRect();
		}
		break;
		case TopLeft:
			break;
		case TopCenter:
			break;
		case TopRight:
			break;
		case RightCenter:
			break;
		case BottomRight:
			break;
		case BottomCenter:
			break;
		case BottomLeft:
			break;
		case LeftCenter:
			break;
		case SelectRect:
		{
			SOUI::CPoint ptLT = point - m_ptDown;			// 相对 鼠标点击 时  的 偏移量  也就是 移动 的 值 
			if (ptLT.x < rcWnd.left)
				ptLT.x = rcWnd.left;
			else if (ptLT.x > rcWnd.right - m_rcCaptureArea.Width())
				ptLT.x = rcWnd.right - m_rcCaptureArea.Width();
			if (ptLT.y < rcWnd.top)
				ptLT.y = rcWnd.top;
			else if (ptLT.y > rcWnd.bottom - m_rcCaptureArea.Height())
				ptLT.y = rcWnd.bottom - m_rcCaptureArea.Height();
			m_rcCaptureArea.MoveToXY(ptLT);

// 			int nCaptureWid = m_rcCaptureArea.Width();
// 			int nCaptureHei = m_rcCaptureArea.Height();
// 			CPoint ptPos(m_rcCaptureArea.TopLeft());
// 
// 			if (m_ptDown.x > point.x)
// 				ptPos.x -= (m_ptDown.x - point.x);
// 			else
// 				ptPos.x += (point.x - m_ptDown.x);
// 			if (m_ptDown.y > point.y)
// 				ptPos.y -= (m_ptDown.y - point.y);
// 			else
// 				ptPos.y += (point.y - m_ptDown.y);
// 
// 			if (ptPos.x < 0) ptPos.x = 0;
// 			if (ptPos.y < 0) ptPos.y = 0;
// 
// 			//m_rcCaptureArea.TopLeft() = ptPos;
// 			m_rcCaptureArea.SetRect(ptPos.x, ptPos.y, ptPos.x + nCaptureWid, ptPos.y + nCaptureHei);
		}
		break;
		default:
			break;
		}
	}

	Invalidate();
}

void SSnapshotCtrl::OnLButtonDown(UINT nFlags, SOUI::CPoint point)
{
	SetCapture();
	
	if (m_rcCaptureArea.IsRectEmpty())
	{
		m_emPosType = Null;
		m_ptDown = point;
	}
	else if (PtInRect(m_rcCaptureArea, point))
	{
		m_emPosType = SelectRect;
		m_ptDown = point;
	}
}

void SSnapshotCtrl::OnLButtonUp(UINT nFlags, SOUI::CPoint point)
{
	SetMsgHandled(FALSE);
	ReleaseCapture();
	SOUI::CRect rcWnd = GetWindowRect();

	m_bCapture = true;
	switch (m_emPosType)
	{
	case Null:
		break;
	case TopLeft:
		break;
	case TopCenter:
		break;
	case TopRight:
		break;
	case RightCenter:
		break;
	case BottomRight:
		break;
	case BottomCenter:
		break;
	case BottomLeft:
		break;
	case LeftCenter:
		break;
	case SelectRect:
	{
		SOUI::CPoint ptLT = point - m_ptDown;			// 相对 鼠标点击 时  的 偏移量  也就是 移动 的 值 
		if (ptLT.x < rcWnd.left)
			ptLT.x = rcWnd.left;
		else if (ptLT.x > rcWnd.right - m_rcCaptureArea.Width())
			ptLT.x = rcWnd.right - m_rcCaptureArea.Width();
		if (ptLT.y < rcWnd.top)
			ptLT.y = rcWnd.top;
		else if (ptLT.y > rcWnd.bottom - m_rcCaptureArea.Height())
			ptLT.y = rcWnd.bottom - m_rcCaptureArea.Height();
		m_rcCaptureArea.MoveToXY(ptLT);
	}
	break;
	default:
		break;
	}

	m_emPosType = Null;
	Invalidate();
}

void SSnapshotCtrl::OnLButtonDblClk(UINT nFlags, SOUI::CPoint point)
{
	//
}

void SSnapshotCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	//
}

void SSnapshotCtrl::SetBmpResource(CBitmap* pBmp)
{
	m_pBitmap = pBmp;

	//m_vecBitmap.push_back(pBmp);
	Invalidate();
}
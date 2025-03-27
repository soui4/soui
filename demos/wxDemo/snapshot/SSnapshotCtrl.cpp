#include "stdafx.h"
#include "SSnapshotCtrl.h"

SSnapshotCtrl::SSnapshotCtrl(void)
{
	m_emPosType = Null;
	m_hCurSelect = LoadCursor(NULL, IDC_SIZEALL);
    m_hCurTopRight = LoadCursor(NULL, IDC_SIZENESW);
    m_hCurBottomRight = LoadCursor(NULL, IDC_SIZENWSE);
    m_hCurTop = LoadCursor(NULL, IDC_SIZENS);
    m_hCurBottom = LoadCursor(NULL, IDC_SIZENS);
    m_hCurLeft = LoadCursor(NULL, IDC_SIZEWE);
    m_hCurRight = LoadCursor(NULL, IDC_SIZEWE);
    m_hCurTopLeft = LoadCursor(NULL, IDC_SIZENWSE);
    m_hCurBottomLeft = LoadCursor(NULL, IDC_SIZENESW);
    m_hCurHand = LoadCursor(NULL, IDC_HAND);
    m_hCurArrow = LoadCursor(NULL, IDC_ARROW);

    m_bCapture = false;
    m_bSelectFinished = false;
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
		pRT->PushClipRect(m_rcCapture, RGN_XOR);

		for (int i = 0; i < 8; i++)
		{
            pRT->PushClipRect(m_rcPos[i], RGN_XOR);
		}

		CAutoRefPtr<IBrush> brush, oldbrush;
		pRT->CreateSolidColorBrush(RGBA(0, 0, 0, 100), &brush);
		pRT->SelectObject(brush, (IRenderObj**)&oldbrush);
		pRT->FillPath(path);
		pRT->SelectObject(oldbrush, NULL);
	}


	{//绘制8个操作点
        CAutoRefPtr<IBrush> brush, oldbrush;
        pRT->CreateSolidColorBrush(RGBA(255, 0, 0, 255), &brush);
        pRT->SelectObject(brush, (IRenderObj **)&oldbrush);
        for (int i = 0; i < 8; ++i)
        {
            CRect rcDot(m_rcPos[i]);
            pRT->FillRectangle(rcDot);
        }
        pRT->SelectObject(oldbrush, NULL);
    }
}

void SSnapshotCtrl::OnMouseMove(UINT nFlags, SOUI::CPoint point)
{
	SetMsgHandled(FALSE);
	SOUI::CRect rcWnd = GetWindowRect();

    //可移动、拉伸截图区域
    if (!m_bSelectFinished)
    {
        if ((nFlags & MK_LBUTTON))
        {
            if (m_ptDown == point)
                return;

            switch (m_emPosType)
            {
            case Null:
            {
                //选取截图的区域
                if (m_bCapture)
                    return;

                m_rcCapture.SetRect(m_ptDown, point);
                m_rcCapture.NormalizeRect();
            }
            break;
            case TopLeft:
            {
                if (point.x > m_rcCapture.right && point.y > m_rcCapture.bottom)
                {
                    m_emPosType = BottomRight;

                    m_rcCapture.left = m_rcCapture.right;
                    m_rcCapture.right = point.x;
                    m_rcCapture.top = m_rcCapture.bottom;
                    m_rcCapture.bottom = point.y;
                }
                else if (point.x > m_rcCapture.right)
                {
                    m_emPosType = TopRight;
                    m_rcCapture.left = m_rcCapture.right;
                    m_rcCapture.right = point.x;
                }
                else if (point.y > m_rcCapture.bottom)
                {
                    m_emPosType = BottomLeft;
                    m_rcCapture.top = m_rcCapture.bottom;
                    m_rcCapture.bottom = point.y;
                }
                else
                {
                    m_rcCapture.left = (point.x > rcWnd.left) ? point.x : rcWnd.left;
                    m_rcCapture.top = (point.y > rcWnd.top) ? point.y : rcWnd.top;
                }
            }
            break;
            case TopCenter:
            {
                if (point.y > m_rcCapture.bottom)
                {
                    m_emPosType = BottomCenter;
                    m_rcCapture.top = m_rcCapture.bottom;
                    m_rcCapture.bottom = point.y;
                }
                else
                {
                    m_rcCapture.top = (point.y > rcWnd.top) ? point.y : rcWnd.top;
                }
            }
            break;
            case TopRight:
            {
                if (point.x < m_rcCapture.left && point.y > m_rcCapture.bottom)
                {
                    m_emPosType = BottomLeft;

                    m_rcCapture.right = m_rcCapture.left;
                    m_rcCapture.left = point.x;

                    m_rcCapture.top = m_rcCapture.bottom;
                    m_rcCapture.bottom = point.y;
                }
                else if (point.x < m_rcCapture.left)
                {
                    m_emPosType = TopLeft;

                    m_rcCapture.right = m_rcCapture.left;
                    m_rcCapture.left = point.x;
                }
                else if (point.y > m_rcCapture.bottom)
                {
                    m_emPosType = BottomRight;

                    m_rcCapture.top = m_rcCapture.bottom;
                    m_rcCapture.bottom = point.y;
                }
                else
                {
                    m_rcCapture.top = (point.y > rcWnd.top) ? point.y : rcWnd.top;
                    m_rcCapture.right = (point.x < rcWnd.right) ? point.x : rcWnd.right;
                }
            }
            break;
            case RightCenter:
            {
                if (point.x < m_rcCapture.left)
                { // 如果 过线了 就 换成 反的
                    m_emPosType = LeftCenter;
                    m_rcCapture.right = m_rcCapture.left;
                    m_rcCapture.left = point.x;
                }
                else
                {
                    m_rcCapture.right = (point.x < rcWnd.right) ? point.x : rcWnd.right;
                }
            }
            break;
            case BottomRight:
            {
                if (point.x < m_rcCapture.left && point.y < m_rcCapture.top)
                {
                    m_emPosType = TopLeft;

                    m_rcCapture.right = m_rcCapture.left;
                    m_rcCapture.left = point.x;

                    m_rcCapture.bottom = m_rcCapture.top;
                    m_rcCapture.top = point.y;
                }
                else if (point.x < m_rcCapture.left)
                {
                    m_emPosType = BottomLeft;

                    m_rcCapture.right = m_rcCapture.left;
                    m_rcCapture.left = point.x;
                }
                else if (point.y < m_rcCapture.top)
                {
                    m_emPosType = TopRight;

                    m_rcCapture.bottom = m_rcCapture.top;
                    m_rcCapture.top = point.y;
                }
                else
                {
                    m_rcCapture.bottom = (point.y < rcWnd.bottom) ? point.y : rcWnd.bottom;
                    m_rcCapture.right = (point.x < rcWnd.right) ? point.x : rcWnd.right;
                }
            }
            break;
            case BottomCenter:
            {
                if (point.y < m_rcCapture.top)
                {
                    m_emPosType = TopCenter;
                    m_rcCapture.bottom = m_rcCapture.top;
                    m_rcCapture.top = point.y;
                }
                else
                {
                    m_rcCapture.bottom = (point.y < rcWnd.bottom) ? point.y : rcWnd.bottom;
                }
            }
            break;
            case BottomLeft:
            {
                if (point.x > m_rcCapture.right && point.y < m_rcCapture.top)
                {
                    m_emPosType = TopRight;

                    m_rcCapture.bottom = m_rcCapture.top;
                    m_rcCapture.top = point.y;

                    m_rcCapture.left = m_rcCapture.right;
                    m_rcCapture.right = point.x;
                }
                else if (point.x > m_rcCapture.right)
                {
                    m_emPosType = BottomRight;
                    m_rcCapture.left = m_rcCapture.right;
                    m_rcCapture.right = point.x;
                }
                else if (point.y < m_rcCapture.top)
                {
                    m_emPosType = TopLeft;
                    m_rcCapture.bottom = m_rcCapture.top;
                    m_rcCapture.top = point.y;
                }
                else
                {
                    m_rcCapture.bottom = (point.y < rcWnd.bottom) ? point.y : rcWnd.bottom;
                    m_rcCapture.left = (point.x > rcWnd.left) ? point.x : rcWnd.left;
                }
            }
            break;
            case LeftCenter:
            {
                if (point.x > m_rcCapture.right)
                {
                    m_emPosType = RightCenter;
                    m_rcCapture.left = m_rcCapture.right;
                    m_rcCapture.right = point.x;
                }
                else
                {
                    m_rcCapture.left = (point.x > rcWnd.left) ? point.x : rcWnd.left;
                }
            }
            break;
            case SelectRect:
            {
                //整体移动截图的区域
                SOUI::CPoint ptLT = point - m_ptDown; // 相对 鼠标点击 时  的 偏移量  也就是 移动 的 值
                if (ptLT.x < rcWnd.left)
                    ptLT.x = rcWnd.left;
                else if (ptLT.x > rcWnd.right - m_rcCapture.Width())
                    ptLT.x = rcWnd.right - m_rcCapture.Width();
                if (ptLT.y < rcWnd.top)
                    ptLT.y = rcWnd.top;
                else if (ptLT.y > rcWnd.bottom - m_rcCapture.Height())
                    ptLT.y = rcWnd.bottom - m_rcCapture.Height();
                m_rcCapture.MoveToXY(ptLT);
            }
            break;
            default:
                break;
            }
        }
    }
    else
    {
        //不允许再移动、拉伸截图区域
    }

    //计算8个操作点
	CalcPos();
    //设置鼠标光标
	OnSetCursor(point);

	Invalidate();
}

void SSnapshotCtrl::OnLButtonDown(UINT nFlags, SOUI::CPoint point)
{
	SetCapture();
	
    if (!m_bSelectFinished)
    {
        if (m_rcCapture.IsRectEmpty())
        {
            m_emPosType = Null;
            m_ptDown = point;
        }
        else if (PtInRect(m_rcCapture, point))
        {
            m_emPosType = HitPos(point);
            m_ptDown = point - m_rcCapture.TopLeft();
        }
    }
    else
    {
        //非操作截图区域操作
    }
}

void SSnapshotCtrl::OnLButtonUp(UINT nFlags, SOUI::CPoint point)
{
	SetMsgHandled(FALSE);
	ReleaseCapture();
	SOUI::CRect rcWnd = GetWindowRect();

    if (!m_bSelectFinished)
    {
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
            SOUI::CPoint ptPos = point - m_ptDown; // 相对 鼠标点击 时  的 偏移量  也就是 移动 的 值
            if (ptPos.x < rcWnd.left)
                ptPos.x = rcWnd.left;
            else if (ptPos.x > rcWnd.right - m_rcCapture.Width())
                ptPos.x = rcWnd.right - m_rcCapture.Width();
            if (ptPos.y < rcWnd.top)
                ptPos.y = rcWnd.top;
            else if (ptPos.y > rcWnd.bottom - m_rcCapture.Height())
                ptPos.y = rcWnd.bottom - m_rcCapture.Height();
            m_rcCapture.MoveToXY(ptPos);
        }
        break;
        default:
            break;
        }

        m_emPosType = Null;
        Invalidate();
    }
    else
    {
        //非操作截图区域操作
    }
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

void SSnapshotCtrl::CalcPos()
{
    SOUI::CRect rcArea(m_rcCapture);
    if (m_rcCapture.IsRectEmpty())
        return;

    rcArea.InflateRect(1, 1);
    CAutoRefPtr<IPen> curPen, oldPen;

    SOUI::CPoint center = rcArea.CenterPoint();
    // 上左 方块
    m_rcPos[(int)TopLeft].SetRect(rcArea.left, rcArea.top, rcArea.left + 6, rcArea.top + 6);
    // 上中 方块
    m_rcPos[(int)TopCenter].SetRect(center.x - 3, rcArea.top, center.x + 3, rcArea.top + 6);
    // 上右 方块
    m_rcPos[(int)TopRight].SetRect(rcArea.right - 6, rcArea.top, rcArea.right, rcArea.top + 6);
    // 右中 方块
    m_rcPos[(int)RightCenter].SetRect(rcArea.right - 6, center.y - 3, rcArea.right, center.y + 3);
    // 下右 方块
    m_rcPos[(int)BottomRight].SetRect(rcArea.right - 6, rcArea.bottom - 6, rcArea.right, rcArea.bottom);
    // 下中 方块
    m_rcPos[(int)BottomCenter].SetRect(center.x - 3, rcArea.bottom - 6, center.x + 3, rcArea.bottom);
    // 下左 方块
    m_rcPos[(int)BottomLeft].SetRect(rcArea.left, rcArea.bottom - 6, rcArea.left + 6, rcArea.bottom);
    // 左中 方块
    m_rcPos[(int)LeftCenter].SetRect(rcArea.left, center.y - 3, rcArea.left + 6, center.y + 3);
}

EcPosType SSnapshotCtrl::HitPos(const SOUI::CPoint &pt)
{
    if (m_rcCapture.PtInRect(pt))
    {
        for (int i = 0; i < 8; ++i)
        {
            if (m_rcPos[i].PtInRect(pt))
                return EcPosType(i);
        }

        return SelectRect;
    }
    return Null;
}

BOOL SSnapshotCtrl::OnSetCursor(const CPoint& pt)
{
    EcPosType hitTest = HitPos(pt);
    switch (hitTest)
    {
    case TopLeft:
    case BottomRight:
        ::SetCursor(m_hCurTopLeft);
        break;
    case TopCenter:
    case BottomCenter:
        ::SetCursor(m_hCurTop);
        break;
    case TopRight:
    case BottomLeft:
        ::SetCursor(m_hCurTopRight);
        break;
    case LeftCenter:
    case RightCenter:
        ::SetCursor(m_hCurLeft);
        break;
    case SelectRect:
        ::SetCursor(m_hCurSelect);
        break;
//     case Opt1:
//         ::SetCursor(m_hCurOpt1);
//         break;
//     case Opt2:
//         ::SetCursor(m_hCurOpt2);
//         break;
//     case Opt3:
//         ::SetCursor(m_hCurOpt3[m_nPenSize == 1 ? 0 : (m_nPenSize == 2) ? 1 : 2]);
//         break;
    case Null:
    default:
        __super::OnSetCursor(pt);
        break;
    }
    return TRUE;
}
#include "stdafx.h"
#include "SSkinChess.h"

SNSBEGIN

SIZE SSkinChess::GetSkinSize() const
{
    SIZE sz = m_pImgShadow->Size();
    return sz;
}

int SSkinChess::GetStates() const
{
    return CHESS_COLS*CHESS_ROWS;
}

CPoint SSkinChess::GetCenter() const
{
    if(m_ptCenter.x != -1)
        return m_ptCenter;
    SIZE szImg = GetImage()->Size();
    SIZE szChess = {szImg.cx/CHESS_COLS,szImg.cy/CHESS_ROWS};
    m_ptCenter = CPoint(szChess.cx/2,szChess.cy/2);
    return m_ptCenter;
}

void SSkinChess::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage())
        return;

    int iCol = iState % CHESS_COLS;
    int iRow = iState / CHESS_ROWS;

    if(iCol == 1){
        pRT->DrawBitmapEx(rcDraw, m_pImgShadow, NULL, GetExpandMode(), byAlpha);
    }
    SIZE szImg = GetImage()->Size();
    SIZE szChess = {szImg.cx/CHESS_COLS,szImg.cy/CHESS_ROWS};
    CRect rcSrc(CPoint(),szChess);
    OffsetRect(&rcSrc, iCol * szChess.cx, iRow * szChess.cy);
    CRect rcDst = *rcDraw;
    rcDst.bottom = rcDst.top + szChess.cy;
    pRT->DrawBitmapEx(&rcDst, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
}
SNSEND
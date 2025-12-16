#include "stdafx.h"
#include "SSkinPiece.h"

SNSBEGIN

SIZE SSkinPiece::GetSkinSize() const
{
    SIZE sz = m_pImgShadow->Size();
    return sz;
}

int SSkinPiece::GetStates() const
{
    return PIECE_COLS*PIECE_ROWS;
}

CPoint SSkinPiece::GetCenter() const
{
    if(m_ptCenter.x != -1)
        return m_ptCenter;
    SIZE szImg = GetImage()->Size();
    SIZE szChess = {szImg.cx/PIECE_COLS,szImg.cy/PIECE_ROWS};
    m_ptCenter = CPoint(szChess.cx/2,szChess.cy/2);
    return m_ptCenter;
}

void SSkinPiece::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage())
        return;

    int iCol = iState % PIECE_COLS;
    int iRow = iState / PIECE_COLS;

    if(iCol == 1){
        pRT->DrawBitmapEx(rcDraw, m_pImgShadow, NULL, GetExpandMode(), byAlpha);
    }
    SIZE szImg = GetImage()->Size();
    SIZE szChess = {szImg.cx/PIECE_COLS,szImg.cy/PIECE_ROWS};
    CRect rcSrc(CPoint(),szChess);
    OffsetRect(&rcSrc, iCol * szChess.cx, iRow * szChess.cy);
    CRect rcDst = *rcDraw;
    rcDst.bottom = rcDst.top + szChess.cy;
    pRT->DrawBitmapEx(&rcDst, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
}
SNSEND
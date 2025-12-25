#include "stdafx.h"
#include "CnchessSkin.h"

SNSBEGIN

SSkinPiece::SSkinPiece(void)
    : m_ptCenter(38,38)
    , m_szChess(76,76)
{
}

SIZE SSkinPiece::GetSkinSize() const
{
    return m_szChess;
}

int SSkinPiece::GetStates() const
{
    return PIECE_COLS*PIECE_ROWS;
}


CPoint SSkinPiece::GetCenter() const
{
    return m_ptCenter;
}

void SSkinPiece::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage())
        return;

    int iCol = iState % PIECE_COLS;
    int iRow = iState / PIECE_COLS;
    SIZE szImg = GetImage()->Size();
    SIZE szChess = {szImg.cx/PIECE_COLS,szImg.cy/PIECE_ROWS};
    CRect rcSrc(CPoint(),szChess);
    rcSrc.OffsetRect(iCol * szChess.cx, iRow * szChess.cy);
    CRect rcDst = *rcDraw;
    rcDst.bottom = rcDst.top + rcDst.Width();
    pRT->DrawBitmapEx(&rcDst, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
}
SNSEND
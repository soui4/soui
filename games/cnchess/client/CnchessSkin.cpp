#include "stdafx.h"
#include "CnchessSkin.h"

SNSBEGIN

SSkinPiece::SSkinPiece(void)
    : m_ptCenter(-1, -1)
{
}

SIZE SSkinPiece::GetSkinSize() const
{
    return m_pImgShadow->Size();
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

void SSkinPiece::_Scale(ISkinObj *skinObj, int nScale)
{
    __baseCls::_Scale(skinObj, nScale);
    SSkinPiece *pRet = sobj_cast<SSkinPiece>(skinObj);
    CSize szSkin = m_pImgShadow->Size(); 
    szSkin.cx = MulDiv(szSkin.cx, nScale, GetScale());
    szSkin.cy = MulDiv(szSkin.cy, nScale, GetScale());
    m_pImgShadow->Scale2(&pRet->m_pImgShadow, szSkin.cx, szSkin.cy, kHigh_FilterLevel);
}

void SSkinPiece::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage())
        return;

    int iCol = iState % PIECE_COLS;
    int iRow = iState / PIECE_COLS;

    if(iCol == 1){
        SIZE sz = m_pImgShadow->Size();
        RECT rcSrc = { 0, 0, sz.cx, sz.cy };
        pRT->DrawBitmapEx(rcDraw, m_pImgShadow, &rcSrc, GetExpandMode(), byAlpha);
    }
    SIZE szImg = GetImage()->Size();
    SIZE szChess = {szImg.cx/PIECE_COLS,szImg.cy/PIECE_ROWS};
    CRect rcSrc(CPoint(),szChess);
    rcSrc.OffsetRect(iCol * szChess.cx, iRow * szChess.cy);
    CRect rcDst = *rcDraw;
    rcDst.bottom = rcDst.top + szChess.cy;
    pRT->DrawBitmapEx(&rcDst, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
}
SNSEND
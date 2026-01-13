#include "stdafx.h"
#include "JunqiPiece.h"

SNSBEGIN

SSkinPiece::SSkinPiece(void)
    : m_ptCenter(15, 14)
    , m_szChess(31, 29)
{
    m_bEnableScale = FALSE;
}

SIZE SSkinPiece::GetSkinSize() const
{
    return m_szChess;
}

int SSkinPiece::GetStates() const
{
    return PIECE_COLS * PIECE_ROWS;
}

CPoint SSkinPiece::GetCenter() const
{
    return m_ptCenter;
}

void SSkinPiece::_DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
    if (!GetImage())
        return;

    int iRow = iState % PIECE_ROWS;
    int iCol = iState / PIECE_ROWS;
    SIZE szImg = GetImage()->Size();
    SIZE szChess = { szImg.cx / PIECE_COLS, szImg.cy / PIECE_ROWS };
    CRect rcSrc(CPoint(), szChess);
    rcSrc.OffsetRect(iCol * szChess.cx, iRow * szChess.cy);
    CRect rcDst = *rcDraw;
    rcDst.bottom = rcDst.top + rcDst.Height();
    pRT->DrawBitmapEx(&rcDst, GetImage(), &rcSrc, GetExpandMode(), byAlpha);
}

//------------------------------------------------------------------
CJunqiPiece::CJunqiPiece()
    : m_fRotate(0.0f)
    , m_chessman(0)
    , m_iColor(0)
    , m_bShow(TRUE)
{
    m_bMsgTransparent = FALSE;
    //m_bClipClient = TRUE;
    m_pos = { 0, 0 };
    m_target = { 0, 0 };
}

CJunqiPiece::~CJunqiPiece()
{
}

void CJunqiPiece::SetChessman(int chessman)
{
    m_chessman = chessman;
    SASSERT(m_chessman >= OR_SAPPER && m_chessman <= OR_FAKE);
    SetIcon((m_iColor * SSkinPiece::PIECE_ROWS) + GetIconIndex());
}

int CJunqiPiece::GetIconIndex() const
{
    int idx = m_chessman - 1;
    if (!m_bShow)
        idx = IDX_FAKE;
    return idx;
}
float CJunqiPiece::CalcRotate(POINT pt, int iBottomColor) const
{
    if (m_iColor % 2 == iBottomColor % 2)
    {
        // self and top
        if (pt.x < 6)
            return 270.0f;
        else if (pt.x > 10)
            return 90.0f;
        else
            return 0.0f;
    }
    else
    {
        // left and right
        if (pt.y < 6)
            return 0.0f;
        else if (pt.y > 10)
            return 180.0f;
        else
            return 90.0f;
    }
}

void CJunqiPiece::SetRotate(float fRotate, BOOL bUpdate)
{
    m_fRotate = fRotate;
    if (bUpdate)
    {
        CRect rcWnd = GetWindowRect();
        SMatrix mtx;
        mtx.setRotate2(fRotate, rcWnd.Width() / 2, rcWnd.Height() / 2);
        SetMatrix(mtx);
    }
}
void CJunqiPiece::OnSize(UINT nType, CSize size)
{
    SWindow::OnSize(nType, size);
    SMatrix mtx;
    mtx.setRotate2(m_fRotate, size.cx / 2, size.cy / 2);
    SetMatrix(mtx);
}

BOOL CJunqiPiece::SetAnimatorValue(IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state)
{
    if (wcscmp(pHolder->GetPropertyName(), WindowProperty::ROTATE) == 0)
    {
        float fRotate;
        pHolder->GetAnimatedValue(fraction, &fRotate);
        SetRotate(fRotate, TRUE);
        return TRUE;
    }
    else
    {
        return __baseCls::SetAnimatorValue(pHolder, fraction, state);
    }
}

SNSEND

#include "stdafx.h"
#include "JunqiPiece.h"

SNSBEGIN
void CRotateImage::SetRotate(float fRotate, BOOL bUpdate)
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

float CRotateImage::CalcRotate(POINT pt, int iBottomColor, int iColor)
{
    if (iColor % 2 == iBottomColor % 2)
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

//---------------------------------------------------------------------
CJunqiPiece::CJunqiPiece()
    :m_chessman(OR_FAKE)
    , m_iColor(0)
    , m_bShow(TRUE)
{
    m_bMsgTransparent = FALSE;
    m_pos = { 0, 0 };
    m_target = { 0, 0 };
}

CJunqiPiece::~CJunqiPiece()
{
}

void CJunqiPiece::SetChessman(int chessman)
{
    SASSERT(m_chessman >= OR_SAPPER && m_chessman <= OR_FAKE);
    m_chessman = chessman;
    InvalidateRect(NULL);
}
void CJunqiPiece::SetColor(int side)
{
    m_iColor = side;
    InvalidateRect(NULL);
}
float CJunqiPiece::CalcRotate(POINT pt, int iBottomColor) const
{
    return CRotateImage::CalcRotate(pt, iBottomColor, m_iColor);
}

void CJunqiPiece::OnSize(UINT nType, CSize size)
{
    SWindow::OnSize(nType, size);
    SMatrix mtx;
    mtx.setRotate2(m_fRotate, size.cx / 2, size.cy / 2);
    SetMatrix(mtx);
}

void CJunqiPiece::OnPaint(IRenderTarget *pRT)
{
    CRect rcWnd = GetWindowRect();
    SASSERT(m_pSkin);
    SASSERT(m_pRankSkin);
    m_pSkin->DrawByIndex(pRT, rcWnd, m_iColor);
    if(m_bShow && m_chessman >= OR_SAPPER && m_chessman < OR_FAKE)
    {
        m_pRankSkin->DrawByIndex(pRT, rcWnd, m_chessman -1);
    }
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

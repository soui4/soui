#include "stdafx.h"
#include "ChessPiece.h"

SNSBEGIN

CChessPiece::CChessPiece()
:m_state(STATE_NORMAL)
,m_chessMan(CHSMAN_BLK_BING)
,m_pos(-1,-1)
,m_target(-1,-1)
{
    SetIcon(m_chessMan*3+m_state);
    m_bMsgTransparent = FALSE;
}

CChessPiece::~CChessPiece()
{
}

BOOL CChessPiece::SetAnimatorValue(IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state){
    if(wcscmp(pHolder->GetPropertyName(), WindowProperty::SCALE) == 0){
        float scale=1.0;
        pHolder->GetAnimatedValue(fraction, &scale);
        CRect rcWnd = GetWindowRect();
        SMatrix mtx;
        mtx.setScale2(scale, scale, rcWnd.Width() / 2.0f, rcWnd.Height() / 2.0f);
        SetMatrix(mtx);
        return TRUE;
    }
    return __baseCls::SetAnimatorValue(pHolder, fraction, state);
}

SNSEND
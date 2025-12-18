#include "stdafx.h"
#include "ChessPiece.h"

SNSBEGIN

CChessPiece::CChessPiece()
:m_state(STATE_NORMAL)
,m_chessMan(CHSMAN_BLK_BING)
{
    SetIcon(m_chessMan*3+m_state);
}

CChessPiece::~CChessPiece()
{
}

SNSEND
#include "stdafx.h"
#include "ChessPiece.h"
#include <helper/slog.h>
#define kLogTag "CChessPiece"

SNSBEGIN

CChessPiece::CChessPiece()
:m_state(STATE_NORMAL)
,m_chessMan(CHSMAN_BLK_BING)
,m_pos(-1,-1)
,m_target(-1,-1)
,m_bLocalMove(TRUE)
{
    SetIcon(m_chessMan*3+m_state);
    m_bMsgTransparent = FALSE;
}

CChessPiece::~CChessPiece()
{
}


SNSEND
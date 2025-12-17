#include "stdafx.h"
#include "ChessBoard.h"

SNSBEGIN

SChessBoard::SChessBoard(void)
{
}

SChessBoard::~SChessBoard(void)
{
}

SIZE SChessBoard::GetBoardSize() const
{
    return GetSkin()->GetSkinSize();
}

void SChessBoard::OnScaleChanged(int scale)
{
    __baseCls::OnScaleChanged(scale);
}
SNSEND
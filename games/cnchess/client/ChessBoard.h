#ifndef CHESSBOARD_H
#define CHESSBOARD_H

SNSBEGIN

class SChessBoard : public SWindow
{
    DEF_SOBJECT(SWindow, L"chessboard")
public:
    SChessBoard(void);
    ~SChessBoard(void);
};

SNSEND

#endif//CHESSBOARD_H
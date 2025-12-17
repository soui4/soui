#ifndef CHESSBOARD_H
#define CHESSBOARD_H

SNSBEGIN

class SChessBoard : public SImageWnd
{
    DEF_SOBJECT(SImageWnd, L"chessboard")
public:
    SChessBoard(void);
    ~SChessBoard(void);

    SIZE GetBoardSize() const;
protected:
    void OnScaleChanged(int scale) override;
};

SNSEND

#endif//CHESSBOARD_H
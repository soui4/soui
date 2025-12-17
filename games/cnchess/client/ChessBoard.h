#ifndef CHESSBOARD_H
#define CHESSBOARD_H

SNSBEGIN

class CChessBoard : public SWindow {
    DEF_SOBJECT(SWindow, L"chessboard")
public:
    CChessBoard(void);

    ~CChessBoard(void);
    
    STDMETHOD_(void, SDispatchMessage)(THIS_ UINT message, WPARAM wParam, LPARAM lParam) OVERRIDE;
};

SNSEND
#endif//CHESSBOARD_H
#ifndef CHESSBOARD_H
#define CHESSBOARD_H

SNSBEGIN

class CChessBoard : public SWindow {
    DEF_SOBJECT(SWindow, L"chessboard")
public:
    CChessBoard(void);

    ~CChessBoard(void);

    int GetChessScale() const { return m_nChsScale; }
public:
    STDMETHOD_(void, SDispatchMessage)(THIS_ UINT message, WPARAM wParam, LPARAM lParam) OVERRIDE;

protected:
    int m_nChsScale;
};

SNSEND
#endif//CHESSBOARD_H
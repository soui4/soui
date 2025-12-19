#ifndef CHESSPIECE_H
#define CHESSPIECE_H

#include <ChessLayout.h>
SNSBEGIN

class CChessPiece : public SImageWnd
{
    DEF_SOBJECT(SImageWnd, L"chesspiece")
public:
    enum {
        STATE_NORMAL = 0,
        STATE_UP,
        STATE_DOWN,
    };
    CChessPiece();
    ~CChessPiece();

    void SetPicesState(int state){
        m_state = state;
        SetIcon(m_chessMan*3+m_state);
    }
    int GetPicesState() const{
        return m_state;
    }

    void SetChessMan(CHESSMAN chessMan){
        m_chessMan = chessMan;
        SetIcon(m_chessMan*3+m_state);
    }
    CHESSMAN GetChessMan() const{
        return m_chessMan;
    }

    BOOL IsRedSide() const{
        return CHSMANSIDE(m_chessMan) == CS_RED;
    }

    void SetPos(POINT pt){
        m_pos = pt;
    }

    POINT GetPos() const{
        return m_pos;
    }

    void SetTarget(POINT pt){
        m_target = pt;
    }

    POINT GetTarget() const{
        return m_target;
    }

public:
    STDMETHOD_(BOOL,SetAnimatorValue)(THIS_ IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state) override;
private:
    int m_state;
    CHESSMAN m_chessMan;
    CPoint m_pos;
    CPoint m_target;
};

SNSEND
#endif//CHESSPIECE_H
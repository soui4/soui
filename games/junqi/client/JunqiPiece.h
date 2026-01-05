#ifndef JUNQIPIECE_H
#define JUNQIPIECE_H

#include <memory>
#include <JunqiLayout.h>
SNSBEGIN

class SSkinPiece : public SSkinImgList
{
	DEF_SOBJECT(SSkinImgList, L"piece")
public:
    enum{
        PIECE_COLS=4,
        PIECE_ROWS=13,
    };
public:
    SSkinPiece(void);
	~SSkinPiece(void){ }
    CPoint GetCenter() const;
   
public:
    STDMETHOD_(SIZE, GetSkinSize)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetStates)(THIS) SCONST OVERRIDE;
protected:
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;
public:
    SOUI_ATTRS_BEGIN()
        ATTR_POINT(L"center", m_ptCenter, TRUE)
        ATTR_SIZE(L"size", m_szChess, TRUE) //chess size, default is 76,86
    SOUI_ATTRS_END()
private:
    CPoint m_ptCenter;
    CSize  m_szChess;
};


class CJunqiPiece : public SImageWnd
{
    DEF_SOBJECT(SImageWnd, L"junqipiece")

    enum{
        IDX_FAKE=12,
    };
    enum
    {
        COLOR_RED=0,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_GRAY,
    };

  public:
    CJunqiPiece();
    ~CJunqiPiece();

    void SetChessman(int chessman);
    int GetChessman() const{
        return m_chessman;
    }

    int GetIconIndex() const;
    void SetColor(int side){
        m_iColor = side;
        SetIcon((m_iColor * SSkinPiece::PIECE_ROWS) + GetIconIndex());
    }    
    int GetColor() const{
        return m_iColor;
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

    void SetShow(BOOL bShow)
    {
        m_bShow = bShow;
    }

    float CalcRotate(POINT pt, int iBottomColor) const;

    void SetRotate(float fRotate,BOOL bUpdate=FALSE);

    void SetMoveResult(int nResult){
        m_nMoveResult = nResult;
    }
    int GetMoveResult() const{
        return m_nMoveResult;
    }
public:
    STDMETHOD_(BOOL, SetAnimatorValue)(THIS_ IPropertyValuesHolder *pHolder, float fraction, ANI_STATE state) OVERRIDE;
public:
    void OnSize(UINT nType, CSize size);
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()
protected:
    float m_fRotate;
    int m_chessman;       // 棋子类型
    int m_iColor;           // 棋子所属方位
    BOOL m_bShow;         // 是否显示
    POINT m_pos;          // 当前位置
    POINT m_target;       // 目标位置
    int  m_nMoveResult;     // 移动结果
};

SNSEND
#endif//JUNQIPIECE_H

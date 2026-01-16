#ifndef JUNQIPIECE_H
#define JUNQIPIECE_H

#include <memory>
#include <JunqiLayout.h>
SNSBEGIN

class CRotateImage : public SImageWnd{
    DEF_SOBJECT(SImageWnd, L"rotateimage")

    public:
        CRotateImage():m_fRotate(0.0f){}
        ~CRotateImage(){}

        void SetRotate(float fRotate,BOOL bUpdate=FALSE);

        float GetRotate() const{
            return m_fRotate;
        }

        static float CalcRotate(POINT pt, int iBottomColor, int iColor);
    protected:
        float m_fRotate;
};

class CJunqiPiece : public CRotateImage
{
    DEF_SOBJECT(CRotateImage, L"junqipiece")

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
    void SetColor(int side);
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
    void OnPaint(IRenderTarget *pRT);
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_SIZE(OnSize)
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

public:
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"rankSkin", m_pRankSkin, TRUE)
    SOUI_ATTRS_END()
protected:
    SAutoRefPtr<ISkinObj> m_pRankSkin;
    int m_chessman;       // 棋子类型
    int m_iColor;           // 棋子所属方位
    BOOL m_bShow;         // 是否显示
    POINT m_pos;          // 当前位置
    POINT m_target;       // 目标位置
    int  m_nMoveResult;     // 移动结果
};

SNSEND
#endif//JUNQIPIECE_H

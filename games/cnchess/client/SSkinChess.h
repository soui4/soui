#ifndef __SSKINCHESS_H__
#define __SSKINCHESS_H__

#include <core/SSkin.h>

SNSBEGIN

class SSkinChess : public SSkinImgList
{
	DEF_SOBJECT(SSkinImgList, L"chess")

    enum{
        CHESS_COLS=3,
        CHESS_ROWS=14,
    };
public:
	SSkinChess(void):m_ptCenter(-1,-1){}
	~SSkinChess(void){ }
    CPoint GetCenter() const;
   
public:
    STDMETHOD_(SIZE, GetSkinSize)(THIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetStates)(THIS) SCONST OVERRIDE;
protected:
    void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const override;
public:
    SOUI_ATTRS_BEGIN()
        ATTR_POINT(L"center", m_ptCenter, TRUE)
        ATTR_IMAGE(L"shadow", m_pImgShadow, FALSE)
    SOUI_ATTRS_END()
private:
    SAutoRefPtr<IBitmapS> m_pImgShadow;
    mutable CPoint m_ptCenter;
};

SNSEND

#endif//__SSKINCHESS_H__
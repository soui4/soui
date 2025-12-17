#ifndef __SSKINPIECE_H__
#define __SSKINPIECE_H__

#include <core/SSkin.h>

SNSBEGIN

class SSkinPiece : public SSkinImgList
{
	DEF_SOBJECT(SSkinImgList, L"piece")

    enum{
        PIECE_COLS=3,
        PIECE_ROWS=14,
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
    void _Scale(ISkinObj *skinObj, int nScale) override;
public:
    SOUI_ATTRS_BEGIN()
        ATTR_POINT(L"center", m_ptCenter, TRUE)
        ATTR_IMAGE(L"shadow", m_pImgShadow, FALSE)
    SOUI_ATTRS_END()
private:
    SAutoRefPtr<IBitmapS> m_pImgShadow;
    mutable CPoint m_ptCenter;
};


class SSkinBoard : public SSkinImgList
{
	DEF_SOBJECT(SSkinImgList, L"board")
public:
    SSkinBoard(void){}
    ~SSkinBoard(void){ }

    CRect GetMargin() const{
        return m_rcMargin;
    }
public:
    //disable scale
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) OVERRIDE{
        return NULL;
    }
public:
    SOUI_ATTRS_BEGIN()
        ATTR_RECT(L"margin", m_rcMargin, TRUE)
    SOUI_ATTRS_END()

    CRect m_rcMargin;
};

SNSEND

#endif//__SSKINPIECE_H__
#ifndef JUNQIBOARD_H
#define JUNQIBOARD_H
#include <event/SEvents.h>
SNSBEGIN


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


DEF_EVT_EXT(EvtGridClick, EVT_EXTERNAL_BEGIN+1000,
{
    POINT pt;
}
)

class CJunqiBoardGrid: public SImageWnd{
    DEF_SOBJECT(SImageWnd, L"boardgrid")
public:
    CJunqiBoardGrid(void);
    ~CJunqiBoardGrid(void);

protected:
    void OnLButtonDown(UINT nFlags, CPoint pt);
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
    SOUI_MSG_MAP_END()
};

class CJunqiBoard : public SWindow {
    DEF_SOBJECT(SWindow, L"junqiboard")
public:
    CJunqiBoard(void);

    ~CJunqiBoard(void);

    int GetChessScale() const { return m_nChsScale; }
public:
    STDMETHOD_(void, SDispatchMessage)(THIS_ UINT message, WPARAM wParam, LPARAM lParam) OVERRIDE;

protected:
    int m_nChsScale;
};


SNSEND
#endif//JUNQIBOARD_H
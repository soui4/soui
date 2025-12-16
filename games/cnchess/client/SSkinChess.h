#ifndef __SSKINCHESS_H__
#define __SSKINCHESS_H__

#include <core/SSkin.h>

SNSBEGIN

class SSkinChess : public SSkinImgList
{
	DEF_SOBJECT(SSkinImgList, L"chess")
public:
	SSkinChess(void):m_ptCenter(-1,-1){}
	~SSkinChess(void){ }
    CPoint GetCenter() const { return m_ptCenter; }
   
public:
    SOUI_ATTRS_BEGIN()
        ATTR_POINT(L"center", m_ptCenter, TRUE)
    SOUI_ATTRS_END()
private:
    CPoint m_ptCenter;
};

SNSEND

#endif//__SSKINCHESS_H__
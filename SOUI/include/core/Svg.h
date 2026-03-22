#ifndef __SVG__H__
#define __SVG__H__
#include <interface/SRender-i.h>
#include <helper/obj-ref-impl.hpp>
#include <soui_exp.h>
SNSBEGIN

class SOUI_EXP SvgObj : public TObjRefImpl<ISvgObj>
{
    void * m_ptr;
public:
    SvgObj():m_ptr(NULL){}
    virtual ~SvgObj();

public:
    STDMETHOD_(BOOL,Load)(THIS_ char* strSvg) OVERRIDE;
    STDMETHOD_(void,Free)(THIS) OVERRIDE;
    STDMETHOD_(void *, GetPtr)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetWidth)(CTHIS) SCONST OVERRIDE;
    STDMETHOD_(int, GetHeight)(CTHIS) SCONST OVERRIDE;
};

EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgObj(char *strSvg);

EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgFromResId(LPCTSTR resId);

SNSEND


#endif // !__SVG__H__
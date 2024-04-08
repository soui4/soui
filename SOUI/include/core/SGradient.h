#ifndef _SGRADIENT_H_
#define _SGRADIENT_H_

#include <interface/SGradient-i.h>
#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>
#include <souicoll.h>

SNSBEGIN
class SOUI_EXP SGradient : public TObjRefImpl<SObjectImpl<IGradient>> {
    DEF_SOBJECT(SObjectImpl<IGradient>, L"gradient")
  public:
    SGradient();

  public:
    STDMETHOD_(const GradientItem *, GetGradientData)(CTHIS) SCONST OVERRIDE;

    STDMETHOD_(int, GetGradientLength)(CTHIS) SCONST OVERRIDE;

  protected:
    STDMETHOD_(void,OnInitFinished)(THIS_ IXmlNode *pNode) OVERRIDE;

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"colors", OnAttrColors)
    SOUI_ATTRS_END()
  protected:
    HRESULT OnAttrColors(const SStringW &value, BOOL bLoading);
    int LoadColorTable(IXmlNode *xmlNode);

  protected:
    SArray<GradientItem> m_arrGradient;
};

SNSEND

#endif //_SGRADIENT_H_
#ifndef __SOBJDEFATTR__H__
#define __SOBJDEFATTR__H__

#include "core/SSingletonMap.h"

SNSBEGIN

class SOUI_EXP SObjDefAttr
    : public SCmnMap<SXmlNode, SStringW>
    , public TObjRefImpl<IObjRef> {
  public:
    SObjDefAttr()
    {
    }
    virtual ~SObjDefAttr()
    {
    }

    BOOL Init(SXmlNode xmlNode);

    bool IsEmpty()
    {
        return !!m_xmlRoot.root();
    }

    SXmlNode GetDefAttribute(LPCWSTR pszClassName);

  protected:
    void BuildClassAttribute(SXmlNode &xmlNode, LPCWSTR pszClassName);

    SXmlDoc m_xmlRoot;
};

SNSEND

#endif // __SOBJDEFATTR__H__
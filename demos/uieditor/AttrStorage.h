
#ifndef _ATTR_STORAGE_H_
#define _ATTR_STORAGE_H_

#include <interface/SAttrStorage-i.h>
#include <map>

SNSBEGIN


class SAttrStorage : public TObjRefImpl<IAttrStorage>
{
public:
    SAttrStorage(void);
    virtual ~SAttrStorage(void);

    STDMETHOD_(void, OnSetAttribute)(THIS_ const IStringW *strName, const IStringW *strValue, BOOL bHandled) OVERRIDE;

    STDMETHOD_(BOOL, OnGetAttribute)(CTHIS_ const IStringW *strName, IStringW *strValue) SCONST OVERRIDE;

private:
    std::map<SStringW,SStringW> m_mapAttr;
};

class SAttrStorageFactory : public TObjRefImpl<IAttrStorageFactory>
{ 
public:
    STDMETHOD_(HRESULT, CreateAttrStorage)(CTHIS_ IWindow * owner, IAttrStorage * *ppAttrStorage) SCONST OVERRIDE;
};

SNSEND

#endif//_ATTR_STORAGE_H_
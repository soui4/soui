#include "stdafx.h"
#include "AttrStorage.h"

SNSBEGIN

SAttrStorage::SAttrStorage(void)
{
}

SAttrStorage::~SAttrStorage(void)
{
}

void SAttrStorage::OnSetAttribute(THIS_ const IStringW *strName, const IStringW *strValue, BOOL bHandled){
    SStringW strNameW(strName);
    SStringW strValueW(strValue);
    m_mapAttr[strNameW] = strValueW;
}

BOOL SAttrStorage::OnGetAttribute(CTHIS_ const IStringW *strName, IStringW *strValue) const{
    SStringW strNameW(strName);
    std::map<SStringW,SStringW>::const_iterator it = m_mapAttr.find(strNameW);
    if(it!=m_mapAttr.end())
    {
        strValue->Copy(&it->second);
        return TRUE;
    }
    return FALSE;
}

HRESULT SAttrStorageFactory::CreateAttrStorage(CTHIS_ IWindow * owner, IAttrStorage * *ppAttrStorage) const
{
    *ppAttrStorage = new SAttrStorage();
    return S_OK;
}

SNSEND
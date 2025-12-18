#include "souistd.h"
#include <helper/SwndFinder.h>
#include <core/SWnd.h>

SNSBEGIN

SFindInfo::SFindInfo(IWindow *pParent, const SStringW &_strName, int _nDeep)
    : hParent(pParent->GetSwnd())
    , strName(_strName)
    , nDeep(_nDeep)
    , findByName(true)
{
}

SFindInfo::SFindInfo(IWindow *pParent, int _nID, int _nDeep)
    : hParent(pParent->GetSwnd())
    , nID(_nID)
    , nDeep(_nDeep)
    , findByName(false)
{
}

//////////////////////////////////////////////////////////////////////////
IWindow *SWindowFinder::FindChildByName(IWindow *pParent, LPCWSTR strName, int nDeep)
{
    SFindInfo fi(pParent, strName, nDeep);
    return FindChildByKey(pParent, fi);
}

IWindow *SWindowFinder::FindChildByID(IWindow *pParent, int nID, int nDeep)
{
    SFindInfo fi(pParent, nID, nDeep);
    return FindChildByKey(pParent, fi);
}

IWindow *SWindowFinder::FindChildByKey(IWindow *pParent, const SFindInfo &fi)
{
    FINDCACHE::CPair *pFind = m_findCache.Lookup(fi);
    if (!pFind)
    {
        return NULL;
    }
    else
    {
        SWindow *pRet = SWindowMgr::GetWindow(pFind->m_value);
        if (!pRet)
        {
            m_findCache.RemoveKey(fi);
        }
        return pRet;
    }
}

void SWindowFinder::CacheResultForName(IWindow *pParent, LPCWSTR strName, int nDeep, IWindow *pResult)
{
    SFindInfo fi(pParent, strName, nDeep);
    SASSERT(m_findCache.Lookup(fi) == NULL);
    SASSERT(pResult);
    m_findCache[fi] = pResult->GetSwnd();
}

void SWindowFinder::CacheResultForID(IWindow *pParent, int nID, int nDeep, IWindow *pResult)
{
    SFindInfo fi(pParent, nID, nDeep);
    SASSERT(m_findCache.Lookup(fi) == NULL);
    SASSERT(pResult);
    m_findCache[fi] = pResult->GetSwnd();
}

BOOL SWindowFinder::EraseCacheForID(IWindow *pParent, int nID, int nDeep)
{
    SFindInfo fi(pParent, nID, nDeep);
    return m_findCache.RemoveKey(fi);
}

BOOL SWindowFinder::EraseCacheForName(IWindow *pParent, LPCWSTR strName, int nDeep)
{
    SFindInfo fi(pParent, strName, nDeep);
    return m_findCache.RemoveKey(fi);
}
SNSEND

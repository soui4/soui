#include "souistd.h"
#include "core/SWindowMgr.h"

SNSBEGIN

//////////////////////////////////////////////////////////////////////////
SWindowMgr::SWindowMgr()
    : m_hNextWnd(SWND_INVALID)
{
}

SWindowMgr::~SWindowMgr()
{
}

// Get SWindow pointer from handle
SWindow *SWindowMgr::GetWindow(SWND swnd)
{
    if (!swnd)
        return NULL;
    SAutoLock lock(getSingleton().m_lockWndMap);
    SWindow *pRet = NULL;
    getSingleton().GetKeyObject(swnd, pRet);
    return pRet;
}

bool SWindowMgr::IsWindow(SWND swnd)
{
    return GetWindow(swnd) != NULL;
}

// Specify a handle to a SWindow
SWND SWindowMgr::NewWindow(SWindow *pSwnd)
{
    SASSERT(pSwnd);
    SAutoLock lock(getSingleton().m_lockWndMap);
    SWND swndNext = ++getSingleton().m_hNextWnd;
    getSingleton().AddKeyObject(swndNext, pSwnd);
    return swndNext;
}

// Destroy SWindow
BOOL SWindowMgr::DestroyWindow(SWND swnd)
{
    SAutoLock lock(getSingleton().m_lockWndMap);
    return getSingleton().RemoveKeyObject(swnd);
}

SNSEND
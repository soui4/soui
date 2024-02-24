#include <include/souistd.h>
#include <helper/SHostMgr.h>

SNSBEGIN
SHostMgr::SHostMgr(void)
{
}

SHostMgr::~SHostMgr(void)
{
}

bool SHostMgr::AddHostMsgHandler(HWND hHost)
{
    SAutoLock autoLock(m_cs);
    if (m_mapHostMsgHandler.Lookup(hHost))
        return false;
    m_mapHostMsgHandler[hHost] = true;
    return true;
}

bool SHostMgr::RemoveHostMsgHandler(HWND hHost)
{
    SAutoLock autoLock(m_cs);
    return m_mapHostMsgHandler.RemoveKey(hHost);
}

void SHostMgr::DispatchMessage(UINT uMsg, WPARAM wp, LPARAM lp)
{
    SAutoLock autoLock(m_cs);
    SPOSITION pos = m_mapHostMsgHandler.GetStartPosition();
    while (pos)
    { // can't call SHostWnd.Fun for multiple ui threads.
        HWND hHost = m_mapHostMsgHandler.GetNextKey(pos);
        ::PostMessage(hHost, uMsg, wp, lp);
    }
}

SNSEND

#ifndef __SHOSTMGR__H__
#define __SHOSTMGR__H__
#include <core/SSingleton2.h>

SNSBEGIN
class SOUI_EXP SHostMgr : public SSingleton2<SHostMgr> {
    SINGLETON2_TYPE(SINGLETON_HOSTMGR)
    friend class SHostWnd;

  public:
    SHostMgr(void);
    ~SHostMgr(void);

    void DispatchMessage(UINT uMsg, WPARAM wp = 0, LPARAM lp = 0);

  protected:
    bool AddHostMsgHandler(HWND hHost);

    bool RemoveHostMsgHandler(HWND hHost);

  protected:
    SMap<HWND, bool> m_mapHostMsgHandler;
    SCriticalSection m_cs;
};
SNSEND

#endif // __SHOSTMGR__H__
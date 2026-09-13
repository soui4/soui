#ifndef __SHOSTMGR__H__
#define __SHOSTMGR__H__

#include <core/SSingleton2.h>

SNSBEGIN

/**
 * @class SHostMgr
 * @brief Singleton class managing host window message handling
 */
class SOUI_EXP SHostMgr : public SSingleton2<SHostMgr> {
    SINGLETON2_TYPE(SINGLETON_HOSTMGR)
    friend class SHostWnd;

  public:
    /**
     * @brief Constructor
     */
    SHostMgr(void);

    /**
     * @brief Destructor
     */
    ~SHostMgr(void);

    /**
     * @brief Dispatch messages to the host window
     * @param uMsg message ID
     * @param wp additional parameter 1
     * @param lp additional parameter 2
     */
    void DispatchMessage(UINT uMsg, WPARAM wp = 0, LPARAM lp = 0);

  protected:
    /**
     * @brief Add a host window message handler
     * @param hHost host window handle
     * @return Returns TRUE on success, FALSE on failure
     */
    bool AddHostMsgHandler(HWND hHost);

    /**
     * @brief Remove a host window message handler
     * @param hHost host window handle
     * @return Returns TRUE on success, FALSE on failure
     */
    bool RemoveHostMsgHandler(HWND hHost);

  protected:
    SMap<HWND, bool> m_mapHostMsgHandler; /**<  Host window message handler map */
    SCriticalSection m_cs;                /**<  Critical section object, used for thread synchronization */
};

SNSEND

#endif /**< __SHOSTMGR__H__ */
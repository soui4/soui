#ifndef __SHOSTMGR__H__
#define __SHOSTMGR__H__

#include <core/SSingleton2.h>

SNSBEGIN

/**
 * @class SHostMgr
 * @brief 管理宿主窗口消息处理的单例类
 */
class SOUI_EXP SHostMgr : public SSingleton2<SHostMgr> {
    SINGLETON2_TYPE(SINGLETON_HOSTMGR)
    friend class SHostWnd;

  public:
    /**
     * @brief 构造函数
     */
    SHostMgr(void);

    /**
     * @brief 析构函数
     */
    ~SHostMgr(void);

    /**
     * @brief 分发消息到宿主窗口
     * @param uMsg 消息ID
     * @param wp 附加参数1
     * @param lp 附加参数2
     */
    void DispatchMessage(UINT uMsg, WPARAM wp = 0, LPARAM lp = 0);

  protected:
    /**
     * @brief 添加宿主窗口消息处理程序
     * @param hHost 宿主窗口句柄
     * @return 成功返回TRUE，失败返回FALSE
     */
    bool AddHostMsgHandler(HWND hHost);

    /**
     * @brief 移除宿主窗口消息处理程序
     * @param hHost 宿主窗口句柄
     * @return 成功返回TRUE，失败返回FALSE
     */
    bool RemoveHostMsgHandler(HWND hHost);

  protected:
    SMap<HWND, bool> m_mapHostMsgHandler; ///< 宿主窗口消息处理程序映射表
    SCriticalSection m_cs;                ///< 临界区对象，用于线程同步
};

SNSEND

#endif // __SHOSTMGR__H__
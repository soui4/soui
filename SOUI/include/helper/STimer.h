#ifndef __STIMER__H__
#define __STIMER__H__

#include <interface/STimer-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @class STimer
 * @brief 定时器类
 */
class SOUI_EXP STimer : public TObjRefImpl<ITimer> {
  public:
    /**
     * @brief 构造函数
     * @param pSlot 事件槽对象指针
     */
    STimer(IEvtSlot *pSlot);

    /**
     * @brief 析构函数
     */
    ~STimer(void);

  public:
    /**
     * @brief 启动定时器
     * @param nElapse 定时器间隔时间（毫秒）
     * @param bRepeat 是否重复定时器
     * @param uData 用户数据（默认为0）
     * @return 如果成功启动定时器返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, StartTimer)(THIS_ int nElapse, BOOL bRepeat, LPARAM uData DEF_VAL(0)) OVERRIDE;

    /**
     * @brief 停止定时器
     */
    STDMETHOD_(void, KillTimer)(THIS) OVERRIDE;

  private:
    UINT m_uTimerId;                 // 定时器ID
    SAutoRefPtr<IEvtSlot> m_evtSlot; // 事件槽对象指针
};

SNSEND

#endif // __STIMER__H__
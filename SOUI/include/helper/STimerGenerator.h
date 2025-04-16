#ifndef __STIMERGENERATOR__H__
#define __STIMERGENERATOR__H__

#include "core/SSingletonMap.h"

SNSBEGIN

/**
 * @struct TIMERINFO
 * @brief 定时器信息结构体
 */
struct TIMERINFO
{
    SAutoRefPtr<IEvtSlot> pEvtSlot; // 事件槽对象指针
    BOOL bRepeat;                   // 是否重复定时器
    LPARAM uData;                   // 用户数据
};

/**
 * @class STimerGenerator
 * @brief 定时器生成器类，继承自单例映射类
 */
class STimerGenerator : public SSingletonMap<STimerGenerator, TIMERINFO, UINT_PTR> {
    SINGLETON2_TYPE(SINGLETON_TIMERGENERATOR)

  public:
    /**
     * @brief 析构函数
     */
    ~STimerGenerator();

    /**
     * @brief 设置定时器
     * @param pEvtSlot 事件槽对象指针
     * @param nElapse 定时器间隔时间（毫秒）
     * @param bRepeat 是否重复定时器
     * @param uData 用户数据
     * @return 定时器ID
     */
    UINT SetTimer(IEvtSlot *pEvtSlot, UINT nElapse, BOOL bRepeat, LPARAM uData);

    /**
     * @brief 清除定时器
     * @param uID 定时器ID
     */
    void ClearTimer(UINT_PTR uID);

    /**
     * @brief 定时器回调函数
     * @param hwnd 窗口句柄
     * @param uMsg 消息ID
     * @param idEvent 定时器ID
     * @param dwTime 时间戳
     */
    static VOID CALLBACK _TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
};

SNSEND

#endif // __STIMERGENERATOR__H__
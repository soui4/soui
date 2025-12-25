#ifndef __SVALUEANIMATOR_I__H__
#define __SVALUEANIMATOR_I__H__
#include <interface/SAnimation-i.h>
#include <interface/STimelineHandler-i.h>

SNSBEGIN

typedef struct ITimelineHandlersMgr ITimelineHandlersMgr;
typedef struct IValueAnimator IValueAnimator;

#undef INTERFACE
#define INTERFACE IAnimatorUpdateListener
DECLARE_INTERFACE(IAnimatorUpdateListener)
{
    /**
     * @brief 动画更新时调用
     * @param pAnimator - 动画指针
     * @return void
     */
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator * pAnimator) PURE;
};

#undef INTERFACE
#define INTERFACE IAnimatorListener
DECLARE_INTERFACE(IAnimatorListener)
{
    /**
     * @brief 动画开始时调用
     * @param pAnimator - 动画指针
     * @return void
     */
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * @brief 动画结束时调用（不适用于无限重复的动画）
     * @param pAnimator - 动画指针
     * @return void
     */
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * @brief 动画重复时调用
     * @param pAnimator - 动画指针
     * @return void
     */
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator * pAnimator) PURE;
};

#undef INTERFACE
#define INTERFACE IValueAnimator
DECLARE_INTERFACE_(IValueAnimator, IObject)
{
    DEF_OBJ_BASE(IValueAnimator, ValueAnimator)
#include <interface/SobjectApi.h>

    /**
     * @brief 设置动画时长
     * @param duration - 动画时长（毫秒）
     * @return void
     */
    STDMETHOD_(void, setDuration)(THIS_ long duration) PURE;

    /**
     * @brief 获取动画时长
     * @return long - 动画时长（毫秒）
     */
    STDMETHOD_(long, getDuration)(CTHIS) SCONST PURE;

    /**
     * @brief 获取动画总时长
     * @return long - 动画总时长（毫秒）
     */
    STDMETHOD_(long, getTotalDuration)(CTHIS) SCONST PURE;

    /**
     * @brief 设置动画当前播放时间
     * @param playTime - 播放时间（毫秒）
     * @return void
     */
    STDMETHOD_(void, setCurrentPlayTime)(THIS_ long playTime) PURE;

    /**
     * @brief 设置动画当前播放进度
     * @param fraction - 播放进度（0到1之间）
     * @return void
     */
    STDMETHOD_(void, setCurrentFraction)(THIS_ float fraction) PURE;

    /**
     * @brief 获取动画当前播放时间
     * @return long - 当前播放时间（毫秒）
     */
    STDMETHOD_(long, getCurrentPlayTime)(THIS) PURE;

    /**
     * @brief 获取动画开始延迟时间
     * @return long - 开始延迟时间（毫秒）
     */
    STDMETHOD_(long, getStartDelay)(CTHIS) SCONST PURE;

    /**
     * @brief 设置动画开始延迟时间
     * @param startDelay - 开始延迟时间（毫秒）
     * @return void
     */
    STDMETHOD_(void, setStartDelay)(THIS_ long startDelay) PURE;

    /**
     * @brief 设置动画重复次数
     * @param value - 重复次数
     * @return void
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int value) PURE;

    /**
     * @brief 获取动画重复次数
     * @return int - 重复次数
     */
    STDMETHOD_(int, getRepeatCount)(CTHIS) SCONST PURE;

    /**
     * @brief 设置动画重复模式
     * @param value - 重复模式（RESTART或REVERSE）
     * @return void
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode value) PURE;

    /**
     * @brief 获取动画重复模式
     * @return RepeatMode - 重复模式（RESTART或REVERSE）
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(CTHIS) SCONST PURE;

    /**
     * @brief 添加动画更新监听器
     * @param listener - 监听器指针
     * @return void
     */
    STDMETHOD_(void, addUpdateListener)(THIS_ IAnimatorUpdateListener * listener) PURE;

    /**
     * @brief 移除所有动画更新监听器
     * @return void
     */
    STDMETHOD_(void, removeAllUpdateListeners)(THIS) PURE;

    /**
     * @brief 移除动画更新监听器
     * @param listener - 监听器指针
     * @return void
     */
    STDMETHOD_(void, removeUpdateListener)(THIS_ IAnimatorUpdateListener * listener) PURE;

    /**
     * @brief 设置时间插值器
     * @param value - 插值器指针
     * @return void
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator * value) PURE;

    /**
     * @brief 获取时间插值器
     * @return IInterpolator* - 插值器指针
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(CTHIS) SCONST PURE;

    /**
     * @brief 添加动画监听器
     * @param p - 监听器指针
     * @return void
     */
    STDMETHOD_(void, addListener)(THIS_ IAnimatorListener * p) PURE;

    /**
     * @brief 移除动画监听器
     * @param p - 监听器指针
     * @return void
     */
    STDMETHOD_(void, removeListener)(THIS_ IAnimatorListener * p) PURE;

    /**
     * @brief 开始动画
     * @param pContainer - 时间线管理器指针
     * @return void
     */
    STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr * pContainer) PURE;

    /**
     * @brief 结束动画
     * @return void
     */
    STDMETHOD_(void, end)(THIS) PURE;

    /**
     * @brief 检查动画是否正在运行
     * @return BOOL - 正在运行返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, isRunning)(CTHIS) SCONST PURE;

    /**
     * @brief 检查动画是否已启动
     * @return BOOL - 已启动返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, isStarted)(CTHIS) SCONST PURE;

    /**
     * @brief 反向播放动画
     * @return void
     */
    STDMETHOD_(void, reverse)(THIS) PURE;

    /**
     * @brief 提交动画帧
     * @param frameTime - 帧时间
     * @return TRUE if the animation is finished, FALSE otherwise.
     */
    STDMETHOD_(BOOL, commitAnimationFrame)(THIS_ uint64_t frameTime) PURE;

    /**
     * @brief 获取动画当前进度
     * @return float - 当前进度
     */
    STDMETHOD_(float, getAnimatedFraction)(CTHIS) SCONST PURE;

    /**
     * @brief 克隆动画对象
     * @return IValueAnimator* - 克隆的动画对象指针
     */
    STDMETHOD_(IValueAnimator *, clone)(CTHIS) SCONST PURE;

    /**
     * @brief 复制动画对象
     * @param src - 源动画对象指针
     * @return void
     */
    STDMETHOD_(void, copy)(THIS_ const IValueAnimator *src) PURE;

    /**
     * @brief 评估动画值
     * @param fraction - 动画进度
     * @return void
     */
    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) PURE;

    /**
     * @brief 获取时间线处理器
     * @return ITimelineHandler* - 时间线处理器指针
     */
    STDMETHOD_(ITimelineHandler *, GetTimelineHandler)(CTHIS) SCONST PURE;

    /**
     * @brief 获取用户数据
     * @return LPVOID - 用户数据指针
     */
    STDMETHOD_(LPVOID, GetUserData)(CTHIS) SCONST PURE;

    /**
     * @brief 设置用户数据
     * @param pUserData - 用户数据指针
     * @return void
     */
    STDMETHOD_(void, SetUserData)(THIS_ LPVOID pUserData) PURE;
};

typedef struct IAnimatorGroup IAnimatorGroup;

#undef INTERFACE
#define INTERFACE IAnimatorGroupListerer
DECLARE_INTERFACE(IAnimatorGroupListerer)
{
    /**
     * @brief 动画组结束时调用
     * @param pGroup - 动画组指针
     * @return void
     */
    STDMETHOD_(void, OnAnimatorGroupEnd)(THIS_ IAnimatorGroup * pGroup, int nID) PURE;
};

#undef INTERFACE
#define INTERFACE IAnimatorGroup
DECLARE_INTERFACE_(IAnimatorGroup, IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return long - 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return long - 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 添加动画
     * @param ani - 动画指针
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, AddAnimator)(THIS_ IValueAnimator * ani) PURE;

    /**
     * @brief 移除动画
     * @param ani - 动画指针
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, RemoveAnimator)(THIS_ IValueAnimator * ani) PURE;

    /**
     * @brief 设置动画组监听器
     * @param listener - 监听器指针
     * @return void
     */
    STDMETHOD_(void, SetListener)(THIS_ IAnimatorGroupListerer * listener) PURE;
};

SNSEND
#endif // __SVALUEANIMATOR_I__H__

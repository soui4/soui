/**
 * @file SPropertyAnimator-i.h
 * @brief SOUI属性动画接口定义 - 参考Android属性动画系统设计
 * @author SOUI group
 * @date 2025
 */

#ifndef __SPROPERTY_AMINATOR_I__H__
#define __SPROPERTY_AMINATOR_I__H__

#include <interface/obj-ref-i.h>

SNSBEGIN

typedef struct IWindow IWindow;
typedef struct _LAYOUTSIZE LAYOUTSIZE;

typedef enum _PROP_TYPE
{
    PROP_TYPE_UNKNOWN = 0,
    PROP_TYPE_BYTE,
    PROP_TYPE_SHORT,
    PROP_TYPE_COLORREF,
    PROP_TYPE_INT,
    PROP_TYPE_FLOAT,
    PROP_TYPE_LAYOUT_SIZE,
    PROP_TYPE_VARIANT,
} PROP_TYPE;

/**
 * @interface IPropertyValuesHolder
 * @brief 属性值持有者接口 - 类似Android PropertyValuesHolder
 */
#undef INTERFACE
#define INTERFACE IPropertyValuesHolder
DECLARE_INTERFACE_(IPropertyValuesHolder, IObjRef)
{
    /**
     * @brief Increases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decreases the reference count.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Called when the reference count reaches zero and the object is about to be released.
     * @details This method is intended to perform any necessary cleanup before the object is deleted.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //--------------------------------------------------------------------------
    /**
     * @brief 设置属性名称
     * @param propertyName 属性名称
     */
    STDMETHOD_(void, SetPropertyName)(THIS_ LPCWSTR propertyName) PURE;

    /**
     * @brief 获取属性名称
     * @return 属性名称
     */
    STDMETHOD_(LPCWSTR, GetPropertyName)(CTHIS) SCONST PURE;

    /**
     * @brief 设置字节值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetByteValues)(THIS_ const BYTE *values, int count) PURE;

    /**
     * @brief 设置短整型值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetShortValues)(THIS_ const short *values, int count) PURE;

    /**
     * @brief 颜色值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetColorRefValues)(THIS_ const COLORREF *values, int count) PURE;

    /**
     * @brief 设置浮点数值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetFloatValues)(THIS_ const float *values, int count) PURE;

    /**
     * @brief 设置整数值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetIntValues)(THIS_ const int *values, int count) PURE;

    /**
     * @brief 设置LAYOUTSIZE值
     * @param values 值数组
     * @param count 值数量
     */
    STDMETHOD_(void, SetLayoutSizeValues)(THIS_ const LAYOUTSIZE *values, int count) PURE;

    /**
     * @brief 设置位置值（用于位置相关属性）
     * @param values 值数组（具体类型依赖于布局类型）
     * @param count 值数量
     * @param valueSize 每个值的大小（字节）
     */
    STDMETHOD_(void, SetPositionValues)(THIS_ const void *values, int count, int valueSize) PURE;

    /**
     * @brief 获取当前动画值
     * @param fraction 动画进度(0.0-1.0)
     * @param pValue 输出值指针
     * @return 成功返回TRUE
     */
    STDMETHOD_(BOOL, GetAnimatedValue)(CTHIS_ float fraction, void *pValue) SCONST PURE;

    /**
     * @brief 获取值类型
     * @return 值类型
     */
    STDMETHOD_(PROP_TYPE, GetValueType)(CTHIS) SCONST PURE;

    /**
     * @brief 获取值占用内存大小
     * @return 值数量
     */
    STDMETHOD_(int, GetValueSize)(CTHIS) SCONST PURE;

    /**
     * @brief 获取起始值
     * @param pValue 输出值指针
     */
    STDMETHOD_(void, GetStartValue)(CTHIS_ void *pValue) SCONST PURE;

    /**
     * @brief 获取结束值
     * @param pValue 输出值指针
     */
    STDMETHOD_(void, GetEndValue)(CTHIS_ void *pValue) SCONST PURE;

    /**
     * @brief 将动画进度转换为索引
     * @param fraction 动画进度(0.0-1.0)
     * @param idx 索引值数组（输出）
     * @return 分段内动画进度（0.0-1.0）
     */
    STDMETHOD_(float, Fraction2Index)(CTHIS_ float fraction, int idx[2]) SCONST PURE;

    /**
     * @brief 将动画进度转换为帧索引
     * @param fraction 动画进度(0.0-1.0)
     * @return 帧索引
     */
    STDMETHOD_(int, Fraction2FrameIndex)(CTHIS_ float fraction) SCONST PURE;

    /**
     * @brief 通过索引获取值
     * @param index 索引值
     * @param pValue 输出值指针
     * @param valueSize 值大小
     * @return 成功返回TRUE
     */
    STDMETHOD_(BOOL, GetValueByIndex)(CTHIS_ int index, void *pValue, int valueSize) SCONST PURE;

    /**
     * @brief 获取关键帧数量
     * @return 关键帧数量
     */
    STDMETHOD_(int, GetKeyframeCount)(CTHIS) SCONST PURE;

    /**
     * @brief 设置关键帧权重
     * @param weights 权重数组
     * @param count 权重数量
     * @return 成功返回TRUE,失败返回FALSE(权重数量与关键帧数量不一致)
     * @note 权重数量必须与关键帧数量一致,不设置权重则默认为1.0
     */
    STDMETHOD_(BOOL, SetKeyFrameWeights)(THIS_ const float *weights, int count) PURE;

    /**
     * @brief 获取关键帧权重
     * @param weights 权重数组
     * @param count 权重数量
     * @return 成功返回TRUE,失败返回FALSE(权重数量与关键帧数量不一致)
     */
    STDMETHOD_(BOOL, GetKeyFrameWeights)(CTHIS_ float *weights, int count) SCONST PURE;
};

DECLARE_INTERFACE_(IPropertyAnimator, IValueAnimator)
{
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
    //------------------------------------------------------------------------------

    /**
     * @brief 获取动画目标
     * @return 动画目标
     */
    STDMETHOD_(IWindow *, GetTarget)(CTHIS) SCONST PURE;

    /**
     * @brief 设置动画目标
     * @param target 动画目标
     */
    STDMETHOD_(void, SetTarget)(THIS_ IWindow * target) PURE;

    /**
     * @brief 设置属性值持有者
     * @param pHolder 属性值持有者
     */
    STDMETHOD_(void, SetPropertyValuesHolder)(THIS_ IPropertyValuesHolder * pHolder) PURE;
    /**
     * @brief 设置属性值持有者数组
     * @param pHolders 属性值持有者数组
     * @param count 属性值持有者数量
     */
    STDMETHOD_(void, SetPropertyValuesHolders)(THIS_ IPropertyValuesHolder * *pHolders, int count) PURE;
    /**
     * @brief 获取属性值持有者
     * @param propertyName 属性名称
     * @return 属性值持有者
     */
    STDMETHOD_(IPropertyValuesHolder *, GetPropertyValuesHolderByName)(CTHIS_ LPCWSTR propertyName) SCONST PURE;

    /**
     * @brief 获取属性值持有者
     * @param index 索引
     * @return 属性值持有者
     */
    STDMETHOD_(IPropertyValuesHolder *, GetPropertyValuesHolderByIndex)(CTHIS_ int index) SCONST PURE;

    /**
     * @brief 获取属性值持有者数量
     * @return 属性值持有者数量
     */
    STDMETHOD_(int, GetPropertyValuesHolderCount)(CTHIS) SCONST PURE;
};

SNSEND

#endif // __SPROPERTY_AMINATOR_I__H__

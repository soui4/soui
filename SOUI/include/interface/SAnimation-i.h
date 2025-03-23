/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Abstraction for an Animation that can be applied to Views, Surfaces, or
 * other objects. See the {@link android.view.animation animation package
 * description file}.
 */
#ifndef __SANIMATION_I__H__
#define __SANIMATION_I__H__

#include <interface/sinterpolator-i.h>
#include <interface/sobject-i.h>
#include <interface/STransform-i.h>
#include <stdint.h>
SNSBEGIN

typedef struct IAnimation IAnimation;

#undef INTERFACE
#define INTERFACE IAnimationListener
DECLARE_INTERFACE(IAnimationListener)
{
    /**
     * @brief 通知动画开始
     * @param animation IAnimation* -- 开始的动画
     * @return void
     */
    STDMETHOD_(void, OnAnimationStart)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief 通知动画结束。对于重复次数设置为INFINITE的动画，不会调用此回调。
     * @param animation IAnimation* -- 结束的动画
     * @return void
     */
    STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief 通知动画重复
     * @param animation IAnimation* -- 重复的动画
     * @return void
     */
    STDMETHOD_(void, OnAnimationRepeat)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief 通知动画暂停状态变化
     * @param animation IAnimation* -- 状态变化的动画
     * @param bPaused BOOL -- TRUE: 暂停，FALSE: 继续
     * @return void
     */
    STDMETHOD_(void, OnAnimationPauseChange)(THIS_ IAnimation * animation, BOOL bPaused) PURE;
};

/**
 * <p>An animation listener receives notifications from an animation.
 * Notifications indicate animation related events, such as the end or the
 * repetition of the animation.</p>
 */
typedef enum RepeatMode
{
    RESTART = 1,
    REVERSE = 2,
} RepeatMode;

/**
 * Can be used as the start time to indicate the start time should be the current
 * time when {@link #getTransformation(long, STransformation)} is invoked for the
 * first animation frame. This can is useful for short animations.
 */
enum
{
    START_ON_FIRST_FRAME = -1,
};

typedef enum AniValueType
{
    /**
     * The specified dimension is an ABSOLUTE_VALUE number of pixels.
     */
    ABSOLUTE_VALUE = 0,
    /**
     * The specified dimension holds a float and should be multiplied by the
     * height or width of the parent of the object being animated.
     */
    RELATIVE_TO_SELF = 1,
    /**
     * The specified dimension holds a float and should be multiplied by the
     * height or width of the parent of the object being animated.
     */
    RELATIVE_TO_PARENT = 2,
} AniValueType;

typedef enum ZAdjustment
{
    /**
     * Requests that the content being animated be kept in its current Z
     * order.
     */
    ZORDER_NORMAL = 0,
    /**
     * Requests that the content being animated be forced on top of all other
     * content for the duration of the animation.
     */
    ZORDER_TOP = 1,
    /**
     * Requests that the content being animated be forced under all other
     * content for the duration of the animation.
     */
    ZORDER_BOTTOM = -1
} ZAdjustment;

#undef INTERFACE
#define INTERFACE IAnimation
DECLARE_INTERFACE_(IAnimation, IObject)
{
    DEF_OBJ_BASE(IAnimation, Animation)
#include <interface/SobjectApi.h>

    /**
     * @brief 克隆动画对象
     * @return IAnimation* -- 克隆的动画对象
     */
    STDMETHOD_(IAnimation *, clone)(CTHIS) SCONST PURE;

    /**
     * @brief 复制动画对象
     * @param src const IAnimation* -- 源动画对象
     * @return void
     */
    STDMETHOD_(void, copy)(THIS_ const IAnimation *src) PURE;

    /**
     * @brief 重置动画的初始化状态
     * @return void
     */
    STDMETHOD_(void, reset)(THIS) PURE;

    /**
     * @brief 取消动画。取消动画会调用动画监听器（如果设置了）来通知动画结束。
     * @return void
     */
    STDMETHOD_(void, cancel)(THIS) PURE;

    /**
     * @brief 设置动画的加速曲线。默认为线性插值。
     * @param i IInterpolator* -- 定义加速曲线的插值器
     * @return void
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator * i) PURE;

    /**
     * @brief 设置动画持续时间。持续时间不能为负数。
     * @param durationMillis long -- 持续时间（毫秒）
     * @return void
     */
    STDMETHOD_(void, setDuration)(THIS_ long durationMillis) PURE;

    /**
     * @brief 缩放当前持续时间
     * @param scale float -- 缩放比例
     * @return void
     */
    STDMETHOD_(void, scaleCurrentDuration)(THIS_ float scale) PURE;

    /**
     * @brief 设置动画开始前是否应用动画效果
     * @param bFill BOOL -- TRUE: 应用，FALSE: 不应用
     * @return void
     */
    STDMETHOD_(void, setFillBefore)(THIS_ BOOL bFill) PURE;

    /**
     * @brief 设置动画结束后是否应用动画效果
     * @param bFill BOOL -- TRUE: 应用，FALSE: 不应用
     * @return void
     */
    STDMETHOD_(void, setFillAfter)(THIS_ BOOL bFill) PURE;

    /**
     * @brief 获取动画开始前是否应用动画效果
     * @return BOOL -- TRUE: 应用，FALSE: 不应用
     */
    STDMETHOD_(BOOL, getFillBefore)(CTHIS) SCONST PURE;

    /**
     * @brief 获取动画结束后是否应用动画效果
     * @return BOOL -- TRUE: 应用，FALSE: 不应用
     */
    STDMETHOD_(BOOL, getFillAfter)(CTHIS) SCONST PURE;

    /**
     * @brief 设置动画开始偏移时间
     * @param offset long -- 偏移时间（毫秒）
     * @return void
     */
    STDMETHOD_(void, setStartOffset)(THIS_ long offset) PURE;

    /**
     * @brief 获取动画是否启用填充效果
     * @return BOOL -- TRUE: 启用，FALSE: 不启用
     */
    STDMETHOD_(BOOL, isFillEnabled)(CTHIS) SCONST PURE;

    /**
     * @brief 设置动画是否启用填充效果
     * @param fillEnabled BOOL -- TRUE: 启用，FALSE: 不启用
     * @return void
     */
    STDMETHOD_(void, setFillEnabled)(THIS_ BOOL fillEnabled) PURE;

    /**
     * @brief 设置动画开始时间
     * @param startTimeMillis int64_t -- 开始时间（毫秒）
     * @return void
     */
    STDMETHOD_(void, setStartTime)(THIS_ int64_t startTimeMillis) PURE;

    /**
     * @brief 方便方法，设置动画在第一次调用getTransformation时开始
     * @return void
     */
    STDMETHOD_(void, start)(THIS) PURE;

    /**
     * @brief 方便方法，设置动画在当前时间开始
     * @return void
     */
    STDMETHOD_(void, startNow)(THIS) PURE;

    /**
     * @brief 设置动画结束时的行为
     * @param repeatMode RepeatMode -- RESTART或REVERSE
     * @return void
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode repeatMode) PURE;

    /**
     * @brief 设置动画重复次数
     * @param repeatCount int -- 重复次数
     * @return void
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int repeatCount) PURE;

    /**
     * @brief 设置动画运行时的Z顺序模式
     * @param zAdjustment ZAdjustment -- ZORDER_NORMAL, ZORDER_TOP, 或 ZORDER_BOTTOM
     * @return void
     */
    STDMETHOD_(void, setZAdjustment)(THIS_ ZAdjustment zAdjustment) PURE;

    /**
     * @brief 获取动画的加速曲线类型
     * @return IInterpolator* -- 插值器
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(CTHIS) SCONST PURE;

    /**
     * @brief 获取动画开始时间
     * @return int64_t -- 开始时间（毫秒）
     */
    STDMETHOD_(int64_t, getStartTime)(CTHIS) SCONST PURE;

    /**
     * @brief 获取动画持续时间
     * @return long -- 持续时间（毫秒）
     */
    STDMETHOD_(long, getDuration)(CTHIS) SCONST PURE;

    /**
     * @brief 获取动画开始偏移时间
     * @return long -- 偏移时间（毫秒）
     */
    STDMETHOD_(long, getStartOffset)(CTHIS) SCONST PURE;

    /**
     * @brief 获取动画结束时的行为
     * @return RepeatMode -- RESTART或REVERSE
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(CTHIS) SCONST PURE;

    /**
     * @brief 获取动画重复次数
     * @return int -- 重复次数
     */
    STDMETHOD_(int, getRepeatCount)(CTHIS) SCONST PURE;

    /**
     * @brief 获取动画运行时的Z顺序模式
     * @return ZAdjustment -- ZORDER_NORMAL, ZORDER_TOP, 或 ZORDER_BOTTOM
     */
    STDMETHOD_(ZAdjustment, getZAdjustment)(CTHIS) SCONST PURE;

    /**
     * @brief 绑定动画监听器
     * @param listener IAnimationListener* -- 动画监听器
     * @return void
     */
    STDMETHOD_(void, setAnimationListener)(THIS_ IAnimationListener * listener) PURE;

    /**
     * @brief 计算整个动画可能持续的时间（毫秒）
     * @return long -- 持续时间（毫秒）
     */
    STDMETHOD_(long, computeDurationHint)(CTHIS) SCONST PURE;

    /**
     * @brief 获取指定时间点的变换
     * @param currentTime uint64_t -- 当前时间（毫秒）
     * @param outTransformation ITransformation* -- 输出变换对象
     * @param scale float -- 缩放比例
     * @return BOOL -- TRUE: 动画仍在运行，FALSE: 动画已结束
     */
    STDMETHOD_(BOOL, getTransformation2)
    (THIS_ uint64_t currentTime, ITransformation * outTransformation, float scale) PURE;

    /**
     * @brief 获取指定时间点的变换
     * @param currentTime uint64_t -- 当前时间（毫秒）
     * @param outTransformation ITransformation* -- 输出变换对象
     * @return BOOL -- TRUE: 动画仍在运行，FALSE: 动画已结束
     */
    STDMETHOD_(BOOL, getTransformation)
    (THIS_ uint64_t currentTime, ITransformation * outTransformation) PURE;

    /**
     * @brief 判断动画是否已开始
     * @return BOOL -- TRUE: 已开始，FALSE: 未开始
     */
    STDMETHOD_(BOOL, hasStarted)(CTHIS) SCONST PURE;

    /**
     * @brief 判断动画是否已结束
     * @return BOOL -- TRUE: 已结束，FALSE: 未结束
     */
    STDMETHOD_(BOOL, hasEnded)(CTHIS) SCONST PURE;

    /**
     * @brief 应用变换
     * @param interpolatedTime float -- 插值时间
     * @param t ITransformation* -- 变换对象
     * @return void
     */
    STDMETHOD_(void, applyTransformation)(THIS_ float interpolatedTime, ITransformation *t) PURE;

    /**
     * @brief 判断动画是否改变视图的alpha属性
     * @return BOOL -- TRUE: 改变，FALSE: 不改变
     */
    STDMETHOD_(BOOL, hasAlpha)(CTHIS) SCONST PURE;

    /**
     * @brief 初始化动画
     * @param width int -- 宽度
     * @param height int -- 高度
     * @param parentWidth int -- 父级宽度
     * @param parentHeight int -- 父级高度
     * @param nScale int -- 缩放比例
     * @return void
     */
    STDMETHOD_(void, initialize)
    (THIS_ int width, int height, int parentWidth, int parentHeight, int nScale) PURE;

    /**
     * @brief 设置用户数据
     * @param data ULONG_PTR -- 用户数据
     * @return void
     */
    STDMETHOD_(void, setUserData)(THIS_ ULONG_PTR data) PURE;

    /**
     * @brief 获取用户数据
     * @return ULONG_PTR -- 用户数据
     */
    STDMETHOD_(ULONG_PTR, getUserData)(CTHIS) SCONST PURE;

    /**
     * @brief 暂停动画
     * @return void
     */
    STDMETHOD_(void, pause)(THIS) PURE;

    /**
     * @brief 恢复动画
     * @return void
     */
    STDMETHOD_(void, resume)(THIS) PURE;
};

SNSEND
#endif // __SANIMATION_I__H__



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
#pragma once

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
     * <p>Notifies the start of the animation.</p>
     *
     * @param animation The started animation.
     */
    STDMETHOD_(void, OnAnimationStart)(THIS_ IAnimation * animation) PURE;

    /**
     * <p>Notifies the end of the animation. This callback is not invoked
     * for animations with repeat count set to INFINITE.</p>
     *
     * @param animation The animation which reached its end.
     */
    STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation * animation) PURE;

    /**
     * <p>Notifies the repetition of the animation.</p>
     *
     * @param animation The animation which was repeated.
     */
    STDMETHOD_(void, OnAnimationRepeat)(THIS_ IAnimation * animation) PURE;
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
	#include <interface/SobjectApi.h>

    STDMETHOD_(IAnimation *, clone)(CTHIS) SCONST PURE;

    STDMETHOD_(void, copy)(THIS_ const IAnimation *src) PURE;

    /**
     * Reset the initialization state of this animation.
     *
     * @see #initialize(int, int, int, int)
     */
    STDMETHOD_(void, reset)(THIS) PURE;

    /**
     * Cancel the animation. Cancelling an animation invokes the animation
     * listener, if set, to notify the end of the animation.
     *
     * If you cancel an animation manually, you must call {@link #reset()}
     * before starting the animation again.
     *
     * @see #reset()
     * @see #start()
     * @see #startNow()
     */
    STDMETHOD_(void, cancel)(THIS) PURE;

    /**
     * Sets the acceleration curve for this animation. Defaults to a linear
     * interpolation.
     *
     * @param i The interpolator which defines the acceleration curve
     * @attr ref android.R.styleable#Animation_interpolator
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator * i) PURE;

    /**
     * How long this animation should last. The duration cannot be negative.
     *
     * @param durationMillis Duration in milliseconds
     *
     * @throws java.lang.IllegalArgumentException if the duration is < 0
     *
     * @attr ref android.R.styleable#Animation_duration
     */
    STDMETHOD_(void, setDuration)(THIS_ long durationMillis) PURE;

    /**
     * How much to scale the duration by.
     *
     * @param scale The amount to scale the duration.
     */
    STDMETHOD_(void, scaleCurrentDuration)(THIS_ float scale) PURE;

    STDMETHOD_(void, setFillBefore)(THIS_ BOOL bFill) PURE;

    STDMETHOD_(void, setFillAfter)(THIS_ BOOL bFill) PURE;

    STDMETHOD_(BOOL, getFillBefore)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, getFillAfter)(CTHIS) SCONST PURE;

    STDMETHOD_(void, setStartOffset)(THIS_ long offset) PURE;

    /**
     * If fillEnabled is true, this animation will apply the value of fillBefore.
     *
     * @return true if the animation will take fillBefore into account
     * @attr ref android.R.styleable#Animation_fillEnabled
     */
    STDMETHOD_(BOOL, isFillEnabled)(CTHIS) SCONST PURE;

    /**
     * If fillEnabled is true, the animation will apply the value of fillBefore.
     * Otherwise, fillBefore is ignored and the animation
     * transformation is always applied until the animation ends.
     *
     * @param fillEnabled true if the animation should take the value of fillBefore into account
     * @attr ref android.R.styleable#Animation_fillEnabled
     *
     * @see #setFillBefore(boolean)
     * @see #setFillAfter(boolean)
     */
    STDMETHOD_(void, setFillEnabled)(THIS_ BOOL fillEnabled) PURE;

    /**
     * When this animation should start. When the start time is set to
     * {@link #START_ON_FIRST_FRAME}, the animation will start the first time
     * {@link #getTransformation(long, STransformation)} is invoked. The time passed
     * to this method should be obtained by calling
     * {@link AnimationUtils#currentAnimationTimeMillis()} instead of
     * {@link System#currentTimeMillis()}.
     *
     * @param startTimeMillis the start time in milliseconds
     */
    STDMETHOD_(void, setStartTime)(THIS_ int64_t startTimeMillis) PURE;

    /**
     * Convenience method to start the animation the first time
     * {@link #getTransformation(long, STransformation)} is invoked.
     */
    STDMETHOD_(void, start)(THIS) PURE;

    /**
     * Convenience method to start the animation at the current time in
     * milliseconds.
     */
    STDMETHOD_(void, startNow)(THIS) PURE;

    /**
     * Defines what this animation should do when it reaches the end. This
     * setting is applied only when the repeat count is either greater than
     * 0 or {@link #INFINITE}. Defaults to {@link #RESTART}.
     *
     * @param repeatMode {@link #RESTART} or {@link #REVERSE}
     * @attr ref android.R.styleable#Animation_repeatMode
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode repeatMode) PURE;

    /**
     * Sets how many times the animation should be repeated. If the repeat
     * count is 0, the animation is never repeated. If the repeat count is
     * greater than 0 or {@link #INFINITE}, the repeat mode will be taken
     * into account. The repeat count is 0 by default.
     *
     * @param repeatCount the number of times the animation should be repeated
     * @attr ref android.R.styleable#Animation_repeatCount
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int repeatCount) PURE;

    /**
     * Set the Z ordering mode to use while running the animation.
     *
     * @param zAdjustment The desired mode, one of {@link #ZORDER_NORMAL},
     * {@link #ZORDER_TOP}, or {@link #ZORDER_BOTTOM}.
     * @attr ref android.R.styleable#Animation_zAdjustment
     */
    STDMETHOD_(void, setZAdjustment)(THIS_ ZAdjustment zAdjustment) PURE;

    /**
     * Gets the acceleration curve type for this animation.
     *
     * @return the {@link Interpolator} associated to this animation
     * @attr ref android.R.styleable#Animation_interpolator
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(CTHIS) SCONST PURE;

    /**
     * When this animation should start. If the animation has not startet yet,
     * this method might return {@link #START_ON_FIRST_FRAME}.
     *
     * @return the time in milliseconds when the animation should start or
     *         {@link #START_ON_FIRST_FRAME}
     */
    STDMETHOD_(int64_t, getStartTime)(CTHIS) SCONST PURE;

    /**
     * How long this animation should last
     *
     * @return the duration in milliseconds of the animation
     * @attr ref android.R.styleable#Animation_duration
     */
    STDMETHOD_(long, getDuration)(CTHIS) SCONST PURE;

    /**
     * When this animation should start, relative to StartTime
     *
     * @return the start offset in milliseconds
     * @attr ref android.R.styleable#Animation_startOffset
     */
    STDMETHOD_(long, getStartOffset)(CTHIS) SCONST PURE;

    /**
     * Defines what this animation should do when it reaches the end.
     *
     * @return either one of {@link #REVERSE} or {@link #RESTART}
     * @attr ref android.R.styleable#Animation_repeatMode
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(CTHIS) SCONST PURE;

    /**
     * Defines how many times the animation should repeat. The default value
     * is 0.
     *
     * @return the number of times the animation should repeat, or {@link #INFINITE}
     * @attr ref android.R.styleable#Animation_repeatCount
     */
    STDMETHOD_(int, getRepeatCount)(CTHIS) SCONST PURE;

    /**
     * Returns the Z ordering mode to use while running the animation as
     * previously set by {@link #setZAdjustment}.
     *
     * @return Returns one of {@link #ZORDER_NORMAL},
     * {@link #ZORDER_TOP}, or {@link #ZORDER_BOTTOM}.
     * @attr ref android.R.styleable#Animation_zAdjustment
     */
    STDMETHOD_(ZAdjustment, getZAdjustment)(CTHIS) SCONST PURE;

    /**
     * <p>Binds an animation listener to this animation. The animation listener
     * is notified of animation events such as the end of the animation or the
     * repetition of the animation.</p>
     *
     * @param listener the animation listener to be notified
     */
    STDMETHOD_(void, setAnimationListener)(THIS_ IAnimationListener * listener) PURE;

    /**
     * Compute a hint at how long the entire animation may last, in milliseconds.
     * Animations can be written to cause themselves to run for a different
     * duration than what is computed here, but generally this should be
     * accurate.
     */
    STDMETHOD_(long, computeDurationHint)(CTHIS) SCONST PURE;
    /**
     * Gets the transformation to apply at a specified point in time. Implementations of this
     * method should always replace the specified STransformation or document they are doing
     * otherwise.
     *
     * @param currentTime Where we are in the animation. This is wall clock time.
     * @param outTransformation A transformation object that is provided by the
     *        caller and will be filled in by the animation.
     * @param scale Scaling factor to apply to any inputs to the transform operation, such
     *        pivot points being rotated or scaled around.
     * @return True if the animation is still running
     */
    STDMETHOD_(BOOL, getTransformation2)
    (THIS_ uint64_t currentTime, ITransformation * outTransformation, float scale) PURE;

    /**
     * Gets the transformation to apply at a specified point in time. Implementations of this
     * method should always replace the specified STransformation or document they are doing
     * otherwise.
     *
     * @param currentTime Where we are in the animation. This is wall clock time.
     * @param outTransformation A transformation object that is provided by the
     *        caller and will be filled in by the animation.
     * @return True if the animation is still running
     */
    STDMETHOD_(BOOL, getTransformation)
    (THIS_ int64_t currentTime, ITransformation * outTransformation) PURE;

    /**
     * <p>Indicates whether this animation has started or not.</p>
     *
     * @return true if the animation has started, false otherwise
     */
    STDMETHOD_(BOOL, hasStarted)(CTHIS) SCONST PURE;

    /**
     * <p>Indicates whether this animation has ended or not.</p>
     *
     * @return true if the animation has ended, false otherwise
     */
    STDMETHOD_(BOOL, hasEnded)(CTHIS) SCONST PURE;

    /**
     * Helper for getTransformation. Subclasses should implement this to apply
     * their transforms given an interpolation value.  Implementations of this
     * method should always replace the specified STransformation or document
     * they are doing otherwise.
     *
     * @param interpolatedTime The value of the normalized time (0.0 to 1.0)
     *        after it has been run through the interpolation function.
     * @param t The STransformation object to fill in with the current
     *        transforms.
     */
    STDMETHOD_(void, applyTransformation)(THIS_ float interpolatedTime, ITransformation *t) PURE;

    /**
     * Return true if this animation changes the view's alpha property.
     *
     * @hide
     */
    STDMETHOD_(BOOL, hasAlpha)(CTHIS) SCONST PURE;

    STDMETHOD_(void, initialize)
    (THIS_ int width, int height, int parentWidth, int parentHeight,int nScale) PURE;

    STDMETHOD_(void, setUserData)(THIS_ ULONG_PTR data) PURE;

    STDMETHOD_(ULONG_PTR, getUserData)(CTHIS) SCONST PURE;
};

SNSEND
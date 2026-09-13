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
     * @brief Notify animation start
     * @param animation IAnimation* -- the started animation
     * @return void
     */
    STDMETHOD_(void, OnAnimationStart)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief Notify animation end. This callback is not called for animations with repeat count set to INFINITE.
     * @param animation IAnimation* -- the finished animation
     * @return void
     */
    STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief Notify animation repeat
     * @param animation IAnimation* -- the repeated animation
     * @return void
     */
    STDMETHOD_(void, OnAnimationRepeat)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief Notify animation pause state change
     * @param animation IAnimation* -- the animation whose state changed
     * @param bPaused BOOL -- TRUE: paused, FALSE: resumed
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
    /** The specified dimension is an ABSOLUTE_VALUE number of pixels. */
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
     * @brief Clone the animation object
     * @return IAnimation* -- the cloned animation object
     */
    STDMETHOD_(IAnimation *, clone)(CTHIS) SCONST PURE;

    /**
     * @brief Copy the animation object
     * @param src const IAnimation* -- source animation object
     * @return void
     */
    STDMETHOD_(void, copy)(THIS_ const IAnimation *src) PURE;

    /**
     * @brief Reset the animation's initialization state
     * @return void
     */
    STDMETHOD_(void, reset)(THIS) PURE;

    /**
     * @brief Cancel the animation. Canceling the animation calls the animation listener (if set) to notify animation end.
     * @return void
     */
    STDMETHOD_(void, cancel)(THIS) PURE;

    /**
     * @brief Set the animation's acceleration curve. Linear interpolation by default.
     * @param i IInterpolator* -- interpolator defining the acceleration curve
     * @return void
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator * i) PURE;

    /**
     * @brief Set the animation duration. Duration cannot be negative.
     * @param durationMillis long -- duration (milliseconds)
     * @return void
     */
    STDMETHOD_(void, setDuration)(THIS_ long durationMillis) PURE;

    /**
     * @brief Scale the current duration
     * @param scale float -- scale ratio
     * @return void
     */
    STDMETHOD_(void, scaleCurrentDuration)(THIS_ float scale) PURE;

    /**
     * @brief Set whether to apply the animation effect before the animation starts
     * @param bFill BOOL -- TRUE: apply, FALSE: do not apply
     * @return void
     */
    STDMETHOD_(void, setFillBefore)(THIS_ BOOL bFill) PURE;

    /**
     * @brief Set whether to apply the animation effect after the animation ends
     * @param bFill BOOL -- TRUE: apply, FALSE: do not apply
     * @return void
     */
    STDMETHOD_(void, setFillAfter)(THIS_ BOOL bFill) PURE;

    /**
     * @brief Get whether to apply the animation effect before the animation starts
     * @return BOOL -- TRUE: apply, FALSE: do not apply
     */
    STDMETHOD_(BOOL, getFillBefore)(CTHIS) SCONST PURE;

    /**
     * @brief Get whether to apply the animation effect after the animation ends
     * @return BOOL -- TRUE: apply, FALSE: do not apply
     */
    STDMETHOD_(BOOL, getFillAfter)(CTHIS) SCONST PURE;

    /**
     * @brief Set the animation start offset time
     * @param offset long -- offset time (milliseconds)
     * @return void
     */
    STDMETHOD_(void, setStartOffset)(THIS_ long offset) PURE;

    /**
     * @brief Get whether the animation has fill enabled
     * @return BOOL -- TRUE: enabled, FALSE: not enabled
     */
    STDMETHOD_(BOOL, isFillEnabled)(CTHIS) SCONST PURE;

    /**
     * @brief Set whether the animation enables the fill effect
     * @param fillEnabled BOOL -- TRUE: enabled, FALSE: disabled
     * @return void
     */
    STDMETHOD_(void, setFillEnabled)(THIS_ BOOL fillEnabled) PURE;

    /**
     * @brief Set the animation start time
     * @param startTimeMillis int64_t -- start time (in milliseconds)
     * @return void
     */
    STDMETHOD_(void, setStartTime)(THIS_ int64_t startTimeMillis) PURE;

    /**
     * @brief Convenience method; start the animation on the first call to getTransformation
     * @return void
     */
    STDMETHOD_(void, start)(THIS) PURE;

    /**
     * @brief Convenience method; start the animation at the current time
     * @return void
     */
    STDMETHOD_(void, startNow)(THIS) PURE;

    /**
     * @brief Set the behavior when the animation ends
     * @param repeatMode RepeatMode -- RESTART or REVERSE
     * @return void
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode repeatMode) PURE;

    /**
     * @brief Set the animation repeat count
     * @param repeatCount int -- repeat count
     * @return void
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int repeatCount) PURE;

    /**
     * @brief Set the Z-order mode while the animation is running
     * @param zAdjustment ZAdjustment -- ZORDER_NORMAL, ZORDER_TOP, or ZORDER_BOTTOM
     * @return void
     */
    STDMETHOD_(void, setZAdjustment)(THIS_ ZAdjustment zAdjustment) PURE;

    /**
     * @brief Get the animation's acceleration curve type
     * @return IInterpolator* -- the interpolator
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(CTHIS) SCONST PURE;

    /**
     * @brief Get the animation start time
     * @return int64_t -- start time (in milliseconds)
     */
    STDMETHOD_(int64_t, getStartTime)(CTHIS) SCONST PURE;

    /**
     * @brief Get the animation duration
     * @return long -- duration (in milliseconds)
     */
    STDMETHOD_(long, getDuration)(CTHIS) SCONST PURE;

    /**
     * @brief Get the animation start offset time
     * @return long -- offset time (in milliseconds)
     */
    STDMETHOD_(long, getStartOffset)(CTHIS) SCONST PURE;

    /**
     * @brief Get the behavior when the animation ends
     * @return RepeatMode -- RESTART or REVERSE
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(CTHIS) SCONST PURE;

    /**
     * @brief Get the animation repeat count
     * @return int -- repeat count
     */
    STDMETHOD_(int, getRepeatCount)(CTHIS) SCONST PURE;

    /**
     * @brief Get the Z-order mode while the animation is running
     * @return ZAdjustment -- ZORDER_NORMAL, ZORDER_TOP, or ZORDER_BOTTOM
     */
    STDMETHOD_(ZAdjustment, getZAdjustment)(CTHIS) SCONST PURE;

    /**
     * @brief Bind the animation listener
     * @param listener IAnimationListener* -- the animation listener
     * @return void
     */
    STDMETHOD_(void, setAnimationListener)(THIS_ IAnimationListener * listener) PURE;

    /**
     * @brief Compute the possible total duration of the animation (in milliseconds)
     * @return long -- duration (in milliseconds)
     */
    STDMETHOD_(long, computeDurationHint)(CTHIS) SCONST PURE;

    /**
     * @brief Get the transformation at the specified time point
     * @param currentTime uint64_t -- current time (in milliseconds)
     * @param outTransformation ITransformation* -- output transformation object
     * @param scale float -- scale factor
     * @return BOOL -- TRUE: animation still running, FALSE: animation ended
     */
    STDMETHOD_(BOOL, getTransformation2)
    (THIS_ uint64_t currentTime, ITransformation * outTransformation, float scale) PURE;

    /**
     * @brief Get the transformation at the specified time point
     * @param currentTime uint64_t -- current time (in milliseconds)
     * @param outTransformation ITransformation* -- output transformation object
     * @return BOOL -- TRUE: animation still running, FALSE: animation ended
     */
    STDMETHOD_(BOOL, getTransformation)
    (THIS_ uint64_t currentTime, ITransformation * outTransformation) PURE;

    /**
     * @brief Check whether the animation has started
     * @return BOOL -- TRUE: started, FALSE: not started
     */
    STDMETHOD_(BOOL, hasStarted)(CTHIS) SCONST PURE;

    /**
     * @brief Check whether the animation has ended
     * @return BOOL -- TRUE: ended, FALSE: not ended
     */
    STDMETHOD_(BOOL, hasEnded)(CTHIS) SCONST PURE;

    /**
     * @brief Apply the transformation
     * @param interpolatedTime float -- interpolated time
     * @param t ITransformation* -- transformation object
     * @return void
     */
    STDMETHOD_(void, applyTransformation)(THIS_ float interpolatedTime, ITransformation *t) PURE;

    /**
     * @brief Check whether the animation changes the view's alpha property
     * @return BOOL -- TRUE: changes, FALSE: does not change
     */
    STDMETHOD_(BOOL, hasAlpha)(CTHIS) SCONST PURE;

    /**
     * @brief Initialize the animation
     * @param width int -- width
     * @param height int -- height
     * @param parentWidth int -- parent width
     * @param parentHeight int -- parent height
     * @param nScale int -- scale factor
     * @return void
     */
    STDMETHOD_(void, initialize)
    (THIS_ int width, int height, int parentWidth, int parentHeight, int nScale) PURE;

    /**
     * @brief Set user data
     * @param data ULONG_PTR -- user data
     * @return void
     */
    STDMETHOD_(void, setUserData)(THIS_ ULONG_PTR data) PURE;

    /**
     * @brief Get user data
     * @return ULONG_PTR -- user data
     */
    STDMETHOD_(ULONG_PTR, getUserData)(CTHIS) SCONST PURE;

    /**
     * @brief Pause the animation
     * @return void
     */
    STDMETHOD_(void, pause)(THIS) PURE;

    /**
     * @brief Resume the animation
     * @return void
     */
    STDMETHOD_(void, resume)(THIS) PURE;
};

SNSEND
#endif /**< __SANIMATION_I__H__ */

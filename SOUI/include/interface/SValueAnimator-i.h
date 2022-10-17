#pragma once
#include <interface/SAnimation-i.h>
#include <interface/STimelineHandler-i.h>

SNSBEGIN

typedef struct IValueAnimator IValueAnimator;

#undef INTERFACE
#define INTERFACE IAnimatorUpdateListener
DECLARE_INTERFACE(IAnimatorUpdateListener)
{
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator * pAnimator) PURE;
};

#undef INTERFACE
#define INTERFACE IAnimatorListener
DECLARE_INTERFACE(IAnimatorListener)
{
    /**
     * <p>Notifies the start of the animation.</p>
     *
     * @param animation The started animation.
     */
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * <p>Notifies the end of the animation. This callback is not invoked
     * for animations with repeat count set to INFINITE.</p>
     *
     * @param animation The animation which reached its end.
     */
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * <p>Notifies the repetition of the animation.</p>
     *
     * @param animation The animation which was repeated.
     */
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator * pAnimator) PURE;
};

#undef INTERFACE
#define INTERFACE IValueAnimator
DECLARE_INTERFACE_(IValueAnimator, IObject)
{
#include <interface/SobjectApi.h>

    /**
     * Sets the length of the animation. The default duration is 300 milliseconds.
     *
     * @param duration The length of the animation, in milliseconds. This value cannot
     * be negative.
     * @return IValueAnimator The object called with setDuration(). This return
     * value makes it easier to compose statements together that construct and then set the
     * duration, as in <code>IValueAnimator.ofInt(0, 10).setDuration(500).start()</code>.
     */
    STDMETHOD_(void, setDuration)(THIS_ long duration) PURE;

    /**
     * Gets the length of the animation. The default duration is 300 milliseconds.
     *
     * @return The length of the animation, in milliseconds.
     */
    STDMETHOD_(long, getDuration)(CTHIS) SCONST PURE;

    STDMETHOD_(long, getTotalDuration)(CTHIS) SCONST PURE;

    /**
     * Sets the position of the animation to the specified point in time. This time should
     * be between 0 and the total duration of the animation, including any repetition. If
     * the animation has not yet been started, then it will not advance forward after it is
     * set to this time; it will simply set the time to this value and perform any appropriate
     * actions based on that time. If the animation is already running, then setCurrentPlayTime()
     * will set the current playing time to this value and continue playing from that point.
     *
     * @param playTime The time, in milliseconds, to which the animation is advanced or rewound.
     */
    STDMETHOD_(void, setCurrentPlayTime)(THIS_ long playTime) PURE;

    /**
     * Sets the position of the animation to the specified fraction. This fraction should
     * be between 0 and the total fraction of the animation, including any repetition. That is,
     * a fraction of 0 will position the animation at the beginning, a value of 1 at the end,
     * and a value of 2 at the end of a reversing animator that repeats once. If
     * the animation has not yet been started, then it will not advance forward after it is
     * set to this fraction; it will simply set the fraction to this value and perform any
     * appropriate actions based on that fraction. If the animation is already running, then
     * setCurrentFraction() will set the current fraction to this value and continue
     * playing from that point. {@link Animator.AnimatorListener} events are not called
     * due to changing the fraction; those events are only processed while the animation
     * is running.
     *
     * @param fraction The fraction to which the animation is advanced or rewound. Values
     * outside the range of 0 to the maximum fraction for the animator will be clamped to
     * the correct range.
     */
    STDMETHOD_(void, setCurrentFraction)(THIS_ float fraction) PURE;

    /**
     * Gets the current position of the animation in time, which is equal to the current
     * time minus the time that the animation started. An animation that is not yet started will
     * return a value of zero, unless the animation has has its play time set via
     * {@link #setCurrentPlayTime(long)} or {@link #setCurrentFraction(float)}, in which case
     * it will return the time that was set.
     *
     * @return The current position in time of the animation.
     */
    STDMETHOD_(long, getCurrentPlayTime)(THIS) PURE;

    /**
     * The amount of time, in milliseconds, to delay starting the animation after
     * {@link #start()} is called.
     *
     * @return the number of milliseconds to delay running the animation
     */
    STDMETHOD_(long, getStartDelay)(CTHIS) SCONST PURE;

    /**
     * The amount of time, in milliseconds, to delay starting the animation after
     * {@link #start()} is called. Note that the start delay should always be non-negative. Any
     * negative start delay will be clamped to 0 on N and above.
     *
     * @param startDelay The amount of the delay, in milliseconds
     */
    STDMETHOD_(void, setStartDelay)(THIS_ long startDelay) PURE;

    /**
     * Sets how many times the animation should be repeated. If the repeat
     * count is 0, the animation is never repeated. If the repeat count is
     * greater than 0 or {@link #INFINITE}, the repeat mode will be taken
     * into account. The repeat count is 0 by default.
     *
     * @param value the number of times the animation should be repeated
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int value) PURE;
    /**
     * Defines how many times the animation should repeat. The default value
     * is 0.
     *
     * @return the number of times the animation should repeat, or {@link #INFINITE}
     */
    STDMETHOD_(int, getRepeatCount)(CTHIS) SCONST PURE;

    /**
     * Defines what this animation should do when it reaches the end. This
     * setting is applied only when the repeat count is either greater than
     * 0 or {@link #INFINITE}. Defaults to {@link #RESTART}.
     *
     * @param value {@link #RESTART} or {@link #REVERSE}
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode value) PURE;

    /**
     * Defines what this animation should do when it reaches the end.
     *
     * @return either one of {@link #REVERSE} or {@link #RESTART}
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(CTHIS) SCONST PURE;

    /**
     * Adds a listener to the set of listeners that are sent update events through the life of
     * an animation. This method is called on all listeners for every frame of the animation,
     * after the values for the animation have been calculated.
     *
     * @param listener the listener to be added to the current set of listeners for this animation.
     */
    STDMETHOD_(void, addUpdateListener)(THIS_ IAnimatorUpdateListener * listener) PURE;

    /**
     * Removes all listeners from the set listening to frame updates for this animation.
     */
    STDMETHOD_(void, removeAllUpdateListeners)(THIS) PURE;

    /**
     * Removes a listener from the set listening to frame updates for this animation.
     *
     * @param listener the listener to be removed from the current set of update listeners
     * for this animation.
     */
    STDMETHOD_(void, removeUpdateListener)(THIS_ IAnimatorUpdateListener * listener) PURE;

    /**
     * The time interpolator used in calculating the elapsed fraction of this animation. The
     * interpolator determines whether the animation runs with linear or non-linear motion,
     * such as acceleration and deceleration. The default value is
     * {@link android.view.animation.AccelerateDecelerateInterpolator}
     *
     * @param value the interpolator to be used by this animation. A value of <code>null</code>
     * will result in linear interpolation.
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator * value) PURE;

    /**
     * Returns the timing interpolator that this IValueAnimator uses.
     *
     * @return The timing interpolator for this IValueAnimator.
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(CTHIS) SCONST PURE;

    STDMETHOD_(void, addListener)(THIS_ IAnimatorListener * p) PURE;

    STDMETHOD_(void, removeListener)(THIS_ IAnimatorListener * p) PURE;

    STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr * pContainer) PURE;

	STDMETHOD_(void, end)(THIS) PURE;

    STDMETHOD_(BOOL, isRunning)(CTHIS) SCONST PURE;

    STDMETHOD_(BOOL, isStarted)(CTHIS) SCONST PURE;

    /**
     * Plays the IValueAnimator in reverse. If the animation is already running,
     * it will stop itself and play backwards from the point reached when reverse was called.
     * If the animation is not currently running, then it will start from the end and
     * play backwards. This behavior is only set for the current animation; future playing
     * of the animation will use the default behavior of playing forward.
     */
    STDMETHOD_(void, reverse)(THIS) PURE;

    /**
     * Applies an adjustment to the animation to compensate for jank between when
     * the animation first ran and when the frame was drawn.
     * @hide
     */
    STDMETHOD_(void, commitAnimationFrame)(THIS_ long frameTime) PURE;

    /**
     * Returns the current animation fraction, which is the elapsed/interpolated fraction used in
     * the most recent frame update on the animation.
     *
     * @return Elapsed/interpolated fraction of the animation.
     */
    STDMETHOD_(float, getAnimatedFraction)(CTHIS) SCONST PURE;

    STDMETHOD_(IValueAnimator *, clone)(CTHIS) SCONST PURE;

    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) PURE;
};

SNSEND
/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef __SVALUEANIMATOR__H__
#define __SVALUEANIMATOR__H__

/**
 * @file SValueAnimator.h
 * @brief Provides a simple timing engine for running animations which calculate animated values and set them on target objects.
 *
 * @details This class provides a simple timing engine for running animations which calculate animated values and set them on target objects.
 * There is a single timing pulse that all animations use. It runs in a custom handler to ensure that property changes happen on the UI thread.
 * By default, SValueAnimator uses non-linear time interpolation, via the AccelerateDecelerateInterpolator class, which accelerates into and decelerates
 * out of an animation. This behavior can be changed by calling setInterpolator(TimeInterpolator).
 */

#include <interface/SValueAnimator-i.h>
#include <valueAnimator/TypeEvaluator.h>
#include <sobject/Sobject.hpp>

SNSBEGIN

/**
 * @class SValueAnimator
 * @brief A simple timing engine for running animations which calculate animated values and set them on target objects.
 *
 * @details This class provides a simple timing engine for running animations which calculate animated values and set them on target objects.
 * There is a single timing pulse that all animations use. It runs in a custom handler to ensure that property changes happen on the UI thread.
 * By default, SValueAnimator uses non-linear time interpolation, via the AccelerateDecelerateInterpolator class, which accelerates into and decelerates
 * out of an animation. This behavior can be changed by calling setInterpolator(TimeInterpolator).
 */
class SOUI_EXP SValueAnimator
    : public TObjRefImpl<SObjectImpl<IValueAnimator> >
    , ITimelineHandler {
    DEF_SOBJECT(SObjectImpl<IValueAnimator>, L"valueAnimator")

  protected:
    /**
     * @brief The first time that the animation's animateFrame() method is called.
     * @details This time is used to determine elapsed time (and therefore the elapsed fraction) in subsequent calls to animateFrame().
     * Whenever mStartTime is set, you must also update mStartTimeCommitted.
     */
    uint64_t mStartTime;

    /**
     * @brief Flag indicating whether the start time has been firmly committed.
     * @details When true, the start time has been firmly committed as a chosen reference point in time by which the progress of the animation
     * will be evaluated. When false, the start time may be updated when the first animation frame is committed so as to compensate for jank
     * that may have occurred between when the start time was initialized and when the frame was actually drawn.
     */
    bool mStartTimeCommitted;

    /**
     * @brief Set when setCurrentPlayTime() is called.
     * @details If negative, animation is not currently seeked to a value.
     */
    float mSeekFraction;

    /**
     * @brief Flag to indicate whether this animator is playing in reverse mode.
     * @details This flag is different than mPlayingBackwards, which indicates merely whether the current iteration of the animator is playing in reverse.
     * It is used in corner cases to determine proper end behavior.
     */
    bool mReversing;

    /**
     * @brief Tracks the overall fraction of the animation, ranging from 0 to mRepeatCount + 1.
     */
    float mOverallFraction;

    /**
     * @brief Tracks current elapsed/eased fraction, for querying in getAnimatedFraction().
     * @details This is calculated by interpolating the fraction (range: [0, 1]) in the current iteration.
     */
    float mCurrentFraction;

    /**
     * @brief Tracks the time (in milliseconds) when the last frame arrived.
     */
    uint64_t mLastFrameTime;

    /**
     * @brief Tracks the time (in milliseconds) when the first frame arrived.
     * @details Note the frame may arrive during the start delay.
     */
    uint64_t mFirstFrameTime;

    /**
     * @brief Additional playing state to indicate whether an animator has been start()'d.
     * @details There is some lag between a call to start() and the first animation frame. We should still note that the animation has been started,
     * even if its first animation frame has not yet happened, and reflect that state in isRunning(). Note that delayed animations are different:
     * they are not started until their first animation frame, which occurs after their delay elapses.
     */
    bool mRunning;

    /**
     * @brief Additional playing state to indicate whether an animator has been start()'d, whether or not there is a nonzero startDelay.
     */
    bool mStarted;

    /**
     * @brief Tracks whether we've notified listeners of the onAnimationStart() event.
     * @details This can be complex to keep track of since we notify listeners at different times depending on startDelay and whether start() was
     * called before end().
     */
    bool mStartListenersCalled;

    /**
     * @brief Flag that denotes whether the animation is set up and ready to go.
     * @details Used to set up animation that has not yet been started.
     */
    bool mInitialized;

    /**
     * @brief Flag that tracks whether animation has been requested to end.
     */
    bool mAnimationEndRequested;

    /**
     * @brief How long the animation should last in milliseconds.
     */
    long mDuration;

    /**
     * @brief The amount of time in milliseconds to delay starting the animation after start() is called.
     * @details Note that this start delay is unscaled. When there is a duration scale set on the animator, the scaling factor will be applied to this delay.
     */
    long mStartDelay;

    /**
     * @brief The number of times the animation will repeat.
     * @details The default is 0, which means the animation will play only once.
     */
    int mRepeatCount;

    /**
     * @brief Scaling factor for the duration.
     */
    float sDurationScale;

    /**
     * @brief The type of repetition that will occur when repeatMode is nonzero.
     * @details RESTART means the animation will start from the beginning on every new cycle. REVERSE means the animation will reverse directions on each iteration.
     */
    RepeatMode mRepeatMode;

    /**
     * @brief The time interpolator to be used.
     * @details The elapsed fraction of the animation will be passed through this interpolator to calculate the interpolated fraction, which is then used to calculate the animated values.
     */
    SAutoRefPtr<IInterpolator> mInterpolator;

    /**
     * @brief The set of listeners to be sent update events through the life of an animation.
     */
    SArray<IAnimatorUpdateListener *> mUpdateListeners;
    SArray<IAnimatorListener *> mListeners;

    /**
     * @brief The container managing the timeline handlers.
     */
    ITimelineHandlersMgr *mContainer;

  public:
    /**
     * @brief Creates a new SValueAnimator object.
     * @details This default constructor is primarily for use internally; the factory methods which take parameters are more generally useful.
     */
    SValueAnimator();

    /**
     * @brief Destructor.
     */
    ~SValueAnimator();

    /**
     * @brief Sets the length of the animation.
     * @param duration The length of the animation, in milliseconds. This value cannot be negative.
     * @return SValueAnimator The object called with setDuration(). This return value makes it easier to compose statements together that construct
     * and then set the duration, as in SValueAnimator.ofInt(0, 10).setDuration(500).start().
     */
    STDMETHOD_(void, setDuration)(THIS_ long duration) OVERRIDE;

  private:
    /**
     * @brief Gets the scaled duration of the animation.
     * @return The scaled duration of the animation.
     */
    long getScaledDuration();

  public:
    /**
     * @brief Gets the length of the animation.
     * @return The length of the animation, in milliseconds.
     */
    STDMETHOD_(long, getDuration)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the total duration of the animation, including any repetitions.
     * @return The total duration of the animation, in milliseconds.
     */
    STDMETHOD_(long, getTotalDuration)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the position of the animation to the specified point in time.
     * @param playTime The time, in milliseconds, to which the animation is advanced or rewound.
     */
    STDMETHOD_(void, setCurrentPlayTime)(THIS_ long playTime) OVERRIDE;

    /**
     * @brief Sets the position of the animation to the specified fraction.
     * @param fraction The fraction to which the animation is advanced or rewound. Values outside the range of 0 to the maximum fraction for the animator
     * will be clamped to the correct range.
     */
    STDMETHOD_(void, setCurrentFraction)(THIS_ float fraction) OVERRIDE;

  private:
    /**
     * @brief Calculates current iteration based on the overall fraction.
     * @param fraction The overall fraction of the animation.
     * @return The current iteration.
     */
    int getCurrentIteration(float fraction);

    /**
     * @brief Calculates the fraction of the current iteration.
     * @param fraction The overall fraction of the animation.
     * @param inReverse Whether the animation is playing backwards.
     * @return The fraction of the current iteration.
     */
    float getCurrentIterationFraction(float fraction, bool inReverse);

    /**
     * @brief Clamps fraction into the correct range: [0, mRepeatCount + 1].
     * @param fraction The fraction to be clamped.
     * @return The clamped fraction.
     */
    float clampFraction(float fraction);

    /**
     * @brief Calculates the direction of animation playing (i.e., forward or backward).
     * @param iteration The current iteration.
     * @param inReverse Whether the animation is being reversed.
     * @return TRUE if the animation should play backward, FALSE otherwise.
     */
    bool shouldPlayBackward(int iteration, bool inReverse);

  public:
    /**
     * @brief Gets the current position of the animation in time.
     * @return The current position in time of the animation.
     */
    STDMETHOD_(long, getCurrentPlayTime)(THIS) OVERRIDE;

    /**
     * @brief Gets the amount of time, in milliseconds, to delay starting the animation after start() is called.
     * @return The number of milliseconds to delay running the animation.
     */
    STDMETHOD_(long, getStartDelay)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the amount of time, in milliseconds, to delay starting the animation after start() is called.
     * @param startDelay The amount of the delay, in milliseconds.
     */
    STDMETHOD_(void, setStartDelay)(THIS_ long startDelay) OVERRIDE;

    /**
     * @brief Sets how many times the animation should be repeated.
     * @param value The number of times the animation should be repeated. If the repeat count is 0, the animation is never repeated.
     * If the repeat count is greater than 0 or INFINITE, the repeat mode will be taken into account. The repeat count is 0 by default.
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int value) OVERRIDE;

    /**
     * @brief Defines how many times the animation should repeat.
     * @return The number of times the animation should repeat, or INFINITE.
     */
    STDMETHOD_(int, getRepeatCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Defines what this animation should do when it reaches the end.
     * @param value Either RESTART or REVERSE. Defaults to RESTART.
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode value) OVERRIDE;

    /**
     * @brief Defines what this animation should do when it reaches the end.
     * @return Either REVERSE or RESTART.
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(THIS) SCONST OVERRIDE;

    /**
     * @brief Adds a listener to the set of listeners that are sent update events through the life of an animation.
     * @param listener The listener to be added to the current set of listeners for this animation.
     */
    STDMETHOD_(void, addUpdateListener)(THIS_ IAnimatorUpdateListener *listener) OVERRIDE;

    /**
     * @brief Removes all listeners from the set listening to frame updates for this animation.
     */
    STDMETHOD_(void, removeAllUpdateListeners)(THIS) OVERRIDE;

    /**
     * @brief Removes a listener from the set listening to frame updates for this animation.
     * @param listener The listener to be removed from the current set of update listeners for this animation.
     */
    STDMETHOD_(void, removeUpdateListener)(THIS_ IAnimatorUpdateListener *listener) OVERRIDE;

    /**
     * @brief Sets the time interpolator used in calculating the elapsed fraction of this animation.
     * @param value The interpolator to be used by this animation. A value of nullptr will result in linear interpolation.
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator *value) OVERRIDE;

    /**
     * @brief Returns the timing interpolator that this SValueAnimator uses.
     * @return The timing interpolator for this SValueAnimator.
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(THIS) SCONST OVERRIDE;

    /**
     * @brief Adds an animator listener.
     * @param p The listener to be added.
     */
    STDMETHOD_(void, addListener)(THIS_ IAnimatorListener *p) OVERRIDE;

    /**
     * @brief Removes an animator listener.
     * @param p The listener to be removed.
     */
    STDMETHOD_(void, removeListener)(THIS_ IAnimatorListener *p) OVERRIDE;

  private:
    /**
     * @brief Notifies start listeners.
     */
    void notifyStartListeners();

    /**
     * @brief Starts the animation playing.
     * @param playBackwards Whether the SValueAnimator should start playing in reverse.
     */
    void start(bool playBackwards);

  public:
    /**
     * @brief Starts the animation playing.
     * @param pContainer The container managing the timeline handlers.
     */
    STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr *pContainer) OVERRIDE;

    /**
     * @brief Ends the animation.
     */
    STDMETHOD_(void, end)(THIS) OVERRIDE;

    /**
     * @brief Checks if the animation is running.
     * @return TRUE if the animation is running, FALSE otherwise.
     */
    STDMETHOD_(BOOL, isRunning)(THIS) SCONST OVERRIDE;

    /**
     * @brief Checks if the animation has been started.
     * @return TRUE if the animation has been started, FALSE otherwise.
     */
    STDMETHOD_(BOOL, isStarted)(THIS) SCONST OVERRIDE;

    /**
     * @brief Plays the SValueAnimator in reverse.
     * @details If the animation is already running, it will stop itself and play backwards from the point reached when reverse was called.
     * If the animation is not currently running, then it will start from the end and play backwards. This behavior is only set for the current
     * animation; future playing of the animation will use the default behavior of playing forward.
     */
    STDMETHOD_(void, reverse)(THIS) OVERRIDE;

    /**
     * @brief Creates a clone of the current animation.
     * @return A clone of the current animation.
     */
    STDMETHOD_(IValueAnimator *, clone)(THIS) SCONST OVERRIDE;

    /**
     * @brief Copies the properties of another animation to this animation.
     * @param pSrc The source animation to copy properties from.
     */
    STDMETHOD_(void, copy)(THIS_ const IValueAnimator *pSrc) OVERRIDE;

  private:
    /**
     * @brief Checks if the animation can reverse.
     * @return TRUE if the animation can reverse, FALSE otherwise.
     */
    bool canReverse();

    /**
     * @brief Ends the animation by removing it from the animations list.
     * @details Must be called on the UI thread.
     */
    void endAnimation();

    /**
     * @brief Starts the animation by adding it to the active animations list.
     * @details Must be called on the UI thread.
     */
    void startAnimation();

    /**
     * @brief Checks if the animation is pulsing internally.
     * @details This is different than isRunning() in that the latter tracks the time after start()
     * is called (or after start delay if any), which may be before the animation loop starts.
     * @return TRUE if the animation is pulsing, FALSE otherwise.
     */
    bool isPulsingInternal();

  public:
    /**
     * @brief Applies an adjustment to the animation to compensate for jank between when the animation first ran and when the frame was drawn.
     * @param frameTime The current frame time.
     */
    STDMETHOD_(void, commitAnimationFrame)(THIS_ long frameTime) OVERRIDE;

  private:
    /**
     * @brief Processes a single animation frame for a given animation.
     * @param currentTime The current time, as tracked by the static timing handler.
     * @return TRUE if the animation's duration, including any repetitions due to repeatCount, has been exceeded and the animation should be ended.
     */
    bool animateBasedOnTime(uint64_t currentTime);

    /**
     * @brief Animates the animation based on the current and last play times.
     * @param currentPlayTime The current play time.
     * @param lastPlayTime The last play time.
     * @param inReverse Whether the animation is playing in reverse.
     */
    void animateBasedOnPlayTime(long currentPlayTime, long lastPlayTime, bool inReverse);

    /**
     * @brief Skips the animation value to the end/start, depending on whether the play direction is forward or backward.
     * @param inReverse Whether the end value is based on a reverse direction.
     */
    void skipToEndValue(bool inReverse);

    /**
     * @brief Checks if the animation is initialized.
     * @return TRUE if the animation is initialized, FALSE otherwise.
     */
    bool isInitialized();

    /**
     * @brief Processes a frame of the animation, adjusting the start time if needed.
     * @param frameTime The frame time.
     * @return TRUE if the animation has ended.
     */
    bool doAnimationFrame(uint64_t frameTime);

  public:
    /**
     * @brief Returns the current animation fraction.
     * @details This is the elapsed/interpolated fraction used in the most recent frame update on the animation.
     * @return The elapsed/interpolated fraction of the animation.
     */
    STDMETHOD_(float, getAnimatedFraction)(THIS) SCONST OVERRIDE;

    /**
     * @brief Handles the next frame of the animation.
     */
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

  private:
    /**
     * @brief This method is called with the elapsed fraction of the animation during every animation frame.
     * @details This function turns the elapsed fraction into an interpolated fraction and then into an animated value (from the evaluator).
     * The function is called mostly during animation updates, but it is also called when the end() function is called, to set the value on the property.
     * @param fraction The elapsed fraction of the animation.
     */
    void animateValue(float fraction);

    /**
     * @brief Removes the animation callback.
     */
    void removeAnimationCallback();

    /**
     * @brief Adds the animation callback.
     */
    void addAnimationCallback();
};

/**
 * @class TValueAnimator
 * @brief Template class for creating value animators.
 * @tparam T The type of the animated value.
 */
template <class T>
class TValueAnimator : public SValueAnimator {
  protected:
    /**
     * @brief Type evaluator for the animated value.
     */
    TypeEvaluator<T> mValueEvaluator;

    /**
     * @brief Current animated value.
     */
    T mValue;

  public:
    /**
     * @brief Constructor.
     * @param from Initial value.
     * @param to Final value.
     */
    TValueAnimator(T from, T to)
        : mValueEvaluator(from, to)
    {
    }

    /**
     * @brief Gets the current animated value.
     * @return The current animated value.
     */
    T getValue() const
    {
        return mValue;
    }

    /**
     * @brief Sets the range of the animation.
     * @param from Initial value.
     * @param to Final value.
     */
    void setRange(T from, T to)
    {
        mValueEvaluator.setRange(from, to);
    }

    /**
     * @brief Copies the properties of another animation to this animation.
     * @param pSrc The source animation to copy properties from.
     */
    STDMETHOD_(void, copy)(THIS_ const IValueAnimator *pSrc) OVERRIDE
    {
        SValueAnimator::copy(pSrc);
        const TValueAnimator *src = (const TValueAnimator *)pSrc;
        mValue = src->mValue;
        mValueEvaluator.copy(&src->mValueEvaluator);
    }

  protected:
    /**
     * @brief Evaluates the animated value based on the given fraction.
     * @param fraction The elapsed fraction of the animation.
     */
    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) OVERRIDE
    {
        mValue = mValueEvaluator.evaluate(fraction);
    }

  public:
    /**
     * @brief Defines the attributes for TValueAnimator.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"duration", mDuration, FALSE)
        ATTR_INT(L"repeatCount", mRepeatCount, FALSE)
        ATTR_ENUM_BEGIN(L"repeatMode", RepeatMode, FALSE)
            ATTR_ENUM_VALUE(L"reverse", REVERSE)
            ATTR_ENUM_VALUE(L"restart", RESTART)
        ATTR_ENUM_END(mRepeatMode)
        ATTR_INTERPOLATOR(L"interpolator", mInterpolator, FALSE)
        ATTR_CHAIN_PTR(mInterpolator, 0)
    SOUI_ATTRS_END()
};

/**
 * @class SIntAnimator
 * @brief Animator for integer values.
 */
class SOUI_EXP SIntAnimator : public TValueAnimator<int> {
    DEF_SOBJECT(TValueAnimator<int>, L"IntAnimator")

  public:
    /**
     * @brief Constructor.
     */
    SIntAnimator()
        : TValueAnimator<int>(0, 100)
    {
    }

  public:
    /**
     * @brief Defines the attributes for SIntAnimator.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"valueFrom", mValueEvaluator.mStart, FALSE)
        ATTR_INT(L"valueTo", mValueEvaluator.mEnd, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class SSizeAnimator
 * @brief Animator for SIZE values.
 */
class SOUI_EXP SSizeAnimator : public TValueAnimator<SIZE> {
    DEF_SOBJECT(TValueAnimator<SIZE>, L"SizeAnimator")

  public:
    /**
     * @brief Constructor.
     */
    SSizeAnimator()
        : TValueAnimator<SIZE>(CSize(), CSize())
    {
    }

  public:
    /**
     * @brief Defines the attributes for SSizeAnimator.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_SIZE(L"valueFrom", mValueEvaluator.mStart, FALSE)
        ATTR_SIZE(L"valueTo", mValueEvaluator.mEnd, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class SPointAnimator
 * @brief Animator for POINT values.
 */
class SOUI_EXP SPointAnimator : public TValueAnimator<POINT> {
    DEF_SOBJECT(TValueAnimator<POINT>, L"PointAnimator")

  public:
    /**
     * @brief Constructor.
     */
    SPointAnimator()
        : TValueAnimator<POINT>(CPoint(), CPoint())
    {
    }

  public:
    /**
     * @brief Defines the attributes for SPointAnimator.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_POINT(L"valueFrom", mValueEvaluator.mStart, FALSE)
        ATTR_POINT(L"valueTo", mValueEvaluator.mEnd, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class SRectAnimator
 * @brief Animator for RECT values.
 */
class SOUI_EXP SRectAnimator : public TValueAnimator<RECT> {
    DEF_SOBJECT(TValueAnimator<RECT>, L"RectAnimator")

  public:
    /**
     * @brief Constructor.
     */
    SRectAnimator()
        : TValueAnimator<RECT>(CRect(), CRect())
    {
    }

  public:
    /**
     * @brief Defines the attributes for SRectAnimator.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_RECT(L"valueFrom", mValueEvaluator.mStart, FALSE)
        ATTR_RECT(L"valueTo", mValueEvaluator.mEnd, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class SFloatAnimator
 * @brief Animator for float values.
 */
class SOUI_EXP SFloatAnimator : public TValueAnimator<float> {
    DEF_SOBJECT(TValueAnimator<float>, L"FloatAnimator")

  public:
    /**
     * @brief Constructor.
     */
    SFloatAnimator()
        : TValueAnimator<float>(0.f, 1.f)
    {
    }

  public:
    /**
     * @brief Defines the attributes for SFloatAnimator.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_FLOAT(L"valueFrom", mValueEvaluator.mStart, FALSE)
        ATTR_FLOAT(L"valueTo", mValueEvaluator.mEnd, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class SColorAnimator
 * @brief Animator for COLORREF values.
 */
class SOUI_EXP SColorAnimator : public TValueAnimator<COLORREF> {
    DEF_SOBJECT(TValueAnimator<COLORREF>, L"ColorAnimator")

  public:
    /**
     * @brief Constructor.
     */
    SColorAnimator()
        : TValueAnimator<COLORREF>(0, 0)
    {
    }

  protected:
    /**
     * @brief Handles the "valueFrom" attribute.
     * @param strValue The attribute value as a string.
     * @param bLoading TRUE if the attribute is being loaded, FALSE otherwise.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrFrom(const SStringW &strValue, BOOL bLoading);

    /**
     * @brief Handles the "valueTo" attribute.
     * @param strValue The attribute value as a string.
     * @param bLoading TRUE if the attribute is being loaded, FALSE otherwise.
     * @return HRESULT indicating success or failure.
     */
    HRESULT OnAttrTo(const SStringW &strValue, BOOL bLoading);

  public:
    /**
     * @brief Defines the attributes for SColorAnimator.
     */
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"valueFrom", OnAttrFrom)
        ATTR_CUSTOM(L"valueTo", OnAttrTo)
    SOUI_ATTRS_END()
};

/**
 * @class SAnimatorGroup
 * @brief Manages a group of animations.
 */
class SOUI_EXP SAnimatorGroup
    : public TObjRefImpl<IAnimatorGroup>
    , public IAnimatorListener {
  public:
    /**
     * @enum AniState
     * @brief Enumeration of animation states.
     */
    enum AniState
    {
        idle,
        started,
        running,
    };

    /**
     * @brief Constructor.
     */
    SAnimatorGroup();

    /**
     * @brief Destructor.
     */
    ~SAnimatorGroup();

    /**
     * @brief Adds an animator to the group.
     * @param ani Pointer to the animator to add.
     * @return TRUE if the animator was added successfully, FALSE otherwise.
     */
    STDMETHOD_(BOOL, AddAnimator)(THIS_ IValueAnimator *ani) OVERRIDE;

    /**
     * @brief Removes an animator from the group.
     * @param ani Pointer to the animator to remove.
     * @return TRUE if the animator was removed successfully, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveAnimator)(THIS_ IValueAnimator *ani) OVERRIDE;

    /**
     * @brief Sets the listener for the animator group.
     * @param listener Pointer to the listener.
     */
    STDMETHOD_(void, SetListener)(THIS_ IAnimatorGroupListerer *listener) OVERRIDE;

  protected:
    /**
     * @brief Called when an animation starts.
     * @param pAnimator Pointer to the animator that started.
     */
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator);

    /**
     * @brief Called when an animation repeats.
     * @param pAnimator Pointer to the animator that repeated.
     */
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator)
    {
    }

    /**
     * @brief Called when an animation ends.
     * @param pAnimator Pointer to the animator that ended.
     */
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator);

  protected:
    /**
     * @brief Map of animators and their states.
     */
    typedef SMap<IValueAnimator *, AniState> AnimatorStateMap;

    /**
     * @brief Map of animators and their states.
     */
    AnimatorStateMap m_lstAnimator;

    /**
     * @brief Listener for the animator group.
     */
    IAnimatorGroupListerer *m_listener;
};

SNSEND

#endif // __SVALUEANIMATOR__H__
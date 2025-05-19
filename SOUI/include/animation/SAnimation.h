#ifndef __SANIMATION__H__
#define __SANIMATION__H__

#include <interface/SAnimation-i.h>
#include <helper/obj-ref-impl.hpp>
#include <animation/SInterpolatorImpl.h>
#include <animation/STransformation.h>
#include <sobject/Sobject.hpp>

#define ATTR_VALUE_DESC(attribname, desc)                    \
    if (0 == strAttribName.CompareNoCase(attribname))        \
    {                                                        \
        desc = SNS::SValueDescription::parseValue(strValue); \
        hRet = S_FALSE;                                      \
    }                                                        \
    else

SNSBEGIN

/**
 * @class SValueDescription
 * @brief Utility class to parse a string description of a size.
 */
class SOUI_EXP SValueDescription {
  public:
    /**
     * @brief Constructor for SValueDescription
     * @param _type Type of the value (ABSOLUTE_VALUE, RELATIVE_TO_SELF, RELATIVE_TO_PARENT)
     * @param _value The value to be described
     */
    SValueDescription(AniValueType _type = ABSOLUTE_VALUE, float _value = 0.0f)
        : type(_type)
        , value(_value)
    {
    }

    /**
     * @brief One of Animation.ABSOLUTE_VALUE, Animation.RELATIVE_TO_SELF, or Animation.RELATIVE_TO_PARENT.
     */
    AniValueType type;

    /**
     * @brief The absolute or relative dimension for this Description.
     */
    SLayoutSize value;

    /**
     * @brief Parses a string description of a size.
     * Size descriptions can appear in three forms:
     * <ol>
     * <li>An absolute size. This is represented by a number.</li>
     * <li>A size relative to the size of the object being animated. This is represented by a number followed by "%".</li>
     * <li>A size relative to the size of the parent of the object being animated. This is represented by a number followed by "%p".</li>
     * </ol>
     * @param value The typed value to parse
     * @return The parsed version of the description
     */
    static SValueDescription parseValue(const SStringW &value);
};

/**
 * @class SAnimation
 * @brief Abstraction for an Animation that can be applied to Views, Surfaces, or other objects.
 *
 * See the {@link android.view.animation animation package description file}.
 */
class SOUI_EXP SAnimation : public TObjRefImpl<SObjectImpl<IAnimation>> {
    DEF_SOBJECT(SObjectImpl<IAnimation>, L"animation")

  protected:
    /**
     * @brief Set by {@link #getTransformation(long, STransformation)} when the animation ends.
     */
    bool mEnded;

    /**
     * @brief Set by {@link #getTransformation(long, STransformation)} when the animation starts.
     */
    bool mStarted;

    /**
     * @brief Set by {@link #getTransformation(long, STransformation)} when the animation repeats in REVERSE mode.
     */
    bool mCycleFlip;

    /**
     * @brief The time in milliseconds at which the animation must start.
     */
    uint64_t mStartTime;

    /**
     * @brief The delay in milliseconds after which the animation must start.
     * When the start offset is > 0, the start time of the animation is startTime + startOffset.
     */
    long mStartOffset;

    /**
     * @brief The duration of one animation cycle in milliseconds.
     */
    long mDuration;

    /**
     * @brief The number of times the animation must repeat. By default, an animation repeats indefinitely.
     */
    int mRepeatCount;

    /**
     * @brief Indicates how many times the animation was repeated.
     */
    int mRepeated;

    /**
     * @brief The behavior of the animation when it repeats. The repeat mode is either {@link #RESTART} or {@link #REVERSE}.
     */
    RepeatMode mRepeatMode;

    /**
     * @brief The interpolator used by the animation to smooth the movement.
     */
    SAutoRefPtr<IInterpolator> mInterpolator;

    /**
     * @brief The animation listener to be notified when the animation starts, ends, or repeats.
     */
    IAnimationListener *mListener;

    /**
     * @brief Desired Z order mode during animation.
     */
    ZAdjustment mZAdjustment;

    /**
     * @brief Scale factor to apply to pivot points, etc. during animation. Subclasses retrieve the value via getScaleFactor().
     */
    float mScaleFactor;

    /**
     * @brief Indicates whether the animation transformation should be applied before the animation starts.
     * The value of this variable is only relevant if mFillEnabled is true; otherwise, it is assumed to be true.
     */
    BOOL mFillBefore;

    /**
     * @brief Indicates whether the animation transformation should be applied after the animation ends.
     */
    BOOL mFillAfter;

    /**
     * @brief Indicates whether fillBefore should be taken into account.
     */
    BOOL mFillEnabled;

    /**
     * @brief User data associated with the animation.
     */
    ULONG_PTR mUserData;

    /**
     * @brief Indicates whether the animation is paused.
     */
    bool mPaused;

    /**
     * @brief The time at which the animation was paused.
     */
    uint64_t mPauseTime;

  public:
    /**
     * @brief Default constructor for SAnimation
     */
    SAnimation();

    /**
     * @brief Destructor for SAnimation
     */
    ~SAnimation();

    /**
     * @brief Creates a new animation with a duration of 0ms, the default interpolator, with fillBefore set to true and fillAfter set to false.
     */
    STDMETHOD_(IAnimation *, clone)(THIS) SCONST OVERRIDE;

    /**
     * @brief Copies the properties of another animation to this animation.
     * @param src Pointer to the source animation to copy from.
     */
    STDMETHOD_(void, copy)(THIS_ const IAnimation *src) OVERRIDE;

    /**
     * @brief Resets the animation to its initial state.
     */
    STDMETHOD_(void, reset)(THIS) OVERRIDE;

    /**
     * @brief Cancels the animation.
     */
    STDMETHOD_(void, cancel)(THIS) OVERRIDE;

    /**
     * @brief Sets the interpolator used by the animation to smooth the movement.
     * @param i Pointer to the interpolator.
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator *i) OVERRIDE;

    /**
     * @brief Sets the duration of one animation cycle in milliseconds.
     * @param durationMillis Duration in milliseconds.
     */
    STDMETHOD_(void, setDuration)(THIS_ long durationMillis) OVERRIDE;

    /**
     * @brief Scales the current duration of the animation.
     * @param scale Scale factor.
     */
    STDMETHOD_(void, scaleCurrentDuration)(THIS_ float scale) OVERRIDE;

    /**
     * @brief Sets whether the animation transformation should be applied before the animation starts.
     * @param bFill Boolean indicating whether to fill before.
     */
    STDMETHOD_(void, setFillBefore)(THIS_ BOOL bFill) OVERRIDE;

    /**
     * @brief Sets whether the animation transformation should be applied after the animation ends.
     * @param bFill Boolean indicating whether to fill after.
     */
    STDMETHOD_(void, setFillAfter)(THIS_ BOOL bFill) OVERRIDE;

    /**
     * @brief Gets whether the animation transformation should be applied before the animation starts.
     * @return TRUE if fill before is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, getFillBefore)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets whether the animation transformation should be applied after the animation ends.
     * @return TRUE if fill after is enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, getFillAfter)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the delay in milliseconds after which the animation must start.
     * @param offset Delay in milliseconds.
     */
    STDMETHOD_(void, setStartOffset)(THIS_ long offset) OVERRIDE;

    /**
     * @brief Checks whether fillBefore should be taken into account.
     * @return TRUE if fill enabled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, isFillEnabled)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets whether fillBefore should be taken into account.
     * @param fillEnabled Boolean indicating whether fill is enabled.
     */
    STDMETHOD_(void, setFillEnabled)(THIS_ BOOL fillEnabled) OVERRIDE;

    /**
     * @brief Sets the start time of the animation.
     * @param startTimeMillis Start time in milliseconds.
     */
    STDMETHOD_(void, setStartTime)(THIS_ int64_t startTimeMillis) OVERRIDE;

    /**
     * @brief Starts the animation.
     */
    STDMETHOD_(void, start)(THIS) OVERRIDE;

    /**
     * @brief Starts the animation immediately.
     */
    STDMETHOD_(void, startNow)(THIS) OVERRIDE;

    /**
     * @brief Sets the repeat mode of the animation.
     * @param repeatMode Repeat mode (RESTART or REVERSE).
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode repeatMode) OVERRIDE;

    /**
     * @brief Sets the number of times the animation must repeat.
     * @param repeatCount Number of repeats.
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int repeatCount) OVERRIDE;

    /**
     * @brief Sets the desired Z order mode during animation.
     * @param zAdjustment Z order adjustment mode.
     */
    STDMETHOD_(void, setZAdjustment)(THIS_ ZAdjustment zAdjustment) OVERRIDE;

    /**
     * @brief Gets the interpolator used by the animation.
     * @return Pointer to the interpolator.
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the start time of the animation.
     * @return Start time in milliseconds.
     */
    STDMETHOD_(int64_t, getStartTime)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the duration of one animation cycle.
     * @return Duration in milliseconds.
     */
    STDMETHOD_(long, getDuration)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the delay in milliseconds after which the animation must start.
     * @return Delay in milliseconds.
     */
    STDMETHOD_(long, getStartOffset)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the repeat mode of the animation.
     * @return Repeat mode (RESTART or REVERSE).
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the number of times the animation must repeat.
     * @return Number of repeats.
     */
    STDMETHOD_(int, getRepeatCount)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the desired Z order mode during animation.
     * @return Z order adjustment mode.
     */
    STDMETHOD_(ZAdjustment, getZAdjustment)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the animation listener to be notified when the animation starts, ends, or repeats.
     * @param listener Pointer to the animation listener.
     */
    STDMETHOD_(void, setAnimationListener)(THIS_ IAnimationListener *listener) OVERRIDE;

    /**
     * @brief Computes the duration hint for the animation.
     * @return Duration hint in milliseconds.
     */
    STDMETHOD_(long, computeDurationHint)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the transformation at a specific time.
     * @param currentTime Current time in milliseconds.
     * @param outTransformation Pointer to the transformation to apply.
     * @param scale Scale factor.
     * @return TRUE if the transformation is valid, FALSE otherwise.
     */
    STDMETHOD_(BOOL, getTransformation2)
    (THIS_ uint64_t currentTime, ITransformation *outTransformation, float scale) OVERRIDE;

    /**
     * @brief Gets the transformation at a specific time.
     * @param currentTime Current time in milliseconds.
     * @param outTransformation Pointer to the transformation to apply.
     * @return TRUE if the transformation is valid, FALSE otherwise.
     */
    STDMETHOD_(BOOL, getTransformation)
    (THIS_ uint64_t currentTime, ITransformation *outTransformation) OVERRIDE;

    /**
     * @brief Checks whether the animation has started.
     * @return TRUE if the animation has started, FALSE otherwise.
     */
    STDMETHOD_(BOOL, hasStarted)(THIS) SCONST OVERRIDE;

    /**
     * @brief Checks whether the animation has ended.
     * @return TRUE if the animation has ended, FALSE otherwise.
     */
    STDMETHOD_(BOOL, hasEnded)(THIS) SCONST OVERRIDE;

    /**
     * @brief Applies the transformation at a specific interpolated time.
     * @param interpolatedTime Interpolated time between 0 and 1.
     * @param t Pointer to the transformation to apply.
     */
    STDMETHOD_(void, applyTransformation)
    (THIS_ float interpolatedTime, ITransformation *t) OVERRIDE;

    /**
     * @brief Checks whether the animation affects the alpha property.
     * @return TRUE if the animation affects the alpha property, FALSE otherwise.
     */
    STDMETHOD_(BOOL, hasAlpha)(THIS) SCONST OVERRIDE;

    /**
     * @brief Initializes the animation with the dimensions of the object and its parent.
     * @param width Width of the object being animated.
     * @param height Height of the object being animated.
     * @param parentWidth Width of the parent of the object being animated.
     * @param parentHeight Height of the parent of the object being animated.
     * @param nScale Scale factor.
     */
    STDMETHOD_(void, initialize)
    (THIS_ int width, int height, int parentWidth, int parentHeight, int nScale) OVERRIDE;

    /**
     * @brief Sets user data associated with the animation.
     * @param data User data.
     */
    STDMETHOD_(void, setUserData)(THIS_ ULONG_PTR data) OVERRIDE;

    /**
     * @brief Gets user data associated with the animation.
     * @return User data.
     */
    STDMETHOD_(ULONG_PTR, getUserData)(THIS) SCONST OVERRIDE;

    /**
     * @brief Pauses the animation.
     */
    STDMETHOD_(void, pause)(THIS) OVERRIDE;

    /**
     * @brief Resumes the animation.
     */
    STDMETHOD_(void, resume)(THIS) OVERRIDE;

  protected:
    /**
     * @brief Checks whether the animation is canceled.
     * @return TRUE if the animation is canceled, FALSE otherwise.
     */
    bool isCanceled();

    /**
     * @brief Notifies the animation listener that the animation has started.
     */
    void fireAnimationStart();

    /**
     * @brief Notifies the animation listener that the animation has repeated.
     */
    void fireAnimationRepeat();

    /**
     * @brief Notifies the animation listener that the animation has ended.
     */
    void fireAnimationEnd();

    /**
     * @brief Converts the information in the description of a size to an actual dimension.
     * @param value The dimension associated with the type parameter.
     * @param size The size of the object being animated.
     * @param parentSize The size of the parent of the object being animated.
     * @param nScale Scale factor.
     * @return The dimension to use for the animation.
     */
    int resolveSize(const SValueDescription &value, int size, int parentSize, int nScale);

    /**
     * @brief Ensures that this animation has an interpolator.
     * Will use an AccelerateDecelerateInterpolator if nothing else was specified.
     */
    void ensureInterpolator();

    /**
     * @brief Gets the scale factor that should be applied to pre-scaled values in an Animation.
     * Such as the pivot points in {@link ScaleAnimation} and {@link RotateAnimation}.
     * @return The scale factor.
     */
    float getScaleFactor();

  public:
    /**
     * @brief Attributes for SAnimation
     */
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"duration", mDuration, FALSE)       ///< Duration of the animation in milliseconds.
        ATTR_INT(L"startOffset", mStartOffset, FALSE) ///< Delay before the animation starts in milliseconds.
        ATTR_BOOL(L"fillEnable", mFillEnabled, FALSE) ///< Whether fillBefore and fillAfter are taken into account.
        ATTR_BOOL(L"fillBefore", mFillBefore, FALSE)  ///< Whether the animation transformation should be applied before the animation starts.
        ATTR_BOOL(L"fillAfter", mFillAfter, FALSE)    ///< Whether the animation transformation should be applied after the animation ends.
        ATTR_INT(L"repeatCount", mRepeatCount, FALSE) ///< Number of times the animation must repeat.
        ATTR_ENUM_BEGIN(L"zAdjustment", ZAdjustment, FALSE)
            ATTR_ENUM_VALUE(L"normal", ZORDER_NORMAL) ///< Normal Z order adjustment.
            ATTR_ENUM_VALUE(L"top", ZORDER_TOP)       ///< Top Z order adjustment.
            ATTR_ENUM_VALUE(L"bottom", ZORDER_BOTTOM) ///< Bottom Z order adjustment.
        ATTR_ENUM_END(mZAdjustment)
        ATTR_ENUM_BEGIN(L"repeatMode", RepeatMode, FALSE)
            ATTR_ENUM_VALUE(L"restart", RESTART) ///< Restart repeat mode.
            ATTR_ENUM_VALUE(L"reverse", REVERSE) ///< Reverse repeat mode.
        ATTR_ENUM_END(mRepeatMode)
        ATTR_INTERPOLATOR(L"interpolator", mInterpolator, FALSE) ///< Interpolator used by the animation.
        ATTR_CHAIN_PTR(mInterpolator, 0)
    SOUI_ATTRS_END()
};

SNSEND

#endif // __SANIMATION__H__
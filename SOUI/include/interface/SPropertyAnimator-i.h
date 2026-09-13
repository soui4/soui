/**
 * @file SPropertyAnimator-i.h
 * @brief SOUI property animation interface definition - modeled after Android's property animation system design
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
 * @brief Property value holder interface - similar to Android's PropertyValuesHolder
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
     * @brief Set property name
     * @param propertyName property name
     */
    STDMETHOD_(void, SetPropertyName)(THIS_ LPCWSTR propertyName) PURE;

    /**
     * @brief Get property name
     * @return property name
     */
    STDMETHOD_(LPCWSTR, GetPropertyName)(CTHIS) SCONST PURE;

    /**
     * @brief Set byte value
     * @param values value array
     * @param count value count
     */
    STDMETHOD_(void, SetByteValues)(THIS_ const BYTE *values, int count) PURE;

    /**
     * @brief Set short value
     * @param values value array
     * @param count value count
     */
    STDMETHOD_(void, SetShortValues)(THIS_ const short *values, int count) PURE;

    /**
     * @brief Color value
     * @param values value array
     * @param count value count
     */
    STDMETHOD_(void, SetColorRefValues)(THIS_ const COLORREF *values, int count) PURE;

    /**
     * @brief Set float value
     * @param values value array
     * @param count value count
     */
    STDMETHOD_(void, SetFloatValues)(THIS_ const float *values, int count) PURE;

    /**
     * @brief Set integer value
     * @param values value array
     * @param count value count
     */
    STDMETHOD_(void, SetIntValues)(THIS_ const int *values, int count) PURE;

    /**
     * @brief Set LAYOUTSIZE value
     * @param values value array
     * @param count value count
     */
    STDMETHOD_(void, SetLayoutSizeValues)(THIS_ const LAYOUTSIZE *values, int count) PURE;

    /**
     * @brief Set position value (for position-related properties)
     * @param values value array (specific type depends on layout type)
     * @param count value count
     * @param valueSize size of each value (in bytes)
     */
    STDMETHOD_(void, SetPositionValues)(THIS_ const void *values, int count, int valueSize) PURE;

    /**
     * @brief Get current animation value
     * @param fraction animation progress (0.0-1.0)
     * @param pValue output value pointer
     * @return returns TRUE on success
     */
    STDMETHOD_(BOOL, GetAnimatedValue)(CTHIS_ float fraction, void *pValue) SCONST PURE;

    /**
     * @brief Get value type
     * @return value type
     */
    STDMETHOD_(PROP_TYPE, GetValueType)(CTHIS) SCONST PURE;

    /**
     * @brief Get memory size occupied by value
     * @return value count
     */
    STDMETHOD_(int, GetValueSize)(CTHIS) SCONST PURE;

    /**
     * @brief Get start value
     * @param pValue output value pointer
     */
    STDMETHOD_(void, GetStartValue)(CTHIS_ void *pValue) SCONST PURE;

    /**
     * @brief Get end value
     * @param pValue output value pointer
     */
    STDMETHOD_(void, GetEndValue)(CTHIS_ void *pValue) SCONST PURE;

    /**
     * @brief Convert animation progress to index
     * @param fraction animation progress (0.0-1.0)
     * @param idx index value array (output)
     * @return animation progress within the segment (0.0-1.0)
     */
    STDMETHOD_(float, Fraction2Index)(CTHIS_ float fraction, int idx[2]) SCONST PURE;

    /**
     * @brief Convert animation progress to frame index
     * @param fraction animation progress (0.0-1.0)
     * @return frame index
     */
    STDMETHOD_(int, Fraction2FrameIndex)(CTHIS_ float fraction) SCONST PURE;

    /**
     * @brief Get value by index
     * @param index index value
     * @param pValue output value pointer
     * @param valueSize value size
     * @return returns TRUE on success
     */
    STDMETHOD_(BOOL, GetValueByIndex)(CTHIS_ int index, void *pValue, int valueSize) SCONST PURE;

    /**
     * @brief Get keyframe count
     * @return keyframe count
     */
    STDMETHOD_(int, GetKeyframeCount)(CTHIS) SCONST PURE;

    /**
     * @brief Set keyframe weights
     * @param weights weight array
     * @param count weight count
     * @return returns TRUE on success, FALSE on failure (weight count does not match keyframe count)
     * @note Weight count must match keyframe count; if not set, default is 1.0
     */
    STDMETHOD_(BOOL, SetKeyFrameWeights)(THIS_ const float *weights, int count) PURE;

    /**
     * @brief Get keyframe weights
     * @param weights weight array
     * @param count weight count
     * @return returns TRUE on success, FALSE on failure (weight count does not match keyframe count)
     */
    STDMETHOD_(BOOL, GetKeyFrameWeights)(CTHIS_ float *weights, int count) SCONST PURE;
};

DECLARE_INTERFACE_(IPropertyAnimator, IValueAnimator)
{
#include <interface/SobjectApi.h>

    /**
     * @brief Set the animation duration
     * @param duration - animation duration (in milliseconds)
     * @return void
     */
    STDMETHOD_(void, setDuration)(THIS_ long duration) PURE;

    /**
     * @brief Get the animation duration
     * @return long - animation duration (in milliseconds)
     */
    STDMETHOD_(long, getDuration)(CTHIS) SCONST PURE;

    /**
     * @brief Get the total animation duration
     * @return long - total animation duration (in milliseconds)
     */
    STDMETHOD_(long, getTotalDuration)(CTHIS) SCONST PURE;

    /**
     * @brief Set the current playback time of the animation
     * @param playTime - playback time (in milliseconds)
     * @return void
     */
    STDMETHOD_(void, setCurrentPlayTime)(THIS_ long playTime) PURE;

    /**
     * @brief Set the current playback progress of the animation
     * @param fraction - playback progress (between 0 and 1)
     * @return void
     */
    STDMETHOD_(void, setCurrentFraction)(THIS_ float fraction) PURE;

    /**
     * @brief Get the current playback time of the animation
     * @return long - current playback time (in milliseconds)
     */
    STDMETHOD_(long, getCurrentPlayTime)(THIS) PURE;

    /**
     * @brief Get the animation start delay
     * @return long - start delay (in milliseconds)
     */
    STDMETHOD_(long, getStartDelay)(CTHIS) SCONST PURE;

    /**
     * @brief Set the animation start delay
     * @param startDelay - start delay (in milliseconds)
     * @return void
     */
    STDMETHOD_(void, setStartDelay)(THIS_ long startDelay) PURE;

    /**
     * @brief Set the animation repeat count
     * @param value - repeat count
     * @return void
     */
    STDMETHOD_(void, setRepeatCount)(THIS_ int value) PURE;

    /**
     * @brief Get the animation repeat count
     * @return int - repeat count
     */
    STDMETHOD_(int, getRepeatCount)(CTHIS) SCONST PURE;

    /**
     * @brief Set the animation repeat mode
     * @param value - repeat mode (RESTART or REVERSE)
     * @return void
     */
    STDMETHOD_(void, setRepeatMode)(THIS_ RepeatMode value) PURE;

    /**
     * @brief Get the animation repeat mode
     * @return RepeatMode - repeat mode (RESTART or REVERSE)
     */
    STDMETHOD_(RepeatMode, getRepeatMode)(CTHIS) SCONST PURE;

    /**
     * @brief Add an animation update listener
     * @param listener - listener pointer
     * @return void
     */
    STDMETHOD_(void, addUpdateListener)(THIS_ IAnimatorUpdateListener * listener) PURE;

    /**
     * @brief Remove all animation update listeners
     * @return void
     */
    STDMETHOD_(void, removeAllUpdateListeners)(THIS) PURE;

    /**
     * @brief Remove an animation update listener
     * @param listener - listener pointer
     * @return void
     */
    STDMETHOD_(void, removeUpdateListener)(THIS_ IAnimatorUpdateListener * listener) PURE;

    /**
     * @brief Set the time interpolator
     * @param value - interpolator pointer
     * @return void
     */
    STDMETHOD_(void, setInterpolator)(THIS_ IInterpolator * value) PURE;

    /**
     * @brief Get the time interpolator
     * @return IInterpolator* - interpolator pointer
     */
    STDMETHOD_(IInterpolator *, getInterpolator)(CTHIS) SCONST PURE;

    /**
     * @brief Add an animation listener
     * @param p - listener pointer
     * @return void
     */
    STDMETHOD_(void, addListener)(THIS_ IAnimatorListener * p) PURE;

    /**
     * @brief Remove an animation listener
     * @param p - listener pointer
     * @return void
     */
    STDMETHOD_(void, removeListener)(THIS_ IAnimatorListener * p) PURE;

    /**
     * @brief Start the animation
     * @param pContainer - timeline manager pointer
     * @return void
     */
    STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr * pContainer) PURE;

    /**
     * @brief End the animation
     * @return void
     */
    STDMETHOD_(void, end)(THIS) PURE;

    /**
     * @brief Check whether the animation is running
     * @return BOOL - returns TRUE if running, otherwise FALSE
     */
    STDMETHOD_(BOOL, isRunning)(CTHIS) SCONST PURE;

    /**
     * @brief Check whether the animation has started
     * @return BOOL - returns TRUE if started, otherwise FALSE
     */
    STDMETHOD_(BOOL, isStarted)(CTHIS) SCONST PURE;

    /**
     * @brief Play the animation in reverse
     * @return void
     */
    STDMETHOD_(void, reverse)(THIS) PURE;

    /**
     * @brief Submit an animation frame
     * @param frameTime - frame time
     * @return TRUE if the animation is finished, FALSE otherwise.
     */
    STDMETHOD_(BOOL, commitAnimationFrame)(THIS_ uint64_t frameTime) PURE;

    /**
     * @brief Get the current animation progress
     * @return float - current progress
     */
    STDMETHOD_(float, getAnimatedFraction)(CTHIS) SCONST PURE;

    /**
     * @brief Clone the animation object
     * @return IValueAnimator* - pointer to the cloned animation object
     */
    STDMETHOD_(IValueAnimator *, clone)(CTHIS) SCONST PURE;

    /**
     * @brief Copy the animation object
     * @param src - source animation object pointer
     * @return void
     */
    STDMETHOD_(void, copy)(THIS_ const IValueAnimator *src) PURE;

    /**
     * @brief Evaluate the animation value
     * @param fraction - animation progress
     * @return void
     */
    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) PURE;

    /**
     * @brief Get the timeline handler
     * @return ITimelineHandler* - timeline handler pointer
     */
    STDMETHOD_(ITimelineHandler *, GetTimelineHandler)(CTHIS) SCONST PURE;

    /**
     * @brief Get user data
     * @return LPVOID - user data pointer
     */
    STDMETHOD_(LPVOID, GetUserData)(CTHIS) SCONST PURE;

    /**
     * @brief Set user data
     * @param pUserData - user data pointer
     * @return void
     */
    STDMETHOD_(void, SetUserData)(THIS_ LPVOID pUserData) PURE;
    //------------------------------------------------------------------------------

    /**
     * @brief Get animation target
     * @return animation target
     */
    STDMETHOD_(IWindow *, GetTarget)(CTHIS) SCONST PURE;

    /**
     * @brief Set animation target
     * @param target animation target
     */
    STDMETHOD_(void, SetTarget)(THIS_ IWindow * target) PURE;

    /**
     * @brief Set property value holder
     * @param pHolder property value holder
     */
    STDMETHOD_(void, SetPropertyValuesHolder)(THIS_ IPropertyValuesHolder * pHolder) PURE;
    /**
     * @brief Set property value holder array
     * @param pHolders property value holder array
     * @param count property value holder count
     */
    STDMETHOD_(void, SetPropertyValuesHolders)(THIS_ IPropertyValuesHolder * *pHolders, int count) PURE;
    /**
     * @brief Get property value holder
     * @param propertyName property name
     * @return property value holder
     */
    STDMETHOD_(IPropertyValuesHolder *, GetPropertyValuesHolderByName)(CTHIS_ LPCWSTR propertyName) SCONST PURE;

    /**
     * @brief Get property value holder
     * @param index index
     * @return property value holder
     */
    STDMETHOD_(IPropertyValuesHolder *, GetPropertyValuesHolderByIndex)(CTHIS_ int index) SCONST PURE;

    /**
     * @brief Get property value holder count
     * @return property value holder count
     */
    STDMETHOD_(int, GetPropertyValuesHolderCount)(CTHIS) SCONST PURE;
};

SNSEND

#endif /**< __SPROPERTY_AMINATOR_I__H__ */

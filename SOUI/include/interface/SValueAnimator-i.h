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
     * @brief Called when the animation updates
     * @param pAnimator - Animation pointer
     * @return void
     */
    STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator * pAnimator) PURE;
};

#undef INTERFACE
#define INTERFACE IAnimatorListener
DECLARE_INTERFACE(IAnimatorListener)
{
    /**
     * @brief Called when the animation starts
     * @param pAnimator - Animation pointer
     * @return void
     */
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * @brief Called when the animation ends (not applicable to infinitely repeating animations)
     * @param pAnimator - Animation pointer
     * @return void
     */
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * @brief Called when the animation repeats
     * @param pAnimator - Animation pointer
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
};

typedef struct IAnimatorGroup IAnimatorGroup;

#undef INTERFACE
#define INTERFACE IAnimatorGroupListerer
DECLARE_INTERFACE(IAnimatorGroupListerer)
{
    /**
     * @brief Called when the animation group ends
     * @param pGroup - Animation group pointer
     * @return void
     */
    STDMETHOD_(void, OnAnimatorGroupEnd)(THIS_ IAnimatorGroup * pGroup, int nID) PURE;
};

#undef INTERFACE
#define INTERFACE IAnimatorGroup
DECLARE_INTERFACE_(IAnimatorGroup, IObjRef)
{
    /**
     * @brief Increment reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Add an animation
     * @param ani - Animation pointer
     * @return BOOL - Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, AddAnimator)(THIS_ IValueAnimator * ani) PURE;

    /**
     * @brief Remove an animation
     * @param ani - Animation pointer
     * @return BOOL - Returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, RemoveAnimator)(THIS_ IValueAnimator * ani) PURE;

    /**
     * @brief Set the animation group listener
     * @param listener - Listener pointer
     * @return void
     */
    STDMETHOD_(void, SetListener)(THIS_ IAnimatorGroupListerer * listener) PURE;
};

#ifdef __cplusplus
class SAnimatorListener : public IAnimatorListener {
    STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator)
    {
    }
    STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator)
    {
    }
    STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator)
    {
    }
};
#endif /**< __cplusplus */

SNSEND
#endif /**< __SVALUEANIMATOR_I__H__ */

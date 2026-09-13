#ifndef __SANIMATORSET_I__H__
#define __SANIMATORSET_I__H__
#include <interface/SValueAnimator-i.h>

SNSBEGIN

typedef struct IAnimatorSet IAnimatorSet;

/**
 * @brief AnimatorSet execution mode
 * @details SEQUENCE: Play animators one after another
 *          PARALLEL: Play all animators at the same time
 */
enum AnimatorSetPlayMode
{
    SEQUENCE = 0, /**<  Sequential execution */
    PARALLEL = 1  /**<  Parallel execution */
};

#undef INTERFACE
#define INTERFACE IAnimatorSet
DECLARE_INTERFACE_(IAnimatorSet, IValueAnimator)
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

    //-------------------------------------------------------------------------

    /**
     * @brief Add a value animator to the set
     * @param pAnimator - The animator to be added
     * @return void
     */
    STDMETHOD_(void, AddAnimator)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * @brief Add a value animator with an after-animator dependency
     * @details This animator will be executed after the specified after-animator ends
     * @param pAnimator - The animator to be added
     * @param pAfterAnimator - The animator that must finish before pAnimator starts
     * @return void
     */
    STDMETHOD_(void, AddAnimatorAfter)(THIS_ IValueAnimator * pAnimator, IValueAnimator * pAfterAnimator) PURE;

    /**
     * @brief Add a value animator with a with-animator dependency
     * @details This animator will be executed in parallel with the specified with-animator
     * @param pAnimator - The animator to be added
     * @param pWithAnimator - The animator that will run in parallel with pAnimator
     * @return void
     */
    STDMETHOD_(void, AddAnimatorWith)(THIS_ IValueAnimator * pAnimator, IValueAnimator * pWithAnimator) PURE;

    /**
     * @brief Remove an animator from the set
     * @param pAnimator - The animator to be removed
     * @return void
     */
    STDMETHOD_(void, RemoveAnimator)(THIS_ IValueAnimator * pAnimator) PURE;

    /**
     * @brief Remove all animators from the set
     * @return void
     */
    STDMETHOD_(void, RemoveAllAnimators)(THIS) PURE;

    /**
     * @brief Get the number of animators in the set
     * @return int - The count of animators
     */
    STDMETHOD_(int, GetAnimatorCount)(CTHIS) SCONST PURE;

    /**
     * @brief Get animator at the specified index
     * @param index - The index of the animator
     * @return IValueAnimator* - The animator at the specified index
     */
    STDMETHOD_(IValueAnimator *, GetAnimatorAt)(CTHIS_ int index) SCONST PURE;

    /**
     * @brief Set play mode for all animators in the set (sequential or parallel)
     * @param mode - The play mode (SEQUENCE or PARALLEL)
     * @return void
     */
    STDMETHOD_(void, SetPlayMode)(THIS_ AnimatorSetPlayMode mode) PURE;

    /**
     * @brief Get the current play mode
     * @return AnimatorSetPlayMode - The current play mode
     */
    STDMETHOD_(AnimatorSetPlayMode, GetPlayMode)(CTHIS) SCONST PURE;
};

SNSEND

#endif /**< __SANIMATORSET_I__H__ */

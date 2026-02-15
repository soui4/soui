#ifndef __SANIMATORSET__H__
#define __SANIMATORSET__H__

/**
 * @file SAnimatorSet.h
 * @brief Provides a mechanism to play a set of SValueAnimator objects in sequence or in parallel.
 *
 * @details This class provides a mechanism to play a set of SValueAnimator objects in sequence or in parallel.
 * It works by establishing dependencies between animations and then playing them in the correct order.
 * This is modeled after Android's AnimatorSet implementation.
 */

#include <interface/SAnimatorSet-i.h>
#include <proxy/SValueAnimatorProxy.h>
SNSBEGIN

/**
 * @class SAnimatorSet
 * @brief A set of SValueAnimator objects to be played together.
 *
 * @details This class provides a mechanism to play a set of SValueAnimator objects in sequence or in parallel.
 * It establishes dependencies between animations and plays them in the correct order.
 * SAnimatorSet itself is an IAnimatorSet (which inherits from IValueAnimator), which means it can be used
 * just like any other animator, and can even be added to other AnimatorSets for nesting, following the
 * Android AnimatorSet design.
 */
class SOUI_EXP SAnimatorSet : public TValueAnimatorProxy<IAnimatorSet> {
    DEF_SOBJECT(SValueAnimator, L"animatorSet")
  protected:
    /**
     * @brief Internal node to track animator dependencies
     */
    struct AnimatorNode
    {
        SAutoRefPtr<IValueAnimator> animator;
        // List of animators that should run after this one
        SArray<AnimatorNode *> afterNodes;
        // List of animators that should run in parallel with this one
        SArray<AnimatorNode *> withNodes;
        // List of animators that should run before this one
        SArray<AnimatorNode *> beforeNodes;
        // Start time of this animator relative to set start
        long startTime;
        // Whether this node has been started
        bool started;

        AnimatorNode(IValueAnimator *pAnimator)
            : animator(pAnimator)
            , startTime(0)
            , started(false)
        {
        }

        ~AnimatorNode()
        {
            afterNodes.RemoveAll();
            withNodes.RemoveAll();
            beforeNodes.RemoveAll();
        }
    };

  private:
    /**
     * @brief Map of animators to their nodes
     */
    SMap<IValueAnimator *, AnimatorNode *> mAnimatorToNodeMap;

    /**
     * @brief List of all animator nodes
     */
    SArray<AnimatorNode *> mAnimatorNodes;

    /**
     * @brief Play mode: SEQUENCE or PARALLEL
     */
    AnimatorSetPlayMode mPlayMode;

    /**
     * @brief Current play time (relative to set start, after delay)
     */
    long mCurrentPlayTime;

    /**
     * @brief The start time when the set was started (absolute time)
     */
    uint64_t mSetStartTime;

    /**
     * @brief List of listeners (inherited from SValueAnimator via IAnimatorListener)
     */

    /**
     * @brief Currently running animators
     */
    SArray<AnimatorNode *> mRunningAnimators;

    /**
     * @brief The animated fraction (0.0 to 1.0)
     */
    float mAnimatedFraction;

  public:
    /**
     * @brief Creates a new SAnimatorSet object
     */
    SAnimatorSet();

    /**
     * @brief Destructor
     */
    ~SAnimatorSet();

    /**
     * @brief Add an animator to the set (will play in parallel by default or according to play mode)
     * @param pAnimator The animator to add
     */
    STDMETHOD_(void, AddAnimator)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

    /**
     * @brief Add an animator that will play after the specified animator finishes
     * @param pAnimator The animator to add
     * @param pAfterAnimator The animator that must finish first
     */
    STDMETHOD_(void, AddAnimatorAfter)(THIS_ IValueAnimator *pAnimator, IValueAnimator *pAfterAnimator) OVERRIDE;

    /**
     * @brief Add an animator that will play in parallel with the specified animator
     * @param pAnimator The animator to add
     * @param pWithAnimator The animator to run in parallel with
     */
    STDMETHOD_(void, AddAnimatorWith)(THIS_ IValueAnimator *pAnimator, IValueAnimator *pWithAnimator) OVERRIDE;

    /**
     * @brief Remove an animator from the set
     * @param pAnimator The animator to remove
     */
    STDMETHOD_(void, RemoveAnimator)(THIS_ IValueAnimator *pAnimator) OVERRIDE;

    /**
     * @brief Remove all animators from the set
     */
    STDMETHOD_(void, RemoveAllAnimators)(THIS) OVERRIDE;

    /**
     * @brief Get the number of animators
     * @return The count of animators
     */
    STDMETHOD_(int, GetAnimatorCount)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Get animator at specified index
     * @param index The index
     * @return The animator at the specified index
     */
    STDMETHOD_(IValueAnimator *, GetAnimatorAt)(CTHIS_ int index) SCONST OVERRIDE;

    /**
     * @brief Set the duration of the set
     * @param duration The duration in milliseconds
     */
    STDMETHOD_(void, setDuration)(THIS_ long duration) OVERRIDE;

    /**
     * @brief Get the duration
     * @return The duration in milliseconds
     */
    STDMETHOD_(long, getDuration)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Get the total duration including all animations
     * @return The total duration in milliseconds
     */
    STDMETHOD_(long, getTotalDuration)(THIS) SCONST OVERRIDE;

    /**
     * @brief Set the start delay
     * @param startDelay The start delay in milliseconds
     */
    STDMETHOD_(void, setStartDelay)(THIS_ long startDelay) OVERRIDE;

    /**
     * @brief Get the start delay
     * @return The start delay in milliseconds
     */
    STDMETHOD_(long, getStartDelay)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Start the animator set
     * @param pContainer The timeline handlers container
     */
    STDMETHOD_(void, start)(THIS_ ITimelineHandlersMgr *pContainer) OVERRIDE;

    /**
     * @brief End the animator set
     */
    STDMETHOD_(void, end)(THIS) OVERRIDE;

    /**
     * @brief Check if the set is running
     * @return TRUE if running, FALSE otherwise
     */
    STDMETHOD_(BOOL, isRunning)(THIS) SCONST OVERRIDE;

    /**
     * @brief Check if the set has been started
     * @return TRUE if started, FALSE otherwise
     */
    STDMETHOD_(BOOL, isStarted)(THIS) SCONST OVERRIDE;

    /**
     * @brief Play the set in reverse
     */
    STDMETHOD_(void, reverse)(THIS) OVERRIDE;

    /**
     * @brief Set the current play time
     * @param playTime The play time in milliseconds
     */
    STDMETHOD_(void, setCurrentPlayTime)(THIS_ long playTime) OVERRIDE;

    /**
     * @brief Set the current animation fraction
     * @param fraction The fraction between 0 and 1
     */
    STDMETHOD_(void, setCurrentFraction)(THIS_ float fraction) OVERRIDE;

    /**
     * @brief Get the current play time
     * @return The current play time in milliseconds
     */
    STDMETHOD_(long, getCurrentPlayTime)(THIS) OVERRIDE;

    /**
     * @brief Get the animated fraction
     * @return The fraction between 0 and 1
     */
    STDMETHOD_(float, getAnimatedFraction)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Commit animation frame (main animation update method)
     * @param frameTime The current frame time
     * @return TRUE if animation is finished, FALSE otherwise
     */
    STDMETHOD_(BOOL, commitAnimationFrame)(THIS_ uint64_t frameTime) OVERRIDE;

    /**
     * @brief Evaluate the animation value
     * @param fraction The animation fraction
     */
    STDMETHOD_(void, onEvaluateValue)(THIS_ float fraction) OVERRIDE;

    /**
     * @brief Clone the animator
     * @return The cloned animator
     */
    STDMETHOD_(IValueAnimator *, clone)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Copy from another animator
     * @param src The source animator
     */
    STDMETHOD_(void, copy)(THIS_ const IValueAnimator *src) OVERRIDE;

    /**
     * @brief Set the play mode (SEQUENCE or PARALLEL)
     * @param mode The play mode
     */
    STDMETHOD_(void, SetPlayMode)(THIS_ AnimatorSetPlayMode mode) OVERRIDE;

    /**
     * @brief Get the current play mode
     * @return The play mode
     */
    STDMETHOD_(AnimatorSetPlayMode, GetPlayMode)(CTHIS) SCONST OVERRIDE;

  private:
    /**
     * @brief Finds or creates a node for the given animator
     * @param pAnimator The animator
     * @return The animator node
     */
    AnimatorNode *findOrCreateNode(IValueAnimator *pAnimator);

    /**
     * @brief Finds a node for the given animator
     * @param pAnimator The animator
     * @return The animator node or NULL if not found
     */
    AnimatorNode *findNode(IValueAnimator *pAnimator) const;

    /**
     * @brief Calculates start times for all animators based on dependencies
     */
    void calculateStartTimes();

    /**
     * @brief Notifies start listeners
     */
    void notifyStartListeners();

    /**
     * @brief Notifies end listeners
     */
    void notifyEndListeners();

    /**
     * @brief Notifies repeat listeners
     */
    void notifyRepeatListeners();

    /**
     * @brief Starts appropriate animators based on current time
     * @param currentTime The current time
     */
    void startAnimatorsAtTime(uint64_t currentTime);

    /**
     * @brief Updates running animators
     * @param frameTime The current frame time
     */
    void updateRunningAnimators(uint64_t frameTime);

    /**
     * @brief Checks if all animators have finished
     * @return TRUE if all finished, FALSE otherwise
     */
    bool allAnimatorsFinished() const;

    /**
     * @brief Cleanup resources
     */
    void cleanup();
};

SNSEND

#endif // __SANIMATORSET__H__

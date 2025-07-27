#ifndef __SVALUEANIMATOR_CAPI_H__
#define __SVALUEANIMATOR_CAPI_H__

#include "../SValueAnimator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SValueAnimator Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IAnimatorUpdateListener C API Macros */
#define IAnimatorUpdateListener_onAnimationUpdate(This, pAnimator) \
    ((This)->lpVtbl->onAnimationUpdate(This, pAnimator))

/* IAnimatorListener C API Macros */
#define IAnimatorListener_onAnimationStart(This, pAnimator) \
    ((This)->lpVtbl->onAnimationStart(This, pAnimator))

#define IAnimatorListener_onAnimationEnd(This, pAnimator) \
    ((This)->lpVtbl->onAnimationEnd(This, pAnimator))

#define IAnimatorListener_onAnimationRepeat(This, pAnimator) \
    ((This)->lpVtbl->onAnimationRepeat(This, pAnimator))

/* IValueAnimator C API Macros */
#define IValueAnimator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IValueAnimator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IValueAnimator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IValueAnimator_setDuration(This, duration) \
    ((This)->lpVtbl->setDuration(This, duration))

#define IValueAnimator_getDuration(This) \
    ((This)->lpVtbl->getDuration(This))

#define IValueAnimator_getTotalDuration(This) \
    ((This)->lpVtbl->getTotalDuration(This))

#define IValueAnimator_setCurrentPlayTime(This, playTime) \
    ((This)->lpVtbl->setCurrentPlayTime(This, playTime))

#define IValueAnimator_setCurrentFraction(This, fraction) \
    ((This)->lpVtbl->setCurrentFraction(This, fraction))

#define IValueAnimator_getCurrentPlayTime(This) \
    ((This)->lpVtbl->getCurrentPlayTime(This))

#define IValueAnimator_getStartDelay(This) \
    ((This)->lpVtbl->getStartDelay(This))

#define IValueAnimator_setStartDelay(This, startDelay) \
    ((This)->lpVtbl->setStartDelay(This, startDelay))

#define IValueAnimator_setRepeatCount(This, value) \
    ((This)->lpVtbl->setRepeatCount(This, value))

#define IValueAnimator_getRepeatCount(This) \
    ((This)->lpVtbl->getRepeatCount(This))

#define IValueAnimator_setRepeatMode(This, value) \
    ((This)->lpVtbl->setRepeatMode(This, value))

#define IValueAnimator_getRepeatMode(This) \
    ((This)->lpVtbl->getRepeatMode(This))

#define IValueAnimator_addUpdateListener(This, listener) \
    ((This)->lpVtbl->addUpdateListener(This, listener))

#define IValueAnimator_removeAllUpdateListeners(This) \
    ((This)->lpVtbl->removeAllUpdateListeners(This))

#define IValueAnimator_removeUpdateListener(This, listener) \
    ((This)->lpVtbl->removeUpdateListener(This, listener))

#define IValueAnimator_setInterpolator(This, value) \
    ((This)->lpVtbl->setInterpolator(This, value))

#define IValueAnimator_getInterpolator(This) \
    ((This)->lpVtbl->getInterpolator(This))

#define IValueAnimator_addListener(This, p) \
    ((This)->lpVtbl->addListener(This, p))

#define IValueAnimator_removeListener(This, p) \
    ((This)->lpVtbl->removeListener(This, p))

#define IValueAnimator_start(This, pContainer) \
    ((This)->lpVtbl->start(This, pContainer))

#define IValueAnimator_end(This) \
    ((This)->lpVtbl->end(This))

#define IValueAnimator_isRunning(This) \
    ((This)->lpVtbl->isRunning(This))

#define IValueAnimator_isStarted(This) \
    ((This)->lpVtbl->isStarted(This))

#define IValueAnimator_reverse(This) \
    ((This)->lpVtbl->reverse(This))

#define IValueAnimator_commitAnimationFrame(This, frameTime) \
    ((This)->lpVtbl->commitAnimationFrame(This, frameTime))

#define IValueAnimator_getAnimatedFraction(This) \
    ((This)->lpVtbl->getAnimatedFraction(This))

#define IValueAnimator_clone(This) \
    ((This)->lpVtbl->clone(This))

#define IValueAnimator_copy(This, src) \
    ((This)->lpVtbl->copy(This, src))

#define IValueAnimator_onEvaluateValue(This, fraction) \
    ((This)->lpVtbl->onEvaluateValue(This, fraction))

/* IAnimatorGroupListerer C API Macros */
#define IAnimatorGroupListerer_OnAnimatorGroupEnd(This, pGroup) \
    ((This)->lpVtbl->OnAnimatorGroupEnd(This, pGroup))

/* IAnimatorGroup C API Macros */
#define IAnimatorGroup_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAnimatorGroup_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAnimatorGroup_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IAnimatorGroup_AddAnimator(This, ani) \
    ((This)->lpVtbl->AddAnimator(This, ani))

#define IAnimatorGroup_RemoveAnimator(This, ani) \
    ((This)->lpVtbl->RemoveAnimator(This, ani))

#define IAnimatorGroup_SetListener(This, listener) \
    ((This)->lpVtbl->SetListener(This, listener))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IValueAnimator Helper Functions */
static inline long IValueAnimator_AddRef_C(IValueAnimator* pThis)
{
    return IValueAnimator_AddRef(pThis);
}

static inline long IValueAnimator_Release_C(IValueAnimator* pThis)
{
    return IValueAnimator_Release(pThis);
}

static inline void IValueAnimator_setDuration_C(IValueAnimator* pThis, long duration)
{
    IValueAnimator_setDuration(pThis, duration);
}

static inline long IValueAnimator_getDuration_C(IValueAnimator* pThis)
{
    return IValueAnimator_getDuration(pThis);
}

static inline void IValueAnimator_start_C(IValueAnimator* pThis, ITimelineHandlersMgr* pContainer)
{
    IValueAnimator_start(pThis, pContainer);
}

static inline void IValueAnimator_end_C(IValueAnimator* pThis)
{
    IValueAnimator_end(pThis);
}

static inline BOOL IValueAnimator_isRunning_C(IValueAnimator* pThis)
{
    return IValueAnimator_isRunning(pThis);
}

static inline float IValueAnimator_getAnimatedFraction_C(IValueAnimator* pThis)
{
    return IValueAnimator_getAnimatedFraction(pThis);
}

static inline IValueAnimator* IValueAnimator_clone_C(IValueAnimator* pThis)
{
    return IValueAnimator_clone(pThis);
}

/* IAnimatorGroup Helper Functions */
static inline long IAnimatorGroup_AddRef_C(IAnimatorGroup* pThis)
{
    return IAnimatorGroup_AddRef(pThis);
}

static inline long IAnimatorGroup_Release_C(IAnimatorGroup* pThis)
{
    return IAnimatorGroup_Release(pThis);
}

static inline BOOL IAnimatorGroup_AddAnimator_C(IAnimatorGroup* pThis, IValueAnimator* ani)
{
    return IAnimatorGroup_AddAnimator(pThis, ani);
}

static inline BOOL IAnimatorGroup_RemoveAnimator_C(IAnimatorGroup* pThis, IValueAnimator* ani)
{
    return IAnimatorGroup_RemoveAnimator(pThis, ani);
}

/*
 * Convenience macros for common animator operations
 */

/* Duration shortcuts */
#define IValueAnimator_SetDurationSeconds(This, seconds) \
    IValueAnimator_setDuration(This, (seconds) * 1000)

#define IValueAnimator_SetDurationMillis(This, millis) \
    IValueAnimator_setDuration(This, millis)

#define IValueAnimator_GetDurationSeconds(This) \
    (IValueAnimator_getDuration(This) / 1000.0f)

/* Playback control shortcuts */
#define IValueAnimator_Play(This, container) \
    IValueAnimator_start(This, container)

#define IValueAnimator_Stop(This) \
    IValueAnimator_end(This)

#define IValueAnimator_Pause(This) \
    IValueAnimator_end(This)

#define IValueAnimator_Resume(This, container) \
    IValueAnimator_start(This, container)

/* Progress control shortcuts */
#define IValueAnimator_SetProgress(This, progress) \
    IValueAnimator_setCurrentFraction(This, progress)

#define IValueAnimator_GetProgress(This) \
    IValueAnimator_getAnimatedFraction(This)

#define IValueAnimator_SetProgressPercent(This, percent) \
    IValueAnimator_setCurrentFraction(This, (percent) / 100.0f)

#define IValueAnimator_GetProgressPercent(This) \
    (IValueAnimator_getAnimatedFraction(This) * 100.0f)

/* Repeat control shortcuts */
#define IValueAnimator_SetInfiniteRepeat(This) \
    IValueAnimator_setRepeatCount(This, -1)

#define IValueAnimator_SetNoRepeat(This) \
    IValueAnimator_setRepeatCount(This, 0)

#define IValueAnimator_SetRepeatOnce(This) \
    IValueAnimator_setRepeatCount(This, 1)

/* State checking shortcuts */
#define IValueAnimator_IsActive(This) \
    (IValueAnimator_isRunning(This) || IValueAnimator_isStarted(This))

#define IValueAnimator_IsStopped(This) \
    (!IValueAnimator_isRunning(This) && !IValueAnimator_isStarted(This))

#define IValueAnimator_IsCompleted(This) \
    (IValueAnimator_getAnimatedFraction(This) >= 1.0f)

#define IValueAnimator_IsAtStart(This) \
    (IValueAnimator_getAnimatedFraction(This) <= 0.0f)

/* Safe animator operations */
#define IValueAnimator_SafeStart(This, container) \
    do { \
        if ((This) && (container) && !IValueAnimator_isRunning(This)) { \
            IValueAnimator_start(This, container); \
        } \
    } while(0)

#define IValueAnimator_SafeEnd(This) \
    do { \
        if ((This) && IValueAnimator_isRunning(This)) { \
            IValueAnimator_end(This); \
        } \
    } while(0)

#define IValueAnimator_SafeSetDuration(This, duration) \
    do { \
        if ((This) && (duration) > 0) { \
            IValueAnimator_setDuration(This, duration); \
        } \
    } while(0)

/* Listener management shortcuts */
#define IValueAnimator_ClearUpdateListeners(This) \
    IValueAnimator_removeAllUpdateListeners(This)

#define IValueAnimator_SafeAddUpdateListener(This, listener) \
    do { \
        if ((This) && (listener)) { \
            IValueAnimator_addUpdateListener(This, listener); \
        } \
    } while(0)

#define IValueAnimator_SafeRemoveUpdateListener(This, listener) \
    do { \
        if ((This) && (listener)) { \
            IValueAnimator_removeUpdateListener(This, listener); \
        } \
    } while(0)

#define IValueAnimator_SafeAddListener(This, listener) \
    do { \
        if ((This) && (listener)) { \
            IValueAnimator_addListener(This, listener); \
        } \
    } while(0)

#define IValueAnimator_SafeRemoveListener(This, listener) \
    do { \
        if ((This) && (listener)) { \
            IValueAnimator_removeListener(This, listener); \
        } \
    } while(0)

/* Animator group shortcuts */
#define IAnimatorGroup_SafeAddAnimator(This, animator) \
    ((This) && (animator) ? IAnimatorGroup_AddAnimator(This, animator) : FALSE)

#define IAnimatorGroup_SafeRemoveAnimator(This, animator) \
    ((This) && (animator) ? IAnimatorGroup_RemoveAnimator(This, animator) : FALSE)

#define IAnimatorGroup_SafeSetListener(This, listener) \
    do { \
        if ((This) && (listener)) { \
            IAnimatorGroup_SetListener(This, listener); \
        } \
    } while(0)

/*
 * Reference counting helpers
 */
#define IValueAnimator_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IValueAnimator_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#define IAnimatorGroup_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IAnimatorGroup_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#ifdef __cplusplus
}
#endif

#endif /* __SVALUEANIMATOR_CAPI_H__ */

#ifndef __SANIMATION_CAPI_H__
#define __SANIMATION_CAPI_H__

#include "../SAnimation-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SAnimation Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IAnimationListener C API Macros */
#define IAnimationListener_OnAnimationStart(This, animation) \
    ((This)->lpVtbl->OnAnimationStart(This, animation))

#define IAnimationListener_OnAnimationStop(This, animation) \
    ((This)->lpVtbl->OnAnimationStop(This, animation))

#define IAnimationListener_OnAnimationRepeat(This, animation) \
    ((This)->lpVtbl->OnAnimationRepeat(This, animation))

#define IAnimationListener_OnAnimationPauseChange(This, animation, bPaused) \
    ((This)->lpVtbl->OnAnimationPauseChange(This, animation, bPaused))

/* IAnimation C API Macros */
#define IAnimation_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAnimation_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAnimation_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IAnimation_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IAnimation_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IAnimation_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IAnimation_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IAnimation_SetID(This, uID) \
    ((This)->lpVtbl->SetID(This, uID))

#define IAnimation_InitFromXml(This, pNode) \
    ((This)->lpVtbl->InitFromXml(This, pNode))

#define IAnimation_OnInitFinished(This, pNode) \
    ((This)->lpVtbl->OnInitFinished(This, pNode))

#define IAnimation_clone(This) \
    ((This)->lpVtbl->clone(This))

#define IAnimation_copy(This, src) \
    ((This)->lpVtbl->copy(This, src))

#define IAnimation_reset(This) \
    ((This)->lpVtbl->reset(This))

#define IAnimation_cancel(This) \
    ((This)->lpVtbl->cancel(This))

#define IAnimation_setInterpolator(This, i) \
    ((This)->lpVtbl->setInterpolator(This, i))

#define IAnimation_setDuration(This, durationMillis) \
    ((This)->lpVtbl->setDuration(This, durationMillis))

#define IAnimation_scaleCurrentDuration(This, scale) \
    ((This)->lpVtbl->scaleCurrentDuration(This, scale))

#define IAnimation_setFillBefore(This, bFill) \
    ((This)->lpVtbl->setFillBefore(This, bFill))

#define IAnimation_setFillAfter(This, bFill) \
    ((This)->lpVtbl->setFillAfter(This, bFill))

#define IAnimation_getFillBefore(This) \
    ((This)->lpVtbl->getFillBefore(This))

#define IAnimation_getFillAfter(This) \
    ((This)->lpVtbl->getFillAfter(This))

#define IAnimation_setStartOffset(This, offset) \
    ((This)->lpVtbl->setStartOffset(This, offset))

#define IAnimation_isFillEnabled(This) \
    ((This)->lpVtbl->isFillEnabled(This))

#define IAnimation_setFillEnabled(This, fillEnabled) \
    ((This)->lpVtbl->setFillEnabled(This, fillEnabled))

#define IAnimation_setStartTime(This, startTimeMillis) \
    ((This)->lpVtbl->setStartTime(This, startTimeMillis))

#define IAnimation_start(This) \
    ((This)->lpVtbl->start(This))

#define IAnimation_startNow(This) \
    ((This)->lpVtbl->startNow(This))

#define IAnimation_setRepeatMode(This, repeatMode) \
    ((This)->lpVtbl->setRepeatMode(This, repeatMode))

#define IAnimation_setRepeatCount(This, repeatCount) \
    ((This)->lpVtbl->setRepeatCount(This, repeatCount))

#define IAnimation_setZAdjustment(This, zAdjustment) \
    ((This)->lpVtbl->setZAdjustment(This, zAdjustment))

#define IAnimation_setBackgroundColor(This, bg) \
    ((This)->lpVtbl->setBackgroundColor(This, bg))

#define IAnimation_setDetachWallpaper(This, detachWallpaper) \
    ((This)->lpVtbl->setDetachWallpaper(This, detachWallpaper))

#define IAnimation_getDetachWallpaper(This) \
    ((This)->lpVtbl->getDetachWallpaper(This))

#define IAnimation_willChangeTransformationMatrix(This) \
    ((This)->lpVtbl->willChangeTransformationMatrix(This))

#define IAnimation_willChangeBounds(This) \
    ((This)->lpVtbl->willChangeBounds(This))

#define IAnimation_setAnimationListener(This, listener) \
    ((This)->lpVtbl->setAnimationListener(This, listener))

#define IAnimation_ensureInterpolator(This) \
    ((This)->lpVtbl->ensureInterpolator(This))

#define IAnimation_computeDurationHint(This) \
    ((This)->lpVtbl->computeDurationHint(This))

#define IAnimation_getTransformation(This, currentTime, outTransformation, scale) \
    ((This)->lpVtbl->getTransformation(This, currentTime, outTransformation, scale))

#define IAnimation_getTransformation2(This, currentTime, outTransformation) \
    ((This)->lpVtbl->getTransformation2(This, currentTime, outTransformation))

#define IAnimation_hasStarted(This) \
    ((This)->lpVtbl->hasStarted(This))

#define IAnimation_hasEnded(This) \
    ((This)->lpVtbl->hasEnded(This))

#define IAnimation_getStartTime(This) \
    ((This)->lpVtbl->getStartTime(This))

#define IAnimation_getDuration(This) \
    ((This)->lpVtbl->getDuration(This))

#define IAnimation_getStartOffset(This) \
    ((This)->lpVtbl->getStartOffset(This))

#define IAnimation_getRepeatMode(This) \
    ((This)->lpVtbl->getRepeatMode(This))

#define IAnimation_getRepeatCount(This) \
    ((This)->lpVtbl->getRepeatCount(This))

#define IAnimation_getZAdjustment(This) \
    ((This)->lpVtbl->getZAdjustment(This))

#define IAnimation_getBackgroundColor(This) \
    ((This)->lpVtbl->getBackgroundColor(This))

#define IAnimation_getScaleFactor(This) \
    ((This)->lpVtbl->getScaleFactor(This))

#define IAnimation_getInvalidateRegion(This, left, top, right, bottom, invalidate, transformation) \
    ((This)->lpVtbl->getInvalidateRegion(This, left, top, right, bottom, invalidate, transformation))

#define IAnimation_initializeInvalidateRegion(This, left, top, right, bottom) \
    ((This)->lpVtbl->initializeInvalidateRegion(This, left, top, right, bottom))

/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

/* IAnimationListener Helper Functions */
static inline void IAnimationListener_OnAnimationStart_C(IAnimationListener* pThis, IAnimation* animation)
{
    IAnimationListener_OnAnimationStart(pThis, animation);
}

static inline void IAnimationListener_OnAnimationStop_C(IAnimationListener* pThis, IAnimation* animation)
{
    IAnimationListener_OnAnimationStop(pThis, animation);
}

static inline void IAnimationListener_OnAnimationRepeat_C(IAnimationListener* pThis, IAnimation* animation)
{
    IAnimationListener_OnAnimationRepeat(pThis, animation);
}

static inline void IAnimationListener_OnAnimationPauseChange_C(IAnimationListener* pThis, IAnimation* animation, BOOL bPaused)
{
    IAnimationListener_OnAnimationPauseChange(pThis, animation, bPaused);
}

/* IAnimation Helper Functions */
static inline long IAnimation_AddRef_C(IAnimation* pThis)
{
    return IAnimation_AddRef(pThis);
}

static inline long IAnimation_Release_C(IAnimation* pThis)
{
    return IAnimation_Release(pThis);
}

static inline IAnimation* IAnimation_clone_C(IAnimation* pThis)
{
    return IAnimation_clone(pThis);
}

static inline void IAnimation_copy_C(IAnimation* pThis, const IAnimation* src)
{
    IAnimation_copy(pThis, src);
}

static inline void IAnimation_reset_C(IAnimation* pThis)
{
    IAnimation_reset(pThis);
}

static inline void IAnimation_cancel_C(IAnimation* pThis)
{
    IAnimation_cancel(pThis);
}

static inline void IAnimation_start_C(IAnimation* pThis)
{
    IAnimation_start(pThis);
}

static inline void IAnimation_startNow_C(IAnimation* pThis)
{
    IAnimation_startNow(pThis);
}

static inline void IAnimation_setDuration_C(IAnimation* pThis, long durationMillis)
{
    IAnimation_setDuration(pThis, durationMillis);
}

static inline long IAnimation_getDuration_C(IAnimation* pThis)
{
    return IAnimation_getDuration(pThis);
}

static inline void IAnimation_setRepeatCount_C(IAnimation* pThis, int repeatCount)
{
    IAnimation_setRepeatCount(pThis, repeatCount);
}

static inline int IAnimation_getRepeatCount_C(IAnimation* pThis)
{
    return IAnimation_getRepeatCount(pThis);
}

static inline void IAnimation_setRepeatMode_C(IAnimation* pThis, RepeatMode repeatMode)
{
    IAnimation_setRepeatMode(pThis, repeatMode);
}

static inline RepeatMode IAnimation_getRepeatMode_C(IAnimation* pThis)
{
    return IAnimation_getRepeatMode(pThis);
}

static inline BOOL IAnimation_hasStarted_C(IAnimation* pThis)
{
    return IAnimation_hasStarted(pThis);
}

static inline BOOL IAnimation_hasEnded_C(IAnimation* pThis)
{
    return IAnimation_hasEnded(pThis);
}

static inline void IAnimation_setAnimationListener_C(IAnimation* pThis, IAnimationListener* listener)
{
    IAnimation_setAnimationListener(pThis, listener);
}

/*
 * Convenience macros for common animation operations
 */
#define IAnimation_StartAnimation(This) \
    IAnimation_start(This)

#define IAnimation_StopAnimation(This) \
    IAnimation_cancel(This)

#define IAnimation_SetDurationMs(This, ms) \
    IAnimation_setDuration(This, ms)

#define IAnimation_SetDurationSec(This, sec) \
    IAnimation_setDuration(This, (sec) * 1000)

#define IAnimation_SetRepeatForever(This) \
    IAnimation_setRepeatCount(This, -1)

#define IAnimation_SetRepeatOnce(This) \
    IAnimation_setRepeatCount(This, 0)

#define IAnimation_SetRepeatTimes(This, times) \
    IAnimation_setRepeatCount(This, times)

#define IAnimation_IsRunning(This) \
    (IAnimation_hasStarted(This) && !IAnimation_hasEnded(This))

#ifdef __cplusplus
}
#endif

#endif /* __SANIMATION_CAPI_H__ */

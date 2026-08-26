#ifndef __SANIMATION_CAPI_H__
#define __SANIMATION_CAPI_H__

#include "../SAnimation-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SAnimation Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 *
 * Interfaces covered:
 *   IAnimationListener (no base interface)
 *   IAnimation         (inherits IObject)
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

/* IAnimation C API Macros (IObject base interface) */
#define IAnimation_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAnimation_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAnimation_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IAnimation_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IAnimation_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define IAnimation_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IAnimation_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IAnimation_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IAnimation_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define IAnimation_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define IAnimation_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define IAnimation_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define IAnimation_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define IAnimation_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define IAnimation_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define IAnimation_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define IAnimation_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define IAnimation_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define IAnimation_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define IAnimation_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

/* IAnimation specific interface macros */
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

#define IAnimation_getInterpolator(This) \
    ((This)->lpVtbl->getInterpolator(This))

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

#define IAnimation_setAnimationListener(This, listener) \
    ((This)->lpVtbl->setAnimationListener(This, listener))

#define IAnimation_computeDurationHint(This) \
    ((This)->lpVtbl->computeDurationHint(This))

#define IAnimation_getTransformation2(This, currentTime, outTransformation, scale) \
    ((This)->lpVtbl->getTransformation2(This, currentTime, outTransformation, scale))

#define IAnimation_getTransformation(This, currentTime, outTransformation) \
    ((This)->lpVtbl->getTransformation(This, currentTime, outTransformation))

#define IAnimation_hasStarted(This) \
    ((This)->lpVtbl->hasStarted(This))

#define IAnimation_hasEnded(This) \
    ((This)->lpVtbl->hasEnded(This))

#define IAnimation_applyTransformation(This, interpolatedTime, t) \
    ((This)->lpVtbl->applyTransformation(This, interpolatedTime, t))

#define IAnimation_hasAlpha(This) \
    ((This)->lpVtbl->hasAlpha(This))

#define IAnimation_initialize(This, width, height, parentWidth, parentHeight, nScale) \
    ((This)->lpVtbl->initialize(This, width, height, parentWidth, parentHeight, nScale))

#define IAnimation_setUserData(This, data) \
    ((This)->lpVtbl->setUserData(This, data))

#define IAnimation_getUserData(This) \
    ((This)->lpVtbl->getUserData(This))

#define IAnimation_pause(This) \
    ((This)->lpVtbl->pause(This))

#define IAnimation_resume(This) \
    ((This)->lpVtbl->resume(This))

#ifdef __cplusplus
}
#endif

#endif /* __SANIMATION_CAPI_H__ */

#ifndef __SVALUEANIMATOR_CAPI_H__
#define __SVALUEANIMATOR_CAPI_H__

#include "../SValueAnimator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SValueAnimator Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 *
 * Interfaces covered:
 *   IAnimatorUpdateListener  (no base interface)
 *   IAnimatorListener        (no base interface)
 *   IValueAnimator           (inherits IObject)
 *   IAnimatorGroupListerer   (no base interface)
 *   IAnimatorGroup           (inherits IObjRef)
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

/* IValueAnimator C API Macros (IObject base interface) */
#define IValueAnimator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IValueAnimator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IValueAnimator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IValueAnimator_IsClass(This, lpszName) \
    ((This)->lpVtbl->IsClass(This, lpszName))

#define IValueAnimator_GetClassNameList(This, ppClassNameList, nMaxCount) \
    ((This)->lpVtbl->GetClassNameList(This, ppClassNameList, nMaxCount))

#define IValueAnimator_GetObjectClass(This) \
    ((This)->lpVtbl->GetObjectClass(This))

#define IValueAnimator_GetObjectType(This) \
    ((This)->lpVtbl->GetObjectType(This))

#define IValueAnimator_GetID(This) \
    ((This)->lpVtbl->GetID(This))

#define IValueAnimator_SetID(This, nID) \
    ((This)->lpVtbl->SetID(This, nID))

#define IValueAnimator_GetName(This) \
    ((This)->lpVtbl->GetName(This))

#define IValueAnimator_GetNameA(This) \
    ((This)->lpVtbl->GetNameA(This))

#define IValueAnimator_SetName(This, pszName) \
    ((This)->lpVtbl->SetName(This, pszName))

#define IValueAnimator_InitFromXml(This, xmlNode) \
    ((This)->lpVtbl->InitFromXml(This, xmlNode))

#define IValueAnimator_OnInitFinished(This, xmlNode) \
    ((This)->lpVtbl->OnInitFinished(This, xmlNode))

#define IValueAnimator_ISetAttribute(This, strAttribName, strValue, bLoading) \
    ((This)->lpVtbl->ISetAttribute(This, strAttribName, strValue, bLoading))

#define IValueAnimator_SetAttributeA(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttributeA(This, pszAttr, pszValue, bLoading))

#define IValueAnimator_SetAttribute(This, pszAttr, pszValue, bLoading) \
    ((This)->lpVtbl->SetAttribute(This, pszAttr, pszValue, bLoading))

#define IValueAnimator_GetAttribute(This, strAttr, pValue) \
    ((This)->lpVtbl->GetAttribute(This, strAttr, pValue))

#define IValueAnimator_AfterAttribute(This, pszAttr, pszValue, bLoading, hr) \
    ((This)->lpVtbl->AfterAttribute(This, pszAttr, pszValue, bLoading, hr))

#define IValueAnimator_SetAttrHandler(This, attrHandler) \
    ((This)->lpVtbl->SetAttrHandler(This, attrHandler))

/* IValueAnimator specific interface macros */
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

#define IValueAnimator_GetTimelineHandler(This) \
    ((This)->lpVtbl->GetTimelineHandler(This))

#define IValueAnimator_GetUserData(This) \
    ((This)->lpVtbl->GetUserData(This))

#define IValueAnimator_SetUserData(This, pUserData) \
    ((This)->lpVtbl->SetUserData(This, pUserData))

/* IAnimatorGroupListerer C API Macros */
#define IAnimatorGroupListerer_OnAnimatorGroupEnd(This, pGroup, nID) \
    ((This)->lpVtbl->OnAnimatorGroupEnd(This, pGroup, nID))

/* IAnimatorGroup C API Macros (IObjRef base interface) */
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

#ifdef __cplusplus
}
#endif

#endif /* __SVALUEANIMATOR_CAPI_H__ */

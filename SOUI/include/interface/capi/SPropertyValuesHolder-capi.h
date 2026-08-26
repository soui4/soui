#ifndef __SPROPERTY_VALUES_HOLDER_CAPI_H__
#define __SPROPERTY_VALUES_HOLDER_CAPI_H__

#include "../SPropertyAnimator-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IPropertyValuesHolder C API Macros (IObjRef base interface) */
#define IPropertyValuesHolder_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IPropertyValuesHolder_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IPropertyValuesHolder_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IPropertyValuesHolder specific interface macros */
#define IPropertyValuesHolder_SetPropertyName(This, propertyName) \
    ((This)->lpVtbl->SetPropertyName(This, propertyName))

#define IPropertyValuesHolder_GetPropertyName(This) \
    ((This)->lpVtbl->GetPropertyName(This))

#define IPropertyValuesHolder_SetByteValues(This, values, count) \
    ((This)->lpVtbl->SetByteValues(This, values, count))

#define IPropertyValuesHolder_SetShortValues(This, values, count) \
    ((This)->lpVtbl->SetShortValues(This, values, count))

#define IPropertyValuesHolder_SetColorRefValues(This, values, count) \
    ((This)->lpVtbl->SetColorRefValues(This, values, count))

#define IPropertyValuesHolder_SetFloatValues(This, values, count) \
    ((This)->lpVtbl->SetFloatValues(This, values, count))

#define IPropertyValuesHolder_SetIntValues(This, values, count) \
    ((This)->lpVtbl->SetIntValues(This, values, count))

#define IPropertyValuesHolder_SetLayoutSizeValues(This, values, count) \
    ((This)->lpVtbl->SetLayoutSizeValues(This, values, count))

#define IPropertyValuesHolder_SetPositionValues(This, values, count, valueSize) \
    ((This)->lpVtbl->SetPositionValues(This, values, count, valueSize))

#define IPropertyValuesHolder_GetAnimatedValue(This, fraction, pValue) \
    ((This)->lpVtbl->GetAnimatedValue(This, fraction, pValue))

#define IPropertyValuesHolder_GetValueType(This) \
    ((This)->lpVtbl->GetValueType(This))

#define IPropertyValuesHolder_GetValueSize(This) \
    ((This)->lpVtbl->GetValueSize(This))

#define IPropertyValuesHolder_GetStartValue(This, pValue) \
    ((This)->lpVtbl->GetStartValue(This, pValue))

#define IPropertyValuesHolder_GetEndValue(This, pValue) \
    ((This)->lpVtbl->GetEndValue(This, pValue))

#define IPropertyValuesHolder_Fraction2Index(This, fraction, idx) \
    ((This)->lpVtbl->Fraction2Index(This, fraction, idx))

#define IPropertyValuesHolder_Fraction2FrameIndex(This, fraction) \
    ((This)->lpVtbl->Fraction2FrameIndex(This, fraction))

#define IPropertyValuesHolder_GetValueByIndex(This, index, pValue, valueSize) \
    ((This)->lpVtbl->GetValueByIndex(This, index, pValue, valueSize))

#define IPropertyValuesHolder_GetKeyframeCount(This) \
    ((This)->lpVtbl->GetKeyframeCount(This))

#define IPropertyValuesHolder_SetKeyFrameWeights(This, weights, count) \
    ((This)->lpVtbl->SetKeyFrameWeights(This, weights, count))

#define IPropertyValuesHolder_GetKeyFrameWeights(This, weights, count) \
    ((This)->lpVtbl->GetKeyFrameWeights(This, weights, count))

/* IPropertyAnimator C API Macros (IValueAnimator base interface) */
#define IPropertyAnimator_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IPropertyAnimator_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IPropertyAnimator_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IPropertyAnimator specific interface macros */
#define IPropertyAnimator_GetTarget(This) \
    ((This)->lpVtbl->GetTarget(This))

#define IPropertyAnimator_SetTarget(This, target) \
    ((This)->lpVtbl->SetTarget(This, target))

#define IPropertyAnimator_SetPropertyValuesHolder(This, pHolder) \
    ((This)->lpVtbl->SetPropertyValuesHolder(This, pHolder))

#define IPropertyAnimator_SetPropertyValuesHolders(This, pHolders, count) \
    ((This)->lpVtbl->SetPropertyValuesHolders(This, pHolders, count))

#define IPropertyAnimator_GetPropertyValuesHolderByName(This, propertyName) \
    ((This)->lpVtbl->GetPropertyValuesHolderByName(This, propertyName))

#define IPropertyAnimator_GetPropertyValuesHolderByIndex(This, index) \
    ((This)->lpVtbl->GetPropertyValuesHolderByIndex(This, index))

#define IPropertyAnimator_GetPropertyValuesHolderCount(This) \
    ((This)->lpVtbl->GetPropertyValuesHolderCount(This))

#ifdef __cplusplus
}
#endif

#endif /* __SPROPERTY_VALUES_HOLDER_CAPI_H__ */

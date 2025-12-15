#ifndef __SPROPERTY_VALUES_HOLDER_CAPI_H__
#define __SPROPERTY_VALUES_HOLDER_CAPI_H__

#include "../sproperty-values-holder-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IObjRef base interface macros */
#define IPropertyValuesHolder_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IPropertyValuesHolder_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IPropertyValuesHolder_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IPropertyValuesHolder specific interface macros */
#define IPropertyValuesHolder_SetPropertyName(This, propertyName) \
    ((This)->lpVtbl->SetPropertyName(This, propertyName))

#define IPropertyValuesHolder_SetPropertyValues(This, nValueCount, pValues) \
    ((This)->lpVtbl->SetPropertyValues(This, nValueCount, pValues))

#define IPropertyValuesHolder_GetPropertyName(This) \
    ((This)->lpVtbl->GetPropertyName(This))

#define IPropertyValuesHolder_GetValueType(This) \
    ((This)->lpVtbl->GetValueType(This))

#define IPropertyValuesHolder_SetConverter(This, pConverter) \
    ((This)->lpVtbl->SetConverter(This, pConverter))

#define IPropertyValuesHolder_GetPropertyValues(This, nValueCount) \
    ((This)->lpVtbl->GetPropertyValues(This, nValueCount))

#define IPropertyValuesHolder_Clone(This) \
    ((This)->lpVtbl->Clone(This))

#ifdef __cplusplus
}
#endif

#endif /* __SPROPERTY_VALUES_HOLDER_CAPI_H__ */
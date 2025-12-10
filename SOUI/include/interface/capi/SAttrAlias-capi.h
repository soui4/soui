#ifndef __SATTRALIAS_CAPI_H__
#define __SATTRALIAS_CAPI_H__

#include "../SAttrAlias-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IObjRef base interface macros */
#define IAttrAlias_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAttrAlias_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAttrAlias_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IAttrAlias specific interface macros */
#define IAttrAlias_GetAliasPtr(This, strName) \
    ((This)->lpVtbl->GetAliasPtr(This, strName))

#define IAttrAlias_GetAliasValue(This, strName) \
    ((This)->lpVtbl->GetAliasValue(This, strName))

#ifdef __cplusplus
}
#endif

#endif /* __SATTRALIAS_CAPI_H__ */
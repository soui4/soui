#ifndef __SACCPROXY_CAPI_H__
#define __SACCPROXY_CAPI_H__

#include "../saccproxy-i.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

/* IObjRef base interface macros */
#define IAccProxy_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IAccProxy_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IAccProxy_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

/* IAccProxy specific interface macros */
#define IAccProxy_get_accName(This, pszName) \
    ((This)->lpVtbl->get_accName(This, pszName))

#define IAccProxy_get_accValue(This, pszValue) \
    ((This)->lpVtbl->get_accValue(This, pszValue))

#define IAccProxy_get_accDescription(This, pszDescription) \
    ((This)->lpVtbl->get_accDescription(This, pszDescription))

#define IAccProxy_get_accRole(This, pvarRole) \
    ((This)->lpVtbl->get_accRole(This, pvarRole))

#define IAccProxy_get_accState(This, pvarState) \
    ((This)->lpVtbl->get_accState(This, pvarState))

#define IAccProxy_get_accHelp(This, pszHelp) \
    ((This)->lpVtbl->get_accHelp(This, pszHelp))

#define IAccProxy_get_accHelpTopic(This, pszHelpFile, pidTopic) \
    ((This)->lpVtbl->get_accHelpTopic(This, pszHelpFile, pidTopic))

#define IAccProxy_get_accKeyboardShortcut(This, pszKeyboardShortcut) \
    ((This)->lpVtbl->get_accKeyboardShortcut(This, pszKeyboardShortcut))

#define IAccProxy_get_accDefaultAction(This, pszDefaultAction) \
    ((This)->lpVtbl->get_accDefaultAction(This, pszDefaultAction))

#define IAccProxy_accSelect(This, flagsSelect) \
    ((This)->lpVtbl->accSelect(This, flagsSelect))

#define IAccProxy_accDoDefaultAction(This) \
    ((This)->lpVtbl->accDoDefaultAction(This))

#define IAccProxy_put_accName(This, szName) \
    ((This)->lpVtbl->put_accName(This, szName))

#define IAccProxy_put_accValue(This, szValue) \
    ((This)->lpVtbl->put_accValue(This, szValue))

#endif //_WIN32

#ifdef __cplusplus
}
#endif

#endif /* __SACCPROXY_CAPI_H__ */

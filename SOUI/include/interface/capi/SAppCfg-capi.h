#ifndef __SAPPCFG_CAPI_H__
#define __SAPPCFG_CAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <interface/SAppCfg-i.h>

#define IAppCfg_SetRender(This, render) \
    (IAppCfg*)This->lpVtbl->ISetRender(This, render)

#define IAppCfg_SetImgDecoder(This, decoder) \
    (IAppCfg*)This->lpVtbl->ISetImgDecoder(This, decoder)

#define IAppCfg_SetSysResPeHandle(This, hResModule) \
    (IAppCfg*)This->lpVtbl->ISetSysResPeHandle(This, hResModule)

#define IAppCfg_SetSysResPeFile(This, pszPath) \
    (IAppCfg*)This->lpVtbl->ISetSysResPeFile(This, pszPath)

#define IAppCfg_SetSysResFile(This, pszPath) \
    (IAppCfg*)This->lpVtbl->ISetSysResFile(This, pszPath)

#define IAppCfg_SetSysResZip(This, pszZipFile, pszPwd) \
    (IAppCfg*)This->lpVtbl->ISetSysResZip(This, pszZipFile, pszPwd)

#define IAppCfg_SetSysRes7z(This, psz7zFile, pszPwd) \
    (IAppCfg*)This->lpVtbl->ISetSysRes7z(This, psz7zFile, pszPwd)

#define IAppCfg_SetAppResPeHandle(This, hResModule) \
    (IAppCfg*)This->lpVtbl->ISetAppResPeHandle(This, hResModule)

#define IAppCfg_SetAppResPeFile(This, pszPath) \
    (IAppCfg*)This->lpVtbl->ISetAppResPeFile(This, pszPath)

#define IAppCfg_SetAppResFile(This, pszPath) \
    (IAppCfg*)This->lpVtbl->ISetAppResFile(This, pszPath)

#define IAppCfg_SetAppResZip(This, pszZipFile, pszPwd) \
    (IAppCfg*)This->lpVtbl->ISetAppResZip(This, pszZipFile, pszPwd)

#define IAppCfg_SetAppRes7z(This, psz7zFile, pszPwd) \
    (IAppCfg*)This->lpVtbl->ISetAppRes7z(This, psz7zFile, pszPwd)

#define IAppCfg_SetUidefId(This, strUidefId) \
    (IAppCfg*)This->lpVtbl->ISetUidefId(This, strUidefId)

#define IAppCfg_SetLog(This, bLogEnable, nLogLevel, pszLogName) \
    (IAppCfg*)This->lpVtbl->ISetLog(This, bLogEnable, nLogLevel, pszLogName)

#define IAppCfg_SetAppDir(This, pszAppDir) \
    (IAppCfg*)This->lpVtbl->ISetAppDir(This, pszAppDir)

#define IAppCfg_EnableScript(This, bEnable) \
    (IAppCfg*)This->lpVtbl->IEnableScript(This, bEnable)

#define IAppCfg_EnableMultiLang(This, langResId, bEnable) \
    (IAppCfg*)This->lpVtbl->IEnableMultiLang(This, langResId, bEnable)

#define IAppCfg_DoConfig(This, pApp) \
    This->lpVtbl->IDoConfig(This, pApp)

/*
 * Safe reference management macros
 */
#define IAppCfg_SafeAddRef(This) \
    IObjRef_SafeAddRef((IUnknown*)(This))

#define IAppCfg_SafeRelease(This) \
    IObjRef_SafeRelease((IUnknown**)(This))

#ifdef __cplusplus
}
#endif

#endif /* __SAPPCFG_CAPI_H__ */
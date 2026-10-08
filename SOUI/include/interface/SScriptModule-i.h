/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SScriptModule-i.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe
 */

#ifndef __SSCRIPTMODULE_I__H__
#define __SSCRIPTMODULE_I__H__
#include <utilities-def.h>
#include <interface/obj-ref-i.h>
#include <interface/SEvtArgs-i.h>
#include <interface/SMsgLoop-i.h>
#include <oleauto.h>
#include <stdint.h>

SNSBEGIN

/**
 * @brief NativeCall handler function pointer type
 *
 * When the script side calls NativeCall(name, ...), the bridge layer boxes
 * every argument after name into a VARIANT and passes them to the handler
 * as an array plus a count. The boxing rules are:
 *   - lua nil      -> VT_EMPTY
 *   - lua boolean  -> VT_BOOL  (boolVal, VARIANT_TRUE / VARIANT_FALSE)
 *   - lua integer  -> VT_I8    (llVal)
 *   - lua number   -> VT_R8    (dblVal)
 *   - lua string   -> VT_LPSTR (pcVal, raw bytes; a NULL-terminated copy)
 * Memory contract: the boxes live on the bridge stack and are NOT released
 * with VariantClear (that would CoTaskMemFree the VT_LPSTR data, which is
 * owned by bridge-side string buffers). Everything reachable through args
 * is valid only during the callback call; the handler must neither free it
 * nor retain pointers beyond the call.
 *
 * @param ctx   user context supplied to RegisterNativeCallHandler at registration
 * @param name  first argument of the script-side NativeCall (function name string)
 * @param args  array of boxed arguments (unbox by vt; valid only during the call)
 * @param argc  number of elements in args (may be 0)
 * @return int  returned directly to the script side as the NativeCall result
 */
typedef int (*PFN_ScriptNativeCall)(void *ctx, LPCSTR name, const VARIANT *args, int argc);

/**
@brief
    Abstract interface required for all scripting support modules to be used with
    the SOUI system.
*/
#undef INTERFACE
#define INTERFACE IScriptModule
DECLARE_INTERFACE_(IScriptModule, IObjRef)
{
    /**
     * @brief Increment reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Get pointer to script engine
     * @return void* - pointer to script engine
     */
    STDMETHOD_(void *, GetScriptEngine)(THIS) PURE;

    /**
     * @brief Get identifier string of script module
     * @return LPCSTR - identifier string
     */
    STDMETHOD_(LPCSTR, getIdentifierString)(CTHIS) SCONST PURE;

    /**
     * @brief Execute script file
     * @param pszScriptFile - script file name
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, executeScriptFile)(THIS_ LPCSTR pszScriptFile) PURE;

    /**
     * @brief Execute script buffer
     * @param buff - Script buffer
     * @param sz - Buffer size
     * @return void
     */
    STDMETHOD_(void, executeScriptBuffer)(THIS_ LPCSTR buff, size_t sz) PURE;

    /**
     * @brief Execute scripted event handler
     * @param handler_name - Handler name
     * @param pEvt - Event parameter
     * @return BOOL - Returns TRUE if event handled, otherwise FALSE
     */
    STDMETHOD_(BOOL, executeScriptedEventHandler)(THIS_ LPCSTR handler_name, IEvtArgs * pEvt) PURE;

    /**
     * @brief Execute main function
     * @param hInst - Instance handle
     * @param pszWorkDir - Working directory
     * @param pszArgs - Extra parameters
     * @return int - Return code
     */
    STDMETHOD_(int, executeMain)(THIS_ HINSTANCE hInst, LPCSTR pszWorkDir, LPCSTR pszArgs) PURE;

    /**
     * @brief Get idle handler
     * @return IIdleHandler* - Idle handler pointer
     */
    STDMETHOD_(IIdleHandler *, getIdleHandler)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IScriptFactory
DECLARE_INTERFACE_(IScriptFactory, IObjRef)
{
    /**
     * @brief Increment reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement reference count
     * @return long - new reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create script module
     * @param ppScriptModule - Pointer to script module pointer
     * @return HRESULT
     */
    STDMETHOD_(HRESULT, CreateScriptModule)(THIS_ IScriptModule * *ppScriptModule) PURE;

    /**
     * @brief Register a handler for the script NativeCall(...) function
     * @param fn - handler function pointer, NULL to unregister
     * @param ctx - user context passed back to the handler on every call
     * @return void
     * @see PFN_ScriptNativeCall
     */
    STDMETHOD_(void, RegisterNativeCallHandler)(THIS_ PFN_ScriptNativeCall fn, void *ctx) PURE;
};

SNSEND

#endif /**< __SSCRIPTMODULE_I__H__ */

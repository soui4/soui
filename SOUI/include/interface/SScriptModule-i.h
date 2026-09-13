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
#include <stdint.h>

SNSBEGIN

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
};

SNSEND

#endif /**< __SSCRIPTMODULE_I__H__ */

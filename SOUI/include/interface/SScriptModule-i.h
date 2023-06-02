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

#pragma once
#include <utilities-def.h>
#include <interface/obj-ref-i.h>
#include <interface/SEvtArgs-i.h>
#include <interface/smsgloop-i.h>
#include <stdint.h>

SNSBEGIN

/*!
\brief
    Abstract interface required for all scripting support modules to be used with
    the SOUI system.
*/
#undef INTERFACE
#define INTERFACE IScriptModule
DECLARE_INTERFACE_(IScriptModule, IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return 新引用计数
    */
    STDMETHOD_(long,AddRef) (THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
    */
    STDMETHOD_(long,Release) (THIS) PURE;

	/**
	 * @brief 释放对象
	 * @return void
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;

	/**
     * GetScriptEngine
     * @brief    获得脚本引擎的指针
     * @return   void * -- 脚本引擎的指针
     * Describe
     */
    STDMETHOD_(void *, GetScriptEngine)(THIS) PURE;

	/*!
    \brief
        Return identification string for the ScriptModule.  If the internal id string has not been
        set by the ScriptModule creator, a generic string of "Unknown scripting module" will be
    returned.

    \return
        String object holding a string that identifies the ScriptModule in use.
    */
    STDMETHOD_(LPCSTR, getIdentifierString)(CTHIS) SCONST PURE;

    /*!
    \brief
        Execute a script file.

    \param pszScriptFile
        String object holding the filename of the script file that is to be executed

    */
    STDMETHOD_(BOOL, executeScriptFile)(THIS_ LPCSTR pszScriptFile) PURE;

    /*!
    \brief
        Execute a script buffer.

    \param buff
        buffer of the script that is to be executed

    \param sz
        size of buffer
    */
    STDMETHOD_(void, executeScriptBuffer)(THIS_ LPCSTR buff, size_t sz) PURE;

    /*!
    \brief
        Execute a scripted global 'event handler' function.  The function should take some kind of
    IEvtArgs like parameter that the concrete implementation of this function can create from the
    passed IEvtArgs based object.

    \param handler_name
        String object holding the name of the scripted handler function.

    \param pEvt
        IEvtArgs * based object that should be passed, by any appropriate means, to the scripted
    function.

    \return
        - true if the event was handled.
        - false if the event was not handled.
    */
    STDMETHOD_(BOOL, executeScriptedEventHandler)(THIS_ LPCSTR handler_name, IEvtArgs * pEvt) PURE;

   /*!
    \brief
        Execute main function

    \param hInst
        HINSTANCE handle of the instance.

	\param pszWorkDir
		LPCSTR work dir.

    \param pszArgs
        LPCSTR additional parameters.
    \return
        - int return code.
    */
	STDMETHOD_(int, executeMain)(THIS_ HINSTANCE hInst,LPCSTR pszWorkDir, LPCSTR pszArgs) PURE;

    STDMETHOD_(IIdleHandler*, getIdleHandler)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IScriptFactory
DECLARE_INTERFACE_(IScriptFactory, IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    STDMETHOD_(HRESULT, CreateScriptModule)(THIS_ IScriptModule * *ppScriptModule) PURE;
};

SNSEND

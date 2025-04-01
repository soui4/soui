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
     * @return long - 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return long - 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 获取脚本引擎的指针
     * @return void* - 脚本引擎的指针
     */
    STDMETHOD_(void *, GetScriptEngine)(THIS) PURE;

    /**
     * @brief 获取脚本模块的标识字符串
     * @return LPCSTR - 标识字符串
     */
    STDMETHOD_(LPCSTR, getIdentifierString)(CTHIS) SCONST PURE;

    /**
     * @brief 执行脚本文件
     * @param pszScriptFile - 脚本文件名
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, executeScriptFile)(THIS_ LPCSTR pszScriptFile) PURE;

    /**
     * @brief 执行脚本缓冲区
     * @param buff - 脚本缓冲区
     * @param sz - 缓冲区大小
     * @return void
     */
    STDMETHOD_(void, executeScriptBuffer)(THIS_ LPCSTR buff, size_t sz) PURE;

    /**
     * @brief 执行脚本化的事件处理函数
     * @param handler_name - 处理函数名称
     * @param pEvt - 事件参数
     * @return BOOL - 事件被处理返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, executeScriptedEventHandler)(THIS_ LPCSTR handler_name, IEvtArgs * pEvt) PURE;

    /**
     * @brief 执行主函数
     * @param hInst - 实例句柄
     * @param pszWorkDir - 工作目录
     * @param pszArgs - 额外参数
     * @return int - 返回代码
     */
    STDMETHOD_(int, executeMain)(THIS_ HINSTANCE hInst, LPCSTR pszWorkDir, LPCSTR pszArgs) PURE;

    /**
     * @brief 获取空闲处理器
     * @return IIdleHandler* - 空闲处理器指针
     */
    STDMETHOD_(IIdleHandler *, getIdleHandler)(THIS) PURE;
};

#undef INTERFACE
#define INTERFACE IScriptFactory
DECLARE_INTERFACE_(IScriptFactory, IObjRef)
{
    /**
     * @brief 增加引用计数
     * @return long - 新引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 减少引用计数
     * @return long - 新引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 创建脚本模块
     * @param ppScriptModule - 脚本模块指针的指针
     * @return HRESULT
     */
    STDMETHOD_(HRESULT, CreateScriptModule)(THIS_ IScriptModule * *ppScriptModule) PURE;
};

SNSEND

#endif // __SSCRIPTMODULE_I__H__

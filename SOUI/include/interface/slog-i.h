#pragma once

#include <interface/obj-ref-i.h>
#include <time.h>

SNSBEGIN

//! the max log content length.
enum
{
    LOG4Z_LOG_BUF_SIZE = 10240,
};

//! LOG Level
enum ENUM_LOG_LEVEL
{
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_ALARM,
    LOG_LEVEL_FATAL,
};

#undef INTERFACE
#define INTERFACE IOutputFileBuilder
DECLARE_INTERFACE_(IOutputFileBuilder,IObjRef)
{
	//!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;


    //生成LOG文件名
    //至少应该包含pszLogName，及curFileIndex这两个参数
    STDMETHOD_(BOOL, buildOutputFile)
    (CTHIS_ char *pszFileName, int nLen, const char *pszLogName, unsigned long pid, int curFileIndex) SCONST PURE;
};


//! log4z class
#undef INTERFACE
#define INTERFACE ILogMgr
DECLARE_INTERFACE_(ILogMgr, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //! Config or overwrite configure
    //! Needs to be called before ILog4zManager::Start,, OR Do not call.
    STDMETHOD_(BOOL, config)(THIS_ const char *configPath) PURE;
    STDMETHOD_(BOOL, configFromString)(THIS_ const char *configContent) PURE;

    //! Start Log Thread. This method can only be called once by one process.
    STDMETHOD_(BOOL, start)(THIS) PURE;

    //! Default the method will be calling at process exit auto.
    //! Default no need to call and no recommended.
    STDMETHOD_(BOOL, stop)(THIS) PURE;

    // pre-check the log filter. if filter out return false.
    STDMETHOD_(BOOL, prePushLog)(THIS_ int level) PURE;

    //! Push log, thread safe.
    STDMETHOD_(BOOL, pushLog)
    (THIS_ int level, const char *filter, const char *log, const char *file, int line, const char *func, const void *pRetAddr) PURE;

    //! set logger's attribute
	STDMETHOD_(BOOL, isLoggerEnable)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, enableLogger)(THIS_ BOOL enable) PURE;
	STDMETHOD_(BOOL, setLoggerName)(THIS_ const char *name_u8) PURE;
    STDMETHOD_(BOOL, setLoggerPath)(THIS_ const char *path_u8) PURE;
    STDMETHOD_(BOOL, setLoggerLevel)(THIS_ int nLevel) PURE;
    STDMETHOD_(BOOL, setLoggerFileLine)(THIS_ BOOL enable) PURE;
    STDMETHOD_(BOOL, setLoggerDisplay)(THIS_ BOOL enable) PURE;
    STDMETHOD_(BOOL, setLoggerOutFile)(THIS_ BOOL enable) PURE;
    STDMETHOD_(BOOL, setLoggerLimitsize)(THIS_ unsigned int limitsize) PURE;

    //设置LOG输出到文件的规则
    STDMETHOD_(void, setOutputFileBuilder)(THIS_ IOutputFileBuilder * pOutputFileBuilder) PURE;
};

SNSEND
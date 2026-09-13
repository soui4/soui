#ifndef __SLOG_I__H__
#define __SLOG_I__H__

#include <interface/obj-ref-i.h>
#include <windows.h>
#include <time.h>

SNSBEGIN

/** the max log content length. */
enum
{
    LOG4Z_LOG_BUF_SIZE = 10240,
};

#undef INTERFACE
#define INTERFACE IOutputFileBuilder
DECLARE_INTERFACE_(IOutputFileBuilder, IObjRef)
{
    /** Add reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Release reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Release object */
    /**
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /** Generate LOG file name */
    /**
     * @param pszFileName - output file name buffer
     * @param nLen - buffer length
     * @param pszLogName - log name
     * @param pid - process ID
     * @param curFileIndex - current file index
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, buildOutputFile)
    (CTHIS_ char *pszFileName, int nLen, const char *pszLogName, unsigned long pid, int curFileIndex) SCONST PURE;
};

/** log4z class */
#undef INTERFACE
#define INTERFACE ILogMgr
DECLARE_INTERFACE_(ILogMgr, IObjRef)
{
    /** Add reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /** Release reference */
    /**
     * @return long - current reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /** Release object */
    /**
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /** Configure or override configuration */
    /**
     * @param configPath - configuration file path
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, config)(THIS_ const char *configPath) PURE;

    /** Configure from string */
    /**
     * @param configContent - configuration content string
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, configFromString)(THIS_ const char *configContent) PURE;

    /** Start log thread */
    /**
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, start)(THIS) PURE;

    /** Stop log thread */
    /**
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, stop)(THIS) PURE;

    /** Pre-check log filter */
    /**
     * @param level - log level
     * @return BOOL - returns TRUE if filter passed, FALSE if filtered out
     */
    STDMETHOD_(BOOL, prePushLog)(THIS_ int level) PURE;

    /** Push log, thread-safe */
    /**
     * @param level - log level
     * @param filter - log filter
     * @param log - log content
     * @param file - file name
     * @param line - line number
     * @param func - function name
     * @param pRetAddr - return address
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, pushLog)
    (THIS_ int level, const char *filter, const char *log, const char *file, int line, const char *func, const void *pRetAddr) PURE;

    /** Check whether logger is enabled */
    /**
     * @return BOOL - returns TRUE if enabled, FALSE if not enabled
     */
    STDMETHOD_(BOOL, isLoggerEnable)(CTHIS) SCONST PURE;

    /** Enable or disable logger */
    /**
     * @param enable - enable flag
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, enableLogger)(THIS_ BOOL enable) PURE;

    /** Set logger name */
    /**
     * @param name_u8 - logger name
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, setLoggerName)(THIS_ const char *name_u8) PURE;

    /** Set logger path */
    /**
     * @param path_u8 - logger path
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, setLoggerPath)(THIS_ const char *path_u8) PURE;

    /** Set log level */
    /**
     * @param nLevel - log level
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, setLoggerLevel)(THIS_ int nLevel) PURE;

    /** Enable or disable file line number */
    /**
     * @param enable - enable flag
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, setLoggerFileLine)(THIS_ BOOL enable) PURE;

    /** Enable or disable log display */
    /**
     * @param enable - enable flag
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, setLoggerDisplay)(THIS_ BOOL enable) PURE;

    /** Enable or disable log output to file */
    /**
     * @param enable - enable flag
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, setLoggerOutFile)(THIS_ BOOL enable) PURE;

    /** Set log file size limit */
    /**
     * @param limitsize - file size limit
     * @return BOOL - returns TRUE on success, FALSE on failure
     */
    STDMETHOD_(BOOL, setLoggerLimitsize)(THIS_ unsigned int limitsize) PURE;

    /** Set rules for log output to file */
    /**
     * @param pOutputFileBuilder - output file builder
     * @return void
     */
    STDMETHOD_(void, setOutputFileBuilder)(THIS_ IOutputFileBuilder * pOutputFileBuilder) PURE;
};

SNSEND
#endif /**< __SLOG_I__H__ */

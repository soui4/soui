#ifndef __SLOG_CAPI_H__
#define __SLOG_CAPI_H__

#include "../slog-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SLog Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 */

/* IOutputFileBuilder C API Macros */
#define IOutputFileBuilder_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IOutputFileBuilder_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IOutputFileBuilder_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define IOutputFileBuilder_buildOutputFile(This, pszFileName, nLen, pszLogName, pid, curFileIndex) \
    ((This)->lpVtbl->buildOutputFile(This, pszFileName, nLen, pszLogName, pid, curFileIndex))

/* ILogMgr C API Macros */
#define ILogMgr_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define ILogMgr_Release(This) \
    ((This)->lpVtbl->Release(This))

#define ILogMgr_OnFinalRelease(This) \
    ((This)->lpVtbl->OnFinalRelease(This))

#define ILogMgr_config(This, configPath) \
    ((This)->lpVtbl->config(This, configPath))

#define ILogMgr_configFromString(This, configContent) \
    ((This)->lpVtbl->configFromString(This, configContent))

#define ILogMgr_start(This) \
    ((This)->lpVtbl->start(This))

#define ILogMgr_stop(This) \
    ((This)->lpVtbl->stop(This))

#define ILogMgr_prePushLog(This, level) \
    ((This)->lpVtbl->prePushLog(This, level))

#define ILogMgr_pushLog(This, level, filter, log, file, line, func, pRetAddr) \
    ((This)->lpVtbl->pushLog(This, level, filter, log, file, line, func, pRetAddr))

#define ILogMgr_isLoggerEnable(This) \
    ((This)->lpVtbl->isLoggerEnable(This))

#define ILogMgr_enableLogger(This, enable) \
    ((This)->lpVtbl->enableLogger(This, enable))

#define ILogMgr_setLoggerName(This, name_u8) \
    ((This)->lpVtbl->setLoggerName(This, name_u8))

#define ILogMgr_setLoggerPath(This, path_u8) \
    ((This)->lpVtbl->setLoggerPath(This, path_u8))

#define ILogMgr_setLoggerLevel(This, nLevel) \
    ((This)->lpVtbl->setLoggerLevel(This, nLevel))

#define ILogMgr_setLoggerFileLine(This, enable) \
    ((This)->lpVtbl->setLoggerFileLine(This, enable))

#define ILogMgr_setLoggerDisplay(This, enable) \
    ((This)->lpVtbl->setLoggerDisplay(This, enable))

#define ILogMgr_setLoggerOutFile(This, enable) \
    ((This)->lpVtbl->setLoggerOutFile(This, enable))

#define ILogMgr_setLoggerLimitsize(This, limitsize) \
    ((This)->lpVtbl->setLoggerLimitsize(This, limitsize))

#define ILogMgr_setOutputFileBuilder(This, pOutputFileBuilder) \
    ((This)->lpVtbl->setOutputFileBuilder(This, pOutputFileBuilder))

#ifdef __cplusplus
}
#endif

#endif /* __SLOG_CAPI_H__ */

#ifndef __EXP_ILOGMGR_H__
#define __EXP_ILOGMGR_H__

#include <interface/slog-i.h>
#include "toobj.h"

static void ExpLua_ILogMgr_Inner(lua_State *L)
{
    lua_tinker::class_add<ILogMgr>(L, "ILogMgr");
    lua_tinker::class_inh<ILogMgr, IObjRef>(L);

    lua_tinker::class_def<ILogMgr>(L, "config", &ILogMgr::config);
    lua_tinker::class_def<ILogMgr>(L, "configFromString", &ILogMgr::configFromString);
    lua_tinker::class_def<ILogMgr>(L, "start", &ILogMgr::start);
    lua_tinker::class_def<ILogMgr>(L, "stop", &ILogMgr::stop);
    lua_tinker::class_def<ILogMgr>(L, "isLoggerEnable", &ILogMgr::isLoggerEnable);
    lua_tinker::class_def<ILogMgr>(L, "enableLogger", &ILogMgr::enableLogger);
    lua_tinker::class_def<ILogMgr>(L, "setLoggerName", &ILogMgr::setLoggerName);
    lua_tinker::class_def<ILogMgr>(L, "setLoggerPath", &ILogMgr::setLoggerPath);
    lua_tinker::class_def<ILogMgr>(L, "setLoggerLevel", &ILogMgr::setLoggerLevel);
    lua_tinker::class_def<ILogMgr>(L, "setLoggerFileLine", &ILogMgr::setLoggerFileLine);
    lua_tinker::class_def<ILogMgr>(L, "setLoggerDisplay", &ILogMgr::setLoggerDisplay);
    lua_tinker::class_def<ILogMgr>(L, "setLoggerOutFile", &ILogMgr::setLoggerOutFile);
    lua_tinker::class_def<ILogMgr>(L, "setLoggerLimitsize", &ILogMgr::setLoggerLimitsize);
    // setOutputFileBuilder needs an IOutputFileBuilder* which is not exported
    // to lua; skip it.

    DEF_CAST_OBJREF(L, ILogMgr);
}

#endif // __EXP_ILOGMGR_H__

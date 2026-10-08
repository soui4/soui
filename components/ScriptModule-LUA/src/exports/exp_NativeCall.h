#pragma once
// ============================================================================
// exp_NativeCall.h
// Generic C++ -> lua function-export bridge. Lua-side usage:
//     local ret = NativeCall("funcName", arg1, arg2, ...)
// The first argument is a function-name string, followed by any number of
// variadic arguments (nil/boolean/integer/number/string are supported;
// userdata/table are not). The return value is an int.
//
// Two ways to export C++ functionality to the lua side:
//   1. Register a handler via IScriptFactory::RegisterNativeCallHandler
//      (PFN_ScriptNativeCall; the variadic arguments are passed as a
//      VARIANT array plus a count, see SScriptModule-i.h);
//   2. Without a registered handler, NativeCall logs a warning and
//      returns -1.
//
// This header holds declarations only (safe to include from multiple TUs);
// all implementations live in exp_soui.cpp.
// ============================================================================

#include <string.h>
#include <interface/SScriptModule-i.h>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
};

SNSBEGIN

// ----------------------------------------------------------------------------
// Handler registry (NativeCall_SetHandler is called through
// SIScriptFactory::RegisterNativeCallHandler):
//   - fn == NULL unregisters; NativeCall then logs a warning and returns -1.
//   - ctx is passed back to the handler untouched on every call.
// ----------------------------------------------------------------------------
void NativeCall_SetHandler(PFN_ScriptNativeCall fn, void *ctx);
PFN_ScriptNativeCall NativeCall_GetHandler(void **ppCtx);

// Registers the NativeCall global function in lua (called by SOUI_Export_Lua)
BOOL ExpLua_NativeCall(lua_State *L);

SNSEND

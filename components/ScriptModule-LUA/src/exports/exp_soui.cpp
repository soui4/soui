#include "stdafx.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
};

#include <lua_tinker.h>

using namespace SNS;

#include "exp_Basic.h"
#include "exp_SMatrix.h"
#include "exp_IBitmapS.h"
#include "exp_string.h"
#include "exp_IXml.h"
#include "exp_ctrls.h"
#include "exp_Window.h"
#include "exp_IObjRef.h"
#include "exp_ISouifac.h"
#include "exp_IString.h"
#include "exp_ITimer.h"
#include "exp_IApp.h"
#include "exp_IResProvider.h"
#include "exp_IResProviderMgr.h"
#include "exp_IScriptModule.h"
#include "exp_IObject.h"
#include "exp_IWindow.h"
#include "exp_IContainer.h"
#include "exp_IEvtArgs.h"
#include "exp_eventArgs.h"
#include "exp_INativeWnd.h"
#include "exp_IHostWnd.h"
#include "exp_ICtrl.h"
#include "exp_IAdapter.h"
#include "exp_IMenu.h"
#include "exp_IMenuEx.h"
#include "exp_IAnimation.h"
#include "exp_IValueAnimator.h"
#include "exp_IInterpolator.h"

#include "exp_IPropertyAnimator.h"
#include "exp_IAnimatorSet.h"
#include "exp_LuaValueAnimator.h"
#include "exp_SXml.h"
#include "exp_ISkinObj.h"
#include "exp_ITranslator.h"
#include "exp_ILogMgr.h"
#include "exp_global.h"
#include "exp_NativeCall.h"
#include "exp_SysApi.h"
#include <commgr2.h>

static SComMgr2 s_comMgr;

SComMgr2* GetLuaScriptComMgr2() {
	return &s_comMgr;
}

// ============================================================================
// NativeCall implementation (lua: NativeCall("name", ...) -> int)
// ============================================================================
SNSBEGIN

// Handler registry: registered via IScriptFactory::RegisterNativeCallHandler
static PFN_ScriptNativeCall s_fnNativeCallHandler = NULL;
static void *s_pNativeCallCtx = NULL;

void NativeCall_SetHandler(PFN_ScriptNativeCall fn, void *ctx)
{
	s_fnNativeCallHandler = fn;
	s_pNativeCallCtx = ctx;
}

PFN_ScriptNativeCall NativeCall_GetHandler(void **ppCtx)
{
	if (ppCtx) *ppCtx = s_pNativeCallCtx;
	return s_fnNativeCallHandler;
}

// Max number of variadic arguments accepted by the lua-side NativeCall
#define NC_MAX_ARGS 16

// Native lua_CFunction entry: variadic arguments require a raw lua_CFunction
// (lua_tinker::def only supports fixed signatures). Boxes lua stack values
// into a VARIANT array (nil->VT_EMPTY, boolean->VT_BOOL, integer->VT_I8,
// number->VT_R8, string->VT_LPSTR). String data is kept alive in SStringA
// buffers valid for the duration of the call; no VariantClear is performed
// (it would CoTaskMemFree the VT_LPSTR data owned by those buffers).
static int LuaNativeCall(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs < 1 || !lua_isstring(L, 1))
	{
		return luaL_error(L, "NativeCall: first argument must be a function name (string)");
	}
	const char *name = lua_tostring(L, 1);

	int n = nargs - 1;
	if (n > NC_MAX_ARGS)
	{
		return luaL_error(L, "NativeCall '%s': too many arguments (max %d)", name, NC_MAX_ARGS);
	}

	// Box lua values -> VARIANT; the boxes are plain stack values, nothing
	// inside them needs freeing.
	VARIANT boxes[NC_MAX_ARGS];
	SStringA strs[NC_MAX_ARGS];
	for (int i = 2; i <= nargs; i++)
	{
		VARIANT &arg = boxes[i - 2];
		memset(&arg, 0, sizeof(arg));
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TNIL:
			break;
		case LUA_TBOOLEAN:
			arg.vt = VT_BOOL;
			arg.boolVal = lua_toboolean(L, i) ? VARIANT_TRUE : VARIANT_FALSE;
			break;
		case LUA_TNUMBER:
			if (lua_isinteger(L, i))
			{
				arg.vt = VT_I8;
				arg.llVal = (LONGLONG)lua_tointeger(L, i);
			}
			else
			{
				arg.vt = VT_R8;
				arg.dblVal = lua_tonumber(L, i);
			}
			break;
		case LUA_TSTRING:
			{
				size_t len = 0;
				const char *p = lua_tolstring(L, i, &len);
				strs[i - 2] = SStringA(p, (int)len);
				arg.vt = VT_LPSTR;
				arg.pcVal = (char*)strs[i - 2].c_str();
			}
			break;
		default:
			return luaL_error(L, "NativeCall '%s': unsupported argument #%d type '%s'",
							  name, i - 1, lua_typename(L, t));
		}
	}

	// Dispatch: invoke the registered handler; with no handler registered,
	// log a warning and return -1.
	int ret = -1;
	void *ctx = NULL;
	PFN_ScriptNativeCall fn = NativeCall_GetHandler(&ctx);
	if (fn)
		ret = fn(ctx, name, boxes, n);
	else
	{
		SLOGW2("luascript") << "NativeCall: no handler registered, name='" << name << "' (" << n << " args)";
	}
	lua_pushinteger(L, ret);
	return 1;
}

BOOL ExpLua_NativeCall(lua_State *L)
{
	try
	{
		lua_register(L, "NativeCall", LuaNativeCall);
		return TRUE;
	}
	catch (...)
	{
		return FALSE;
	}
}
SNSEND

BOOL SOUI_Export_Lua(lua_State *L)
{
	BOOL bRet=TRUE;
	if(bRet) bRet=ExpLua_Basic(L);
	if(bRet) bRet=ExpLua_SMatrix(L);
	if(bRet) bRet=ExpLua_IBitmapS(L);
	if(bRet) bRet=ExpLua_String(L);
	if(bRet) bRet=ExpLua_IObjRef(L);
	if(bRet) bRet=ExpLua_ISouiFactory(L);
	if(bRet) bRet=ExpLua_IStringA(L);
	if(bRet) bRet=ExpLua_IStringW(L);
	if(bRet) bRet=ExpLua_IXml(L);
	if(bRet) bRet=ExpLua_IResProvider(L);
	if(bRet) bRet=ExpLua_IResProviderMgr(L);
	if(bRet) bRet=ExpLua_IAppication(L);
	if(bRet) bRet=ExpLua_IMenu(L);
	if(bRet) bRet=ExpLua_IMenuEx(L);
	if(bRet) bRet=ExpLua_ITimer(L);

	if(bRet) bRet=ExpLua_IObject(L);
	if(bRet) bRet=ExpLua_IWindow(L);
	if(bRet) bRet=ExpLua_IContainer(L);

	if(bRet) bRet=ExpLua_Window(L);

	if(bRet) bRet=ExpLua_IEvtArgs(L);

	if(bRet) bRet=ExpLua_Ctrls(L);
	if(bRet) bRet=ExpLua_IScriptModule(L);

    if(bRet) bRet=ExpLua_EventArgs(L);
	if(bRet) bRet=ExpLua_INativeWnd(L);
	if(bRet) bRet=ExpLua_IHostWnd(L);
	if(bRet) bRet=ExpLua_IHostDialog(L);
	
	if(bRet) bRet=ExpLua_ICtrl(L);
	if(bRet) bRet=ExpLua_IAdapter(L);
	if(bRet) bRet=ExpLua_IAnimation(L);
	if(bRet) bRet=ExpLua_IValueAnimator(L);
	if(bRet) bRet=ExpLua_IInterpolator(L);

	
	if(bRet) bRet=ExpLua_Global(L);
	if(bRet) bRet=ExpLua_NativeCall(L);
	if(bRet) bRet=ExpLua_SysApi(L);

	// new exports (parity with soui4js), self-contained registrations
	if(bRet)
	{
		ExpLua_IPropertyValuesHolder_Inner(L);
		ExpLua_IPropertyAnimator_Inner(L);
		ExpLua_IAnimatorSet_Inner(L);
		ExpLua_IAnimatorGroup_Inner(L);
		ExpLua_ISkinObj_Inner(L);
		ExpLua_ITranslator_Inner(L);
		ExpLua_ITranslatorMgr_Inner(L);
		ExpLua_ILogMgr_Inner(L);
		ExpLua_LuaValueAnimator(L);
		bRet = ExpLua_SXml(L);
	}

	return bRet;
}
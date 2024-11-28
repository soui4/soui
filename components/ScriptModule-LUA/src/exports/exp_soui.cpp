#include "stdafx.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
};

#include <lua_tinker.h>

using namespace SOUI;

#include "exp_Basic.h"
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
#include "exp_global.h"
#include "exp_SysApi.h"

BOOL SOUI_Export_Lua(lua_State *L)
{
	BOOL bRet=TRUE;
	if(bRet) bRet=ExpLua_Basic(L);
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
	if(bRet) bRet=ExpLua_SysApi(L);

	return bRet;
}
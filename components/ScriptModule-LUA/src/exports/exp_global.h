#include <string/strcpcvt.h>
#include "luaFunSlot.h"
#include "luaAdapter.h"

//导出基本结构体类型
UINT rgb(int r,int g,int b)
{
	return RGBA(r,g,b,255);
}

UINT rgba(int r,int g, int b, int a)
{
	return RGBA(r,g,b,a);
}

IHostWnd * getHostFromInitEvent(IEvtArgs *pEvt)
{
	lua_State *state = lua_tinker::get_state();
	SRootWindow *pRoot = sobj_cast<SRootWindow>(pEvt->Sender());
	if(pRoot)
		return pRoot->GetHostWnd();
	else
		return NULL;
}

struct LuaMsgHandlerCtx{
	lua_State *luaState;
	INativeWnd * pNativeWnd;
	SStringA strLuaFun;
	void *pCtx;
};

BOOL LuaMsgHandler(const LPMSG pMsg, LRESULT *pRes, void *ctx){
	LuaMsgHandlerCtx *pCtx2 = (LuaMsgHandlerCtx*)ctx;
	int nRet = lua_tinker::call<int>(pCtx2->luaState,pCtx2->strLuaFun,pCtx2->pNativeWnd,pMsg->message,pMsg->wParam,pMsg->lParam,pRes);
	return nRet!=0;
}

void NativeWnd_SetMsgHandler(INativeWnd *pNativeWnd, LPCSTR pszLuaFun, void *ctx){
	SStringA strFun(pszLuaFun);
	if(strFun.IsEmpty())
	{
		MsgHandlerInfo *pInfo = pNativeWnd->GetMsgHandler();
		if(pInfo->fun){
			delete (LuaMsgHandlerCtx*)pInfo->ctx;
			pNativeWnd->SetMsgHandler(NULL,NULL);
		}
	}else
	{
		LuaMsgHandlerCtx *pCtx2 = new LuaMsgHandlerCtx;
		pCtx2->luaState = lua_tinker::get_state();
		pCtx2->pNativeWnd = pNativeWnd;
		pCtx2->pCtx = ctx;
		pCtx2->strLuaFun = strFun;
		pNativeWnd->SetMsgHandler(LuaMsgHandler,pCtx2);
	}
}

struct LuaEventHandlerCtx{
	lua_State *luaState;
	IHostWnd * pHostWnd;
	SStringA strLuaFun;
	void *pCtx;
};

BOOL LuaEvtHandler(IEvtArgs *e, void *ctx){
	LuaEventHandlerCtx *pCtx2 = (LuaEventHandlerCtx*)ctx;
	int nRet = lua_tinker::call<int>(pCtx2->luaState,pCtx2->strLuaFun,pCtx2->pHostWnd,e, pCtx2->pCtx);
	return nRet!=0;
}

void HostWnd_SetEventHandler(IHostWnd *pHostWnd, LPCSTR pszLuaFun, void *ctx){
	SStringA strFun(pszLuaFun);
	if(strFun.IsEmpty()){
		EventHandlerInfo * pInfo = pHostWnd->GetEventHandler();
		if(pInfo->fun){
			delete (LuaEventHandlerCtx*)pInfo->ctx;
			pHostWnd->SetEventHandler(NULL,NULL);
		}
	}else{
		LuaEventHandlerCtx *pCtx2 = new LuaEventHandlerCtx;
		pCtx2->luaState = lua_tinker::get_state();
		pCtx2->pHostWnd = pHostWnd;
		pCtx2->pCtx = ctx;
		pCtx2->strLuaFun = pszLuaFun;
		pHostWnd->SetEventHandler(LuaEvtHandler,pCtx2);
	}
}

void slog(LPCSTR pszLog){
	SLOGI2("luascript")<<pszLog;
}


IEvtSlot * CreateEventSlot(LPCSTR pszLuaFun){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaFunctionSlot(luaState,pszLuaFun);
}

ILvAdapter * CreateLvAdapter(void *ctx){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaLvAdapter(luaState,ctx);
}

IMcAdapter * CreateMcAdapter(void *ctx){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaMcAdapter(luaState,ctx);
}

BOOL InitFileResProvider(IResProvider *pResProvider, const char * path)
{
	SStringT strPath=S_CA2T(path);
	return pResProvider->Init((WPARAM)strPath.c_str(),0);
}

BOOL InitPEResProvider(IResProvider *pResProvider, const char * path)
{
	SStringT strPath=S_CA2T(path);
	return pResProvider->Init((WPARAM)strPath.c_str(),1);
}

IApplication * GetApp(){
	return SApplication::getSingletonPtr();
}

HWND GetNullHwnd(){
	return NULL;
}

IHostWnd * GetHostWndFromObject(IObject *pObj)
{
	SWindow *pWnd = sobj_cast<SWindow>(pObj);
	if(!pWnd) return NULL;
	SWindow *pRoot = pWnd->GetRoot();
	SRootWindow *pRootWnd = sobj_cast<SRootWindow>(pRoot);
	if(pRootWnd){
		return pRootWnd->GetHostWnd();
	}else{
		return NULL;
	}
}

BOOL ExpLua_Global(lua_State *L)
{
	try{
		lua_tinker::set_print_callback(slog);

		lua_tinker::def(L,"RGB",rgb);
		lua_tinker::def(L,"RGBA",rgba);
		lua_tinker::def(L,"slog",slog);

		lua_tinker::def(L,"S_W2A",&SStrCpCvt::CvtW2A);
		lua_tinker::def(L,"S_A2W",&SStrCpCvt::CvtA2W);
		lua_tinker::def(L,"S_A2A",&SStrCpCvt::CvtA2A);
		lua_tinker::def(L,"S_W2W",&SStrCpCvt::CvtW2W);

		lua_tinker::def(L,"GetActiveWindow",GetActiveWindow);
		lua_tinker::def(L,"GetNullHwnd",GetNullHwnd);
		lua_tinker::def(L,"GetHostWndFromObject",GetHostWndFromObject);
		lua_tinker::def(L,"GetApp",GetApp);
		
		lua_tinker::def(L,"SMessageBox",SMessageBox);
		lua_tinker::def(L,"InitFileResProvider",InitFileResProvider);
		lua_tinker::def(L,"InitPEResProvider",InitPEResProvider);

		lua_tinker::def(L,"getHostFromInitEvent",getHostFromInitEvent);

		lua_tinker::def(L,"NativeWnd_SetMsgHandler",NativeWnd_SetMsgHandler);

		lua_tinker::def(L,"HostWnd_SetEventHandler",HostWnd_SetEventHandler);
		
		lua_tinker::def(L,"CreateEventSlot",CreateEventSlot);
		lua_tinker::def(L,"CreateSouiFactory",CreateSouiFactory);

		lua_tinker::def(L,"CreateLvAdapter",CreateLvAdapter);
		lua_tinker::def(L,"CreateMcAdapter",CreateMcAdapter);


		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
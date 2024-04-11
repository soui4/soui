#include <string/strcpcvt.h>
#include <interface/SWindow-i.h>
#include <interface/smenu-i.h>
#include <interface/smenuex-i.h>

#include "luaFunSlot.h"
#include "luaAdapter.h"
#include "luaAnimationListener.h"

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
	IHostWnd * pHostWnd;
	SStringA strLuaFun;
	void *pCtx;
};

BOOL LuaMsgHandler(const LPMSG pMsg, LRESULT *pRes, void *ctx){
	LuaMsgHandlerCtx *pCtx2 = (LuaMsgHandlerCtx*)ctx;
	int nRet = lua_tinker::call<int>(pCtx2->luaState,pCtx2->strLuaFun,pCtx2->pHostWnd,pMsg->message,pMsg->wParam,pMsg->lParam,pRes);
	return nRet!=0;
}

void HostWnd_SetMsgHandler(IHostWnd *pHostWnd, LPCSTR pszLuaFun, void *ctx){
	SStringA strFun(pszLuaFun);
	if(strFun.IsEmpty())
	{
		MsgHandlerInfo *pInfo = pHostWnd->GetNative()->GetMsgHandler();
		if(pInfo->fun){
			delete (LuaMsgHandlerCtx*)pInfo->ctx;
			pHostWnd->GetNative()->SetMsgHandler(NULL,NULL);
		}
	}else
	{
		LuaMsgHandlerCtx *pCtx2 = new LuaMsgHandlerCtx;
		pCtx2->luaState = lua_tinker::get_state();
		pCtx2->pHostWnd = pHostWnd;
		pCtx2->pCtx = ctx;
		pCtx2->strLuaFun = strFun;
		pHostWnd->GetNative()->SetMsgHandler(LuaMsgHandler,pCtx2);
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

LuaLvAdapter * CreateLvAdapter(int ctx){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaLvAdapter(luaState,ctx);
}

LuaMcAdapter * CreateMcAdapter(int ctx){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaMcAdapter(luaState,ctx);
}

LuaTvAdapter * CreateTvAdapter(int ctx){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaTvAdapter(luaState,ctx);
}

LuaAnimationListener * CreateAnimationListener(int ctx){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaAnimationListener(luaState,ctx);
}

LuaAnimatorListener * CreateValueAnimatorListener(int ctx){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaAnimatorListener(luaState,ctx);
}

LuaAnimatorUpdateListener * CreateValueAnimatorUpdateListener(int ctx){
	lua_State *luaState = lua_tinker::get_state();
	return new LuaAnimatorUpdateListener(luaState,ctx);
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

void LuaConnect(IWindow *pWnd, int idEvt,LPCSTR pszFun){
	IEvtSlot * pSlot = CreateEventSlot(pszFun);
	pWnd->SubscribeEvent(idEvt,pSlot);
	pSlot->Release();
}

int TrackPopupIMenu(IMenu *pMenu,UINT flag,int x,int y, int scale){
	return pMenu->TrackPopupMenu(flag,x,y,NULL,NULL,scale);
}

int TrackPopupIMenuEx(IMenuEx *pMenu,UINT flag,int x,int y, int scale){
	return pMenu->TrackPopupMenu(flag,x,y,NULL,scale);
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


void SClientToScreen(INativeWnd *pWnd,CRect & rc){
	pWnd->ClientToScreen2(&rc);
}

void SScreenToClient(INativeWnd *pWnd,CRect & rc){
	pWnd->ScreenToClient2(&rc);
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

		lua_tinker::def(L,"GetHostWndFromObject",GetHostWndFromObject);
		lua_tinker::def(L,"LuaConnect",LuaConnect);
		lua_tinker::def(L,"GetApp",GetApp);
		lua_tinker::def(L,"TrackPopupIMenu",TrackPopupIMenu);
		lua_tinker::def(L,"TrackPopupIMenuEx",TrackPopupIMenuEx);

		lua_tinker::def(L,"SMessageBox",SMessageBox);
		lua_tinker::def(L,"InitFileResProvider",InitFileResProvider);
		lua_tinker::def(L,"InitPEResProvider",InitPEResProvider);

		lua_tinker::def(L,"getHostFromInitEvent",getHostFromInitEvent);

		lua_tinker::def(L,"HostWnd_SetMsgHandler", HostWnd_SetMsgHandler);

		lua_tinker::def(L,"HostWnd_SetEventHandler",HostWnd_SetEventHandler);
		
		lua_tinker::def(L,"CreateEventSlot",CreateEventSlot);
		lua_tinker::def(L,"CreateSouiFactory",CreateSouiFactory);

		lua_tinker::def(L,"CreateLvAdapter",CreateLvAdapter);
		lua_tinker::def(L,"CreateMcAdapter",CreateMcAdapter);
		lua_tinker::def(L,"CreateTvAdapter",CreateTvAdapter);

		lua_tinker::def(L,"CreateAnimationListener",CreateAnimationListener);
		lua_tinker::def(L,"CreateValueAnimatorListener",CreateValueAnimatorListener);
		lua_tinker::def(L,"CreateValueAnimatorUpdateListener",CreateValueAnimatorUpdateListener);

		lua_tinker::def(L,"SScreenToClient",SScreenToClient);
		lua_tinker::def(L,"SClientToScreen",SClientToScreen);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
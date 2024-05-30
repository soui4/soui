#pragma once
SNSBEGIN

class LuaAnimationListener : public IAnimationListener
{
private:
	lua_State * m_luaState;
	int m_ctxId;
	SStringA m_luaOnAniStart;
	SStringA m_luaOnAniStop;
	SStringA m_luaOnAniRepeat;
	SStringA m_luaOnAniPauseChange;
public:
	LuaAnimationListener(lua_State *luaState,int nCtxId):m_luaState(luaState),m_ctxId(nCtxId){

	}

	enum{
		fid_onAniStart = 0,
		fid_onAniStop,
		fid_onAniRepreat,
		fid_onAniPauseChange,
	};
	void initCallback(int fid, LPCSTR cbFun){
		switch(fid){
case fid_onAniStart:m_luaOnAniStart=cbFun;break;
case fid_onAniStop:m_luaOnAniStop=cbFun;break;
case fid_onAniRepreat:m_luaOnAniRepeat=cbFun;break;
case fid_onAniPauseChange:m_luaOnAniPauseChange=cbFun;break;
default:lua_tinker::print_error(m_luaState,"unknown fid %d",fid);break;
		}
	}
public:
	STDMETHOD_(void, OnAnimationStart)(THIS_ IAnimation * animation) {
		if(m_luaOnAniStart.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaOnAniStart,m_ctxId,animation);
	}

	STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation * animation) {
		if(m_luaOnAniStop.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaOnAniStop,m_ctxId,animation);
	}
	STDMETHOD_(void, OnAnimationRepeat)(THIS_ IAnimation * animation) {
		if(m_luaOnAniRepeat.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaOnAniRepeat,m_ctxId,animation);
	}

	STDMETHOD_(void, OnAnimationPauseChange)(THIS_ IAnimation * animation,BOOL bPaused) {
		if(m_luaOnAniPauseChange.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaOnAniPauseChange,m_ctxId,animation,bPaused);
	}

};

class LuaAnimatorListener : public IAnimatorListener
{
private:
	lua_State * m_luaState;
	int m_ctxId;
	SStringA m_luaOnAniStart;
	SStringA m_luaOnAniStop;
	SStringA m_luaOnAniRepeat;

public:
	LuaAnimatorListener(lua_State *luaState,int nCtxId):m_luaState(luaState),m_ctxId(nCtxId){

	}

	enum{
		fid_onAniStart = 0,
		fid_onAniStop,
		fid_onAniRepreat,
	};
	void initCallback(int fid, LPCSTR cbFun){
		switch(fid){
case fid_onAniStart:m_luaOnAniStart=cbFun;break;
case fid_onAniStop:m_luaOnAniStop=cbFun;break;
case fid_onAniRepreat:m_luaOnAniRepeat=cbFun;break;
default:lua_tinker::print_error(m_luaState,"unknown fid %d",fid);break;
		}
	}

public:
	STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator * animation) {
		if(m_luaOnAniStart.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaOnAniStart,m_ctxId,animation);
	}

	STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator * animation) {
		if(m_luaOnAniStop.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaOnAniStop,m_ctxId,animation);
	}
	STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator * animation) {
		if(m_luaOnAniRepeat.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaOnAniRepeat,m_ctxId,animation);
	}

};

class LuaAnimatorUpdateListener : public IAnimatorUpdateListener{
private:
	lua_State * m_luaState;
	int m_ctxId;
	SStringA m_luaOnAniUpdate;
public:
	LuaAnimatorUpdateListener(lua_State *luaState,int nCtxId):m_luaState(luaState),m_ctxId(nCtxId){

	}
	enum{
		fid_onAniUpdate = 0,
	};

	void initCallback(int fid, LPCSTR cbFun){
		switch(fid){
case fid_onAniUpdate:m_luaOnAniUpdate=cbFun;break;
default:lua_tinker::print_error(m_luaState,"unknown fid %d",fid);break;
		}
	}

public:
	STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator * pAnimator) {
		if(m_luaOnAniUpdate.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaOnAniUpdate,m_ctxId,pAnimator);
	}

};

SNSEND
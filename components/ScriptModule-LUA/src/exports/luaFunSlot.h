#pragma once

SNSBEGIN

class LuaFunctionSlot : public TObjRefImpl<IEvtSlot>
{
public:
	//! Slot function type.
	LuaFunctionSlot(lua_State *pLuaState,LPCSTR pszLuaFun) 
		: m_pLuaState(pLuaState)
		, m_luaFun(pszLuaFun)
	{}


	STDMETHOD_(BOOL,Run)(THIS_ IEvtArgs *pArg) OVERRIDE
	{
		return lua_tinker::call<bool>(m_pLuaState,m_luaFun,pArg);
	}
	STDMETHOD_(IEvtSlot*, Clone)(THIS) SCONST OVERRIDE
	{
		return new LuaFunctionSlot(m_pLuaState,m_luaFun);
	}
	STDMETHOD_(BOOL,Equal)(THIS_ const IEvtSlot * sour) SCONST OVERRIDE
	{
		if(sour->GetSlotType()!=GetSlotType()) return false;
		const LuaFunctionSlot *psour=static_cast<const LuaFunctionSlot*>(sour);
		SASSERT(psour);
		return psour->m_luaFun==m_luaFun && psour->m_pLuaState==m_pLuaState;
	}

	STDMETHOD_(UINT,GetSlotType)(THIS) SCONST OVERRIDE
	{
		return SLOT_USER+1;
	}

private:
	SStringA m_luaFun;
	lua_State *m_pLuaState;
};

SNSEND
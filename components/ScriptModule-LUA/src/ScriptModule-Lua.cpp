﻿// luaScriptModule.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <event/SEventSlot.h>
#include "ScriptModule-Lua.h"
#include <lua_tinker.h>
#include <string/strcpcvt.h>
#include "exports/luaFunSlot.h"

extern BOOL SOUI_Export_Lua(lua_State *L);


SNSBEGIN
    wchar_t * cast_a2w(char * str)
    {
        return (wchar_t *)str;
    }

    int Utf8ToW(lua_State* L)
    {
        size_t n = 0;
        char* str = (char*)luaL_checklstring(L, -1, &n);
        if(!str)   return 0;
#ifdef WIN32
        SStringW strW=S_CA2W(str,CP_UTF8);
        lua_pushlstring(L, (const char*)(LPCWSTR)strW, sizeof(wchar_t)*(strW.GetLength()+1));
#else
        lua_pushlstring(L, (const char*)str, n);
#endif
        return 1;
    }

    TCHAR * cast_a2t(char * str)
    {
        return (TCHAR *)str;
    }


    int Utf8ToT(lua_State *L)
    {
        size_t n = 0;
        char* str = (char*)luaL_checklstring(L, -1, &n);
        if(!str)   return 0;
        SStringT strT=S_CA2T(str,CP_UTF8);
        lua_pushlstring(L, (const char*)(LPCTSTR)strT, (strT.GetLength()+1)*sizeof(TCHAR));
        return 1;
    }


    SScriptModule_Lua::SScriptModule_Lua()
    {
        d_state = luaL_newstate();
        if(d_state)
        {
            luaL_openlibs(d_state);
            SOUI_Export_Lua(d_state);
            lua_register(d_state, "A2W", Utf8ToW);
            lua_tinker::def(d_state, "cast_a2w", cast_a2w);
#ifdef WIN32
            luaL_dostring(d_state,"function L (str)\n return cast_a2w(A2W(str));\nend");//注册一个全局的"L"函数，用来将utf8编码的字符串转换为WCHAR
#else
            luaL_dostring(d_state,"function L (str)\n return str;\nend");//注册一个全局的"L"函数，用来将utf8编码的字符串转换为WCHAR
#endif

            lua_register(d_state, "A2T", Utf8ToT);
            lua_tinker::def(d_state, "cast_a2t", cast_a2t);
#ifdef WIN32
            luaL_dostring(d_state,"function T (str)\n return cast_a2t(A2T(str));\nend");//注册一个全局的"T"函数，用来将utf8编码的字符串转换为TCHAR
#else
            luaL_dostring(d_state,"function T (str)\n return str;\nend");//注册一个全局的"T"函数，用来将utf8编码的字符串转换为TCHAR
#endif
        }
    }


    SScriptModule_Lua::~SScriptModule_Lua()
    {
        if (d_state)
        {
            lua_close( d_state );
			d_state = NULL;
        }
    }


    LPCSTR SScriptModule_Lua::getIdentifierString() const
    {
        return "SOUI.Script.Lua5.4";
    }

    BOOL SScriptModule_Lua::executeScriptFile( LPCSTR pszScriptFile )
    {
        lua_tinker::dofile(d_state,pszScriptFile);
		return TRUE;
    }


    void SScriptModule_Lua::executeScriptBuffer( const char* buff, size_t sz )
    {
        lua_tinker::dobuffer(d_state,buff,sz);
    }

    BOOL SScriptModule_Lua::executeScriptedEventHandler( LPCSTR handler_name, IEvtArgs *pArg)
    {
        LuaFunctionSlot luaFunSlot(d_state,handler_name);
        BOOL bRet =  luaFunSlot.Run(pArg);
		if(bRet) pArg->IncreaseHandleCount();
		return bRet;
    }

	int SScriptModule_Lua::executeMain(THIS_ HINSTANCE hInst,LPCSTR pszWorkDir, LPCSTR pszArgs)
	{
		return lua_tinker::call<int>(d_state,"main",hInst,pszWorkDir, pszArgs);
	}


    HRESULT SIScriptFactory::CreateScriptModule( IScriptModule ** ppScriptModule )
    {
        *ppScriptModule= new SOUI::SScriptModule_Lua;
        return S_OK;
    }

	namespace SCRIPT_LUA
	{
		BOOL SCreateInstance(IObjRef ** ppScript)
		{
			*ppScript= new SOUI::SIScriptFactory;
			return TRUE;
		}
	}

SNSEND

EXTERN_C BOOL Script_Lua_SCreateInstance(IObjRef ** ppScript)
{
	return SOUI::SCRIPT_LUA::SCreateInstance(ppScript);
}

//////////////////////////////////////////////////////////////////////////

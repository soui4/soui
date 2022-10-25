#include <string/tstring.h>

BOOL ExpLua_String(lua_State *L)
{
	try{

		lua_tinker::class_add<SStringA>(L,"SStringA");
		lua_tinker::class_inh<SStringA,IStringA>(L);
        lua_tinker::class_con<SStringA>(L,lua_tinker::constructor<SStringA>);
        lua_tinker::class_con<SStringA>(L,lua_tinker::constructor<SStringA,const SStringA &>);
        lua_tinker::class_con<SStringA>(L,lua_tinker::constructor<SStringA,const char*>);
        
        lua_tinker::class_def<SStringA>(L,"Mid",(SStringA (SStringA::*)(int) const)&SStringA::Mid);
        lua_tinker::class_def<SStringA>(L,"Mid2",(SStringA (SStringA::*)(int,int) const)&SStringA::Mid);
        lua_tinker::class_def<SStringA>(L,"Left",&SStringA::Left);
        lua_tinker::class_def<SStringA>(L,"Right",&SStringA::Right);


        lua_tinker::class_add<SStringW>(L,"SStringW");
		lua_tinker::class_inh<SStringW,IStringW>(L);
        lua_tinker::class_con<SStringW>(L,lua_tinker::constructor<SStringW>);
        lua_tinker::class_con<SStringW>(L,lua_tinker::constructor<SStringW,const SStringW &>);
        lua_tinker::class_con<SStringW>(L,lua_tinker::constructor<SStringW,const wchar_t*>);

        lua_tinker::class_def<SStringW>(L,"Mid",(SStringW (SStringW::*)(int) const)&SStringW::Mid);
        lua_tinker::class_def<SStringW>(L,"Mid2",(SStringW (SStringW::*)(int,int) const)&SStringW::Mid);
        lua_tinker::class_def<SStringW>(L,"Left",&SStringW::Left);
        lua_tinker::class_def<SStringW>(L,"Right",&SStringW::Right);
		return TRUE;
	}catch(...)
	{
		return FALSE;
	}

}
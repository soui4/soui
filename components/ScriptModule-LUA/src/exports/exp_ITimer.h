#include <SApp.h>


BOOL ExpLua_ITimer(lua_State *L)
{
	try{
		lua_tinker::class_add<ITimer>(L,"ITimer");
		lua_tinker::class_inh<ITimer,IObjRef>(L);
		lua_tinker::class_def<ITimer>(L,"StartTimer",&ITimer::StartTimer);
		lua_tinker::class_def<ITimer>(L,"KillTimer",&ITimer::KillTimer);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
#include <interface/sinterpolator-i.h>


BOOL ExpLua_IInterpolator(lua_State *L)
{
	try{
		lua_tinker::class_add<IInterpolator>(L,"IInterpolator");
		lua_tinker::class_inh<IInterpolator,IObject>(L);
		lua_tinker::class_def<IInterpolator>(L,"getInterpolation",&IInterpolator::getInterpolation);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}

}
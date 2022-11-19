#include <interface/SScriptModule-i.h>

BOOL ExpLua_IScriptModule(lua_State *L)
{
	try
	{
		lua_tinker::class_add<IScriptModule>(L,"IScriptModule");
		lua_tinker::class_inh<IScriptModule,IObjRef>(L);
		lua_tinker::class_def<IScriptModule>(L,"GetScriptEngine",&IScriptModule::GetScriptEngine);
		lua_tinker::class_def<IScriptModule>(L,"executeScriptFile",&IScriptModule::executeScriptFile);
		lua_tinker::class_def<IScriptModule>(L,"executeScriptBuffer",&IScriptModule::executeScriptBuffer);
		lua_tinker::class_def<IScriptModule>(L,"executeScriptedEventHandler",&IScriptModule::executeScriptedEventHandler);
		lua_tinker::class_def<IScriptModule>(L,"getIdentifierString",&IScriptModule::getIdentifierString);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
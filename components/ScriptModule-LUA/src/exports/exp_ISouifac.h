#include <interface/SFactory-i.h>


BOOL ExpLua_ISouiFactory(lua_State *L)
{
	try{
		lua_tinker::class_add<ISouiFactory>(L,"ISouiFactory");
		lua_tinker::class_inh<ISouiFactory,IObjRef>(L);
		lua_tinker::class_def<ISouiFactory>(L,"CreateApp",&ISouiFactory::CreateApp);
		lua_tinker::class_def<ISouiFactory>(L,"CreateNativeWnd",&ISouiFactory::CreateNativeWnd);
		lua_tinker::class_def<ISouiFactory>(L,"CreateHostWnd",&ISouiFactory::CreateHostWnd);
		lua_tinker::class_def<ISouiFactory>(L,"CreateHostDialog",&ISouiFactory::CreateHostDialog);
		lua_tinker::class_def<ISouiFactory>(L,"CreateStringA",&ISouiFactory::CreateStringA);
		lua_tinker::class_def<ISouiFactory>(L,"CreateStringW",&ISouiFactory::CreateStringW);
		lua_tinker::class_def<ISouiFactory>(L,"CreateXmlDoc",&ISouiFactory::CreateXmlDoc);
		lua_tinker::class_def<ISouiFactory>(L,"CreateResProvider",&ISouiFactory::CreateResProvider);
		lua_tinker::class_def<ISouiFactory>(L,"CreateFuncSlot",&ISouiFactory::CreateFuncSlot);
		lua_tinker::class_def<ISouiFactory>(L,"CreateMenu",&ISouiFactory::CreateMenu);
		lua_tinker::class_def<ISouiFactory>(L,"CreateMenuEx",&ISouiFactory::CreateMenuEx);
		lua_tinker::class_def<ISouiFactory>(L,"CreateTimer",&ISouiFactory::CreateTimer);


		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
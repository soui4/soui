#include <interface/obj-ref-i.h>



BOOL ExpLua_IObjRef(lua_State *L)
{
	try{
		lua_tinker::class_add<IObjRef>(L,"IObjRef");
		lua_tinker::class_def<IObjRef>(L,"AddRef",&IObjRef::AddRef);
		lua_tinker::class_def<IObjRef>(L,"Release",&IObjRef::Release);
		lua_tinker::class_def<IObjRef>(L,"OnFinalRelease",&IObjRef::OnFinalRelease);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
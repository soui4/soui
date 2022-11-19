#include <interface/sobject-i.h>


BOOL ExpLua_IObject(lua_State *L)
{
	try{
		lua_tinker::class_add<IObject>(L,"IObject");
		lua_tinker::class_inh<IObject,IObjRef>(L);
		lua_tinker::class_def<IObject>(L,"IsClass",&IObject::IsClass);
        lua_tinker::class_def<IObject>(L,"GetObjectClass",&IObject::GetObjectClass);
        lua_tinker::class_def<IObject>(L,"GetObjectType",&IObject::GetObjectType);

		lua_tinker::class_def<IObject>(L,"GetID",&IObject::GetID);
		lua_tinker::class_def<IObject>(L,"SetID",&IObject::SetID);
		lua_tinker::class_def<IObject>(L,"GetName",&IObject::GetName);
		lua_tinker::class_def<IObject>(L,"GetNameA",&IObject::GetNameA);
		lua_tinker::class_def<IObject>(L,"SetName",&IObject::SetName);

		lua_tinker::class_def<IObject>(L,"InitFromXml",&IObject::InitFromXml);
		lua_tinker::class_def<IObject>(L,"OnInitFinished",&IObject::OnInitFinished);
		lua_tinker::class_def<IObject>(L,"ISetAttribute",&IObject::ISetAttribute);
		lua_tinker::class_def<IObject>(L,"SetAttributeA",&IObject::SetAttributeA);
		lua_tinker::class_def<IObject>(L,"SetAttribute",&IObject::SetAttribute);
		lua_tinker::class_def<IObject>(L,"GetAttribute",&IObject::GetAttribute);
		lua_tinker::class_def<IObject>(L,"AfterAttribute",&IObject::AfterAttribute);
		lua_tinker::class_def<IObject>(L,"SetAttrHandler",&IObject::SetAttrHandler);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
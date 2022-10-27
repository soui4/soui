#include <interface/smenu-i.h>


BOOL ExpLua_IMenuEx(lua_State *L)
{
	try{
		lua_tinker::class_add<IMenuEx>(L,"IMenuEx");
		lua_tinker::class_inh<IMenuEx,IObjRef>(L);

		lua_tinker::class_def<IMenuEx>(L,"LoadMenu",&IMenuEx::LoadMenu);
		lua_tinker::class_def<IMenuEx>(L,"LoadMenu2",&IMenuEx::LoadMenu2);
		lua_tinker::class_def<IMenuEx>(L,"InsertMenu",&IMenuEx::InsertMenu);
		lua_tinker::class_def<IMenuEx>(L,"AppendMenu",&IMenuEx::AppendMenu);
		lua_tinker::class_def<IMenuEx>(L,"CheckMenuItem",&IMenuEx::CheckMenuItem);
		lua_tinker::class_def<IMenuEx>(L,"CheckMenuRadioItem",&IMenuEx::CheckMenuRadioItem);
		lua_tinker::class_def<IMenuEx>(L,"DeleteMenu",&IMenuEx::DeleteMenu);
		lua_tinker::class_def<IMenuEx>(L,"TrackPopupMenu",&IMenuEx::TrackPopupMenu);
		lua_tinker::class_def<IMenuEx>(L,"DestroyMenu",&IMenuEx::DestroyMenu);
		lua_tinker::class_def<IMenuEx>(L,"ModifyMenuString",&IMenuEx::ModifyMenuString);
		lua_tinker::class_def<IMenuEx>(L,"SetMenuUserData",&IMenuEx::SetMenuUserData);
		lua_tinker::class_def<IMenuEx>(L,"GetMenuUserData",&IMenuEx::GetMenuUserData);
		lua_tinker::class_def<IMenuEx>(L,"GetContextHelpId",&IMenuEx::GetContextHelpId);
		lua_tinker::class_def<IMenuEx>(L,"SetContextHelpId",&IMenuEx::SetContextHelpId);
		lua_tinker::class_def<IMenuEx>(L,"GetSubMenu",&IMenuEx::GetSubMenu);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
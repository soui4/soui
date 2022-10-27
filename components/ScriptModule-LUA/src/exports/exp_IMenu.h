#include <interface/smenu-i.h>


BOOL ExpLua_IMenu(lua_State *L)
{
	try{
		lua_tinker::class_add<IMenu>(L,"IMenu");
		lua_tinker::class_inh<IMenu,IObjRef>(L);

		lua_tinker::class_def<IMenu>(L,"Attach",&IMenu::Attach);
		lua_tinker::class_def<IMenu>(L,"Detach",&IMenu::Detach);
		lua_tinker::class_def<IMenu>(L,"GetHMenu",&IMenu::GetHMenu);
		lua_tinker::class_def<IMenu>(L,"LoadMenu",&IMenu::LoadMenu);
		lua_tinker::class_def<IMenu>(L,"LoadMenu2",&IMenu::LoadMenu2);
		lua_tinker::class_def<IMenu>(L,"SetIconSkin",&IMenu::SetIconSkin);
		lua_tinker::class_def<IMenu>(L,"InsertMenu",&IMenu::InsertMenu);
		lua_tinker::class_def<IMenu>(L,"AppendMenu",&IMenu::AppendMenu);
		lua_tinker::class_def<IMenu>(L,"CheckMenuItem",&IMenu::CheckMenuItem);
		lua_tinker::class_def<IMenu>(L,"CheckMenuRadioItem",&IMenu::CheckMenuRadioItem);
		lua_tinker::class_def<IMenu>(L,"DeleteMenu",&IMenu::DeleteMenu);
		lua_tinker::class_def<IMenu>(L,"TrackPopupMenu",&IMenu::TrackPopupMenu);
		lua_tinker::class_def<IMenu>(L,"DestroyMenu",&IMenu::DestroyMenu);
		lua_tinker::class_def<IMenu>(L,"ModifyMenuString",&IMenu::ModifyMenuString);
		lua_tinker::class_def<IMenu>(L,"SetMenuUserData",&IMenu::SetMenuUserData);
		lua_tinker::class_def<IMenu>(L,"GetMenuUserData",&IMenu::GetMenuUserData);
		lua_tinker::class_def<IMenu>(L,"GetContextHelpId",&IMenu::GetContextHelpId);
		lua_tinker::class_def<IMenu>(L,"SetContextHelpId",&IMenu::SetContextHelpId);
		lua_tinker::class_def<IMenu>(L,"GetSubMenu",&IMenu::GetSubMenu);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
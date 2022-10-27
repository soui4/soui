BOOL ExpLua_SysApi(lua_State *L)
{
	try{

		lua_tinker::def(L,"PostQuitMessage",PostQuitMessage);
		lua_tinker::def(L,"GetActiveWindow",GetActiveWindow);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}

}
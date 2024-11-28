#include <core/SWnd.h>
#include "toobj.h"

BOOL ExpLua_Window(lua_State *L)
{
	try{
		lua_tinker::class_add<SWindow>(L,"SWindow");
        lua_tinker::class_inh<SWindow,IWindow>(L);

		lua_tinker::class_def<SWindow>(L,"FindChildByNameA",(SWindow* (SWindow::*)(LPCSTR,int))&SWindow::FindChildByName);
		lua_tinker::class_def<SWindow>(L,"FindChildByNameW",(SWindow* (SWindow::*)(LPCWSTR,int ))&SWindow::FindChildByName);
		lua_tinker::class_def<SWindow>(L,"FindChildByID",(SWindow* (SWindow::*)(int,int))&SWindow::FindChildByID);
		lua_tinker::class_def<SWindow>(L,"GetWindowRect2",(CRect (SWindow::*)() const)&SWindow::GetWindowRect);
		lua_tinker::class_def<SWindow>(L,"GetClientRect2",(CRect (SWindow::*)()const)&SWindow::GetClientRect);
		lua_tinker::class_def<SWindow>(L,"GetWindow",(SWindow *(SWindow::*)(int))&SWindow::GetWindow);
		DEF_TOOBJ(L,SWindow);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
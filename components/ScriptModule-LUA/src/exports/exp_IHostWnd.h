#include <interface/shostwnd-i.h>
#include "toobj.h"

BOOL ExpLua_IHostWnd(lua_State *L)
{
	try{
		lua_tinker::class_add<IHostWnd>(L,"IHostWnd");
		lua_tinker::class_inh<IHostWnd,INativeWnd>(L);
		lua_tinker::class_def<IHostWnd>(L,"CreateEx",&IHostWnd::CreateEx);
		lua_tinker::class_def<IHostWnd>(L,"Create",&IHostWnd::Create);
		lua_tinker::class_def<IHostWnd>(L,"SetLayoutId",&IHostWnd::SetLayoutId);
		lua_tinker::class_def<IHostWnd>(L,"GetIRoot",&IHostWnd::GetIRoot);
		lua_tinker::class_def<IHostWnd>(L,"FindIChildByID",&IHostWnd::FindIChildByID);
		lua_tinker::class_def<IHostWnd>(L,"FindIChildByName",&IHostWnd::FindIChildByName);
		lua_tinker::class_def<IHostWnd>(L,"FindIChildByNameA",&IHostWnd::FindIChildByNameA);
		lua_tinker::class_def<IHostWnd>(L,"GetNcPainter",&IHostWnd::GetNcPainter);
		lua_tinker::class_def<IHostWnd>(L,"SetEventHandler",&IHostWnd::SetEventHandler);
		lua_tinker::class_def<IHostWnd>(L,"GetEventHandler",&IHostWnd::GetEventHandler);
		lua_tinker::class_def<IHostWnd>(L,"AnimateHostWindow",&IHostWnd::AnimateHostWindow);
        lua_tinker::class_def<IHostWnd>(L,"GetRoot",&IHostWnd::GetIRoot);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}

BOOL ExpLua_IHostDialog(lua_State *L)
{
	try{
		lua_tinker::class_add<IHostDialog>(L,"IHostDialog");
		lua_tinker::class_inh<IHostDialog,IHostWnd>(L);
		lua_tinker::class_def<IHostDialog>(L,"DoModal",&IHostDialog::DoModal);
		lua_tinker::class_def<IHostDialog>(L,"EndDialog",&IHostDialog::EndDialog);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
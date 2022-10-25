#include <interface/SEvtArgs-i.h>

BOOL ExpLua_IEvtArgs(lua_State *L)
{
	try{
		lua_tinker::class_add<IEvtArgs>(L,"IEvtArgs");
		lua_tinker::class_inh<IEvtArgs,IObject>(L);
		lua_tinker::class_def<IEvtArgs>(L,"Sender",&IEvtArgs::Sender);
		lua_tinker::class_def<IEvtArgs>(L,"IdFrom",&IEvtArgs::IdFrom);
		lua_tinker::class_def<IEvtArgs>(L,"SetIdFrom",&IEvtArgs::SetIdFrom);
		lua_tinker::class_def<IEvtArgs>(L,"NameFrom",&IEvtArgs::NameFrom);
		lua_tinker::class_def<IEvtArgs>(L,"SetNameFrom",&IEvtArgs::SetNameFrom);
		lua_tinker::class_def<IEvtArgs>(L,"IsBubbleUp",&IEvtArgs::IsBubbleUp);
		lua_tinker::class_def<IEvtArgs>(L,"SetBubbleUp",&IEvtArgs::SetBubbleUp);
		lua_tinker::class_def<IEvtArgs>(L,"HandleCount",&IEvtArgs::HandleCount);
		lua_tinker::class_def<IEvtArgs>(L,"IncreaseHandleCount",&IEvtArgs::IncreaseHandleCount);
		lua_tinker::class_def<IEvtArgs>(L,"Data",&IEvtArgs::Data);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
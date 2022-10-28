#include "luaAdapter.h"
#include "toobj.h"

BOOL ExpLua_IAdapter(lua_State *L)
{
	try{
		lua_tinker::class_add<ILvAdapter>(L,"ILvAdapter");
		lua_tinker::class_inh<ILvAdapter,IObjRef>(L);
		lua_tinker::class_def<ILvAdapter>(L,"registerDataSetObserver",&ILvAdapter::registerDataSetObserver);
		lua_tinker::class_def<ILvAdapter>(L,"unregisterDataSetObserver",&ILvAdapter::unregisterDataSetObserver);
		lua_tinker::class_def<ILvAdapter>(L,"getCount",&ILvAdapter::getCount);
		lua_tinker::class_def<ILvAdapter>(L,"getView",&ILvAdapter::getView);
		lua_tinker::class_def<ILvAdapter>(L,"getItemViewType",&ILvAdapter::getItemViewType);
		lua_tinker::class_def<ILvAdapter>(L,"getViewTypeCount",&ILvAdapter::getViewTypeCount);
		lua_tinker::class_def<ILvAdapter>(L,"getViewDesiredSize",&ILvAdapter::getViewDesiredSize);
		lua_tinker::class_def<ILvAdapter>(L,"isEmpty",&ILvAdapter::isEmpty);
		lua_tinker::class_def<ILvAdapter>(L,"getItemDesc",&ILvAdapter::getItemDesc);
		lua_tinker::class_def<ILvAdapter>(L,"getItemDesc",&ILvAdapter::getItemDesc);
		lua_tinker::class_def<ILvAdapter>(L,"InitByTemplate",&ILvAdapter::InitByTemplate);
		lua_tinker::class_def<ILvAdapter>(L,"QueryInterface",&ILvAdapter::QueryInterface);
		lua_tinker::class_def<ILvAdapter>(L,"registerDataSetObserver",&ILvAdapter::registerDataSetObserver);
		lua_tinker::class_def<ILvAdapter>(L,"registerDataSetObserver",&ILvAdapter::registerDataSetObserver);

		lua_tinker::class_add<LuaLvAdapter>(L,"LuaLvAdapter");
		lua_tinker::class_inh<LuaLvAdapter,ILvAdapter>(L);
		lua_tinker::class_def<LuaLvAdapter>(L,"notifyDataSetChanged",&LuaLvAdapter::notifyDataSetChanged);
		lua_tinker::class_def<LuaLvAdapter>(L,"notifyDataSetInvalidated",&LuaLvAdapter::notifyDataSetInvalidated);
		lua_tinker::class_def<LuaLvAdapter>(L,"notifyItemDataChanged",&LuaLvAdapter::notifyItemDataChanged);
		lua_tinker::class_def<LuaLvAdapter>(L,"initCallback",&LuaLvAdapter::initCallback);
		lua_tinker::class_def<LuaLvAdapter>(L,"getContext",&LuaLvAdapter::getContext);
		DEF_CAST_OBJREF(L,LuaLvAdapter);

		lua_tinker::class_add<IMcAdapter>(L,"IMcAdapter");
		lua_tinker::class_inh<IMcAdapter,IObjRef>(L);
		lua_tinker::class_def<IMcAdapter>(L,"registerDataSetObserver",&IMcAdapter::registerDataSetObserver);
		lua_tinker::class_def<IMcAdapter>(L,"unregisterDataSetObserver",&IMcAdapter::unregisterDataSetObserver);
		lua_tinker::class_def<IMcAdapter>(L,"getCount",&IMcAdapter::getCount);
		lua_tinker::class_def<IMcAdapter>(L,"getView",&IMcAdapter::getView);
		lua_tinker::class_def<IMcAdapter>(L,"getItemViewType",&IMcAdapter::getItemViewType);
		lua_tinker::class_def<IMcAdapter>(L,"getViewTypeCount",&IMcAdapter::getViewTypeCount);
		lua_tinker::class_def<IMcAdapter>(L,"getViewDesiredSize",&IMcAdapter::getViewDesiredSize);
		lua_tinker::class_def<IMcAdapter>(L,"isEmpty",&IMcAdapter::isEmpty);
		lua_tinker::class_def<IMcAdapter>(L,"getItemDesc",&IMcAdapter::getItemDesc);
		lua_tinker::class_def<IMcAdapter>(L,"getItemDesc",&IMcAdapter::getItemDesc);
		lua_tinker::class_def<IMcAdapter>(L,"InitByTemplate",&IMcAdapter::InitByTemplate);
		lua_tinker::class_def<IMcAdapter>(L,"QueryInterface",&IMcAdapter::QueryInterface);
		lua_tinker::class_def<IMcAdapter>(L,"GetColumnName",&IMcAdapter::GetColumnName);
		lua_tinker::class_def<IMcAdapter>(L,"IsColumnVisible",&IMcAdapter::IsColumnVisible);
		lua_tinker::class_def<IMcAdapter>(L,"OnSort",&IMcAdapter::OnSort);

		lua_tinker::class_add<LuaMcAdapter>(L,"LuaMcAdapter");
		lua_tinker::class_inh<LuaMcAdapter,IMcAdapter>(L);
		lua_tinker::class_def<LuaMcAdapter>(L,"notifyDataSetChanged",&LuaMcAdapter::notifyDataSetChanged);
		lua_tinker::class_def<LuaMcAdapter>(L,"notifyDataSetInvalidated",&LuaMcAdapter::notifyDataSetInvalidated);
		lua_tinker::class_def<LuaMcAdapter>(L,"notifyItemDataChanged",&LuaMcAdapter::notifyItemDataChanged);
		lua_tinker::class_def<LuaMcAdapter>(L,"initCallback",&LuaMcAdapter::initCallback);
		lua_tinker::class_def<LuaMcAdapter>(L,"getContext",&LuaMcAdapter::getContext);
		DEF_CAST_OBJREF(L,LuaMcAdapter);

		lua_tinker::class_add<ITvAdapter>(L,"ITvAdapter");
		lua_tinker::class_inh<ITvAdapter,IObjRef>(L);
		lua_tinker::class_def<ITvAdapter>(L,"registerDataSetObserver",&ITvAdapter::registerDataSetObserver);
		lua_tinker::class_def<ITvAdapter>(L,"unregisterDataSetObserver",&ITvAdapter::unregisterDataSetObserver);
		lua_tinker::class_def<ITvAdapter>(L,"GetItemDataByIndex",&ITvAdapter::GetItemDataByIndex);
		lua_tinker::class_def<ITvAdapter>(L,"SetItemDataByIndex",&ITvAdapter::SetItemDataByIndex);
		lua_tinker::class_def<ITvAdapter>(L,"GetParentItem",&ITvAdapter::GetParentItem);
		lua_tinker::class_def<ITvAdapter>(L,"GetFirstChildItem",&ITvAdapter::GetFirstChildItem);
		lua_tinker::class_def<ITvAdapter>(L,"GetLastChildItem",&ITvAdapter::GetLastChildItem);
		lua_tinker::class_def<ITvAdapter>(L,"GetPrevSiblingItem",&ITvAdapter::GetPrevSiblingItem);
		lua_tinker::class_def<ITvAdapter>(L,"GetNextSiblingItem",&ITvAdapter::GetNextSiblingItem);
		lua_tinker::class_def<ITvAdapter>(L,"HasChildren",&ITvAdapter::HasChildren);
		lua_tinker::class_def<ITvAdapter>(L,"IsItemVisible",&ITvAdapter::IsItemVisible);
		lua_tinker::class_def<ITvAdapter>(L,"GetFirstVisibleItem",&ITvAdapter::GetFirstVisibleItem);
		lua_tinker::class_def<ITvAdapter>(L,"GetLastVisibleItem",&ITvAdapter::GetLastVisibleItem);
		lua_tinker::class_def<ITvAdapter>(L,"GetPrevVisibleItem",&ITvAdapter::GetPrevVisibleItem);
		lua_tinker::class_def<ITvAdapter>(L,"GetNextVisibleItem",&ITvAdapter::GetNextVisibleItem);
		lua_tinker::class_def<ITvAdapter>(L,"ExpandItem",&ITvAdapter::ExpandItem);
		lua_tinker::class_def<ITvAdapter>(L,"IsItemExpanded",&ITvAdapter::IsItemExpanded);
		lua_tinker::class_def<ITvAdapter>(L,"SetItemExpanded",&ITvAdapter::SetItemExpanded);
		lua_tinker::class_def<ITvAdapter>(L,"getView",&ITvAdapter::getView);
		lua_tinker::class_def<ITvAdapter>(L,"getViewType",&ITvAdapter::getViewType);
		lua_tinker::class_def<ITvAdapter>(L,"getViewTypeCount",&ITvAdapter::getViewTypeCount);
		lua_tinker::class_def<ITvAdapter>(L,"getViewDesiredSize",&ITvAdapter::getViewDesiredSize);
		lua_tinker::class_def<ITvAdapter>(L,"isViewWidthMatchParent",&ITvAdapter::isViewWidthMatchParent);
		lua_tinker::class_def<ITvAdapter>(L,"InitByTemplate",&ITvAdapter::InitByTemplate);
		lua_tinker::class_def<ITvAdapter>(L,"QueryInterface",&ITvAdapter::QueryInterface);

		lua_tinker::class_add<LuaTvAdapter>(L,"LuaTvAdapter");
		lua_tinker::class_inh<LuaTvAdapter,ITvAdapter>(L);
		lua_tinker::class_def<LuaTvAdapter>(L,"notifyBranchChanged",&LuaTvAdapter::notifyBranchChanged);
		lua_tinker::class_def<LuaTvAdapter>(L,"notifyBranchInvalidated",&LuaTvAdapter::notifyBranchInvalidated);
		lua_tinker::class_def<LuaTvAdapter>(L,"notifyBranchExpandChanged",&LuaTvAdapter::notifyBranchExpandChanged);
		lua_tinker::class_def<LuaTvAdapter>(L,"initCallback",&LuaTvAdapter::initCallback);
		lua_tinker::class_def<LuaTvAdapter>(L,"getContext",&LuaTvAdapter::getContext);
		DEF_CAST_OBJREF(L,LuaTvAdapter);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
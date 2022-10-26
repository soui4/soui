#pragma once
#include <helper/SAdapterBase.h>

SNSBEGIN
class LuaLvAdapter : public SAdapterBase
{
public:
	LuaLvAdapter(lua_State *luaState,void * ctx):m_luaState(luaState),m_ctx(ctx){}

	enum{
		fid_getView=0,
		fid_getCount,
		fid_initByTemplate,
		fid_getViewTypeCount,
		fid_getItemViewType,
	};

	void initCallback(int fid, LPCSTR pszCbName){
		switch(fid){
case fid_getView: m_luaGetView = pszCbName;break;
case fid_getCount:m_luaGetCount = pszCbName;break;
case fid_initByTemplate:m_luaInitByTemplate=pszCbName;break;
case fid_getViewTypeCount:m_luaGetViewTypeCount=pszCbName;break;
case fid_getItemViewType:m_luaGetItemViewType=pszCbName;break;
default:lua_tinker::print_error(m_luaState,"unknown fid: %d",fid);break;
		}
	}
public:
	STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE{
		if(m_luaGetView.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaGetView,m_ctx,position,pItem,(IXmlNode*)&xmlTemplate);
	}

	STDMETHOD_(int, getCount)(THIS) OVERRIDE{
		if(m_luaGetCount.IsEmpty())
			return 0;
		return lua_tinker::call<int>(m_luaState,m_luaGetCount,m_ctx);
	}

	STDMETHOD_(void, InitByTemplate)(IXmlNode *pXmlTemplate) OVERRIDE
	{
		if(m_luaInitByTemplate.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaInitByTemplate,m_ctx,pXmlTemplate);
	}

	STDMETHOD_(int, getViewTypeCount)() OVERRIDE
	{
		if(m_luaGetViewTypeCount.IsEmpty())
			return 1;
		return lua_tinker::call<int>(m_luaState,m_luaGetViewTypeCount,m_ctx);
	}

	STDMETHOD_(int, getItemViewType)(int position)
	{
		if(m_luaGetItemViewType.IsEmpty())
			return 0;
		return lua_tinker::call<int>(m_luaState,m_luaGetItemViewType,m_ctx,position);
	}


private:
	lua_State * m_luaState;
	void * m_ctx;

	SStringA m_luaGetView;
	SStringA m_luaGetItemViewType;
	SStringA m_luaGetViewTypeCount;
	SStringA m_luaGetCount;
	SStringA m_luaInitByTemplate;
};

class LuaMcAdapter : public SMcAdapterBase
{
public:
	LuaMcAdapter(lua_State *luaState,void * ctx):m_luaState(luaState),m_ctx(ctx){}

	enum{
		fid_getView=0,
		fid_getCount,
		fid_initByTemplate,
		fid_getViewTypeCount,
		fid_getItemViewType,
		fid_onSort,
		fid_getColumnName,
		fid_IsColumnVisible,
	};

	void initCallback(int fid, LPCSTR pszCbName){
		switch(fid){
case fid_getView: m_luaGetView = pszCbName;break;
case fid_getCount:m_luaGetCount = pszCbName;break;
case fid_initByTemplate:m_luaInitByTemplate=pszCbName;break;
case fid_getViewTypeCount:m_luaGetViewTypeCount=pszCbName;break;
case fid_getItemViewType:m_luaGetItemViewType=pszCbName;break;
case fid_onSort:m_luaOnSort = pszCbName;break;
case fid_getColumnName:m_luaGetColumnName=pszCbName;break;
case fid_IsColumnVisible:m_luaIsColumnVisible=pszCbName;break;
default:lua_tinker::print_error(m_luaState,"unknown fid: %d",fid);break;
		}
	}
public:
	STDMETHOD_(void, getView)(int position, SItemPanel *pItem, SXmlNode xmlTemplate) OVERRIDE{
		if(m_luaGetView.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaGetView,m_ctx,position,pItem,(IXmlNode*)&xmlTemplate);
	}

	STDMETHOD_(int, getCount)(THIS) OVERRIDE{
		if(m_luaGetCount.IsEmpty())
			return 0;
		return lua_tinker::call<int>(m_luaState,m_luaGetCount,m_ctx);
	}

	STDMETHOD_(void, InitByTemplate)(IXmlNode *pXmlTemplate) OVERRIDE
	{
		if(m_luaInitByTemplate.IsEmpty())
			return;
		lua_tinker::call<void>(m_luaState,m_luaInitByTemplate,m_ctx,pXmlTemplate);
	}

	STDMETHOD_(int, getViewTypeCount)() OVERRIDE
	{
		if(m_luaGetViewTypeCount.IsEmpty())
			return 1;
		return lua_tinker::call<int>(m_luaState,m_luaGetViewTypeCount,m_ctx);
	}

	STDMETHOD_(int, getItemViewType)(int position)
	{
		if(m_luaGetItemViewType.IsEmpty())
			return 0;
		return lua_tinker::call<int>(m_luaState,m_luaGetItemViewType,m_ctx,position);
	}

	STDMETHOD_(BOOL, OnSort)(int iCol, UINT *pFmts, int nCols) OVERRIDE
	{
		if(m_luaOnSort.IsEmpty())
			return FALSE;
		return lua_tinker::call<int>(m_luaState,m_luaOnSort,m_ctx,iCol,pFmts,nCols) != 0;;
	}

	STDMETHOD_(SStringW, GetColumnName)(int iCol) SCONST
	{
		if(m_luaGetColumnName.IsEmpty())
			return L"";
		const char * pszName = lua_tinker::call<const char*>(m_luaState,m_luaGetColumnName,m_ctx,iCol);
		return S_CA2W(pszName);
	}

	STDMETHOD_(BOOL, IsColumnVisible)(THIS_ int iCol) SCONST OVERRIDE
	{
		if(m_luaIsColumnVisible.IsEmpty())
			return TRUE;
		return lua_tinker::call<int>(m_luaState,m_luaIsColumnVisible,m_ctx,iCol) != 0;;
	}

private:
	lua_State * m_luaState;
	void * m_ctx;

	SStringA m_luaGetView;
	SStringA m_luaGetItemViewType;
	SStringA m_luaGetViewTypeCount;
	SStringA m_luaGetCount;
	SStringA m_luaInitByTemplate;
	SStringA m_luaOnSort;
	SStringA m_luaGetColumnName;
	SStringA m_luaIsColumnVisible;
};
SNSEND
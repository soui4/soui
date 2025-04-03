#include "stdafx.h"
#include "SysdataMgr.h"
#include "CDebug.h"
#include <vector>
#include <algorithm>
#include "helpapi.h"
#include "Global.h"
#include <algorithm>

template<>
CSysDataMgr * SSingleton<CSysDataMgr>::ms_Singleton = NULL;

CSysDataMgr::CSysDataMgr()
{
}


CSysDataMgr::~CSysDataMgr()
{
}

bool CSysDataMgr::LoadSysData(LPCTSTR cfgDir)
{
	m_strConfigDir = cfgDir;
	InitCtrlDef();
	InitSkinProp();
	return true;
}

SStringA CSysDataMgr::GetCtrlAutos()
{
	SStringW strAuto;
	pugi::xml_node xmlCtrl = getCtrlDefNode().child(L"controls").first_child();
	while(xmlCtrl)
	{
		strAuto += xmlCtrl.name();
		strAuto += L' ';
		xmlCtrl = xmlCtrl.next_sibling();
	}
	return S_CW2A(strAuto, CP_UTF8);
}

SStringA CSysDataMgr::GetCtrlAttrAutos(SStringW ctrlname)
{
	std::set<SStringW> attrs;
	_GetCtrlAttrs(ctrlname,attrs);
	SStringW strAuto;
	std::set<SStringW>::iterator it = attrs.begin();
	while(it!=attrs.end())
	{
		strAuto += *it + L" ";
		it++;
	}
	strAuto.TrimRight();
	return S_CW2A(strAuto,CP_UTF8);
}

void CSysDataMgr::InitCtrlDef()
{
	pugi::xml_parse_result result = m_xmlCtrlDef.load_file(m_strConfigDir + _T("/Ctrl.xml"));
}

pugi::xml_node CSysDataMgr::getCtrlDefNode()
{
	return m_xmlCtrlDef.child(L"root");
}

void CSysDataMgr::InitSkinProp()
{
	m_xmlSkinProp.load_file(m_strConfigDir + _T("/Skin.xml"));
}

pugi::xml_node CSysDataMgr::getSkinDefNode()
{
	return m_xmlSkinProp.root().child(L"root");
}

SOUI::SStringT CSysDataMgr::GetConfigDir() const
{
	return m_strConfigDir;
}

SOUI::SStringA CSysDataMgr::GetSkinAutos()
{
	std::set<SStringW> skins;
	pugi::xml_node xmlSkin = getSkinDefNode().child(L"skins").first_child();
	while(xmlSkin)
	{
		if(xmlSkin.attribute(L"visible").as_bool(true))
		{
			skins.insert(xmlSkin.name());
		}
		xmlSkin = xmlSkin.next_sibling();
	}
	SStringW strAuto;
	std::set<SStringW>::iterator it = skins.begin();
	while(it!=skins.end())
	{
		strAuto += *it + L" ";
		it++;
	}
	strAuto.TrimRight();
	return S_CW2A(strAuto,CP_UTF8);
}

SOUI::SStringA CSysDataMgr::GetSkinAttrAutos(SStringW skinName)
{
	std::set<SStringW> attrs;
	_GetSkinAttrs(skinName,attrs);
	SStringW strAuto;
	std::set<SStringW>::iterator it = attrs.begin();
	while(it!=attrs.end())
	{
		strAuto += *it + L" ";
		it++;
	}
	strAuto.TrimRight();
	return S_CW2A(strAuto,CP_UTF8);
}

void CSysDataMgr::_GetSkinAttrs(SStringW skinName,std::set<SStringW> &attrs)
{
	pugi::xml_node xmlSkin = getSkinDefNode().child(L"skins").child(skinName);
	if(xmlSkin)
	{
		pugi::xml_node skinAttr = xmlSkin.child(L"groups").child(L"propgroup").first_child();
		while(skinAttr)
		{
			if(skinAttr.attribute(L"name"))
			{
				attrs.insert(skinAttr.attribute(L"name").as_string());
			}
			skinAttr = skinAttr.next_sibling();
		}
		if(xmlSkin.attribute(L"parent"))
		{
			SStringW strParent = xmlSkin.attribute(L"parent").as_string();
			if(!strParent.IsEmpty())
			{
				SStringWList parents;
				SplitString(strParent,L'|',parents);
				for(int i=0;i<parents.GetCount();i++)
				{
					_GetSkinAttrs(parents[i],attrs);
				}
			}
		}
	}
}

void CSysDataMgr::_GetCtrlAttrs(SStringW ctrlName,std::set<SStringW> &attrs)
{
	pugi::xml_node xmlCtrl = getCtrlDefNode().child(L"controls").child(ctrlName);
	if(xmlCtrl)
	{
		pugi::xml_node skinAttr = xmlCtrl.child(L"groups").child(L"propgroup").first_child();
		while(skinAttr)
		{
			if(skinAttr.attribute(L"name"))
			{
				attrs.insert(skinAttr.attribute(L"name").as_string());
			}
			skinAttr = skinAttr.next_sibling();
		}
		if(xmlCtrl.attribute(L"parent"))
		{
			SStringW strParent = xmlCtrl.attribute(L"parent").as_string();
			if(!strParent.IsEmpty())
			{
				SStringWList parents;
				SplitString(strParent,L'|',parents);
				for(int i=0;i<parents.GetCount();i++)
				{
					_GetCtrlAttrs(parents[i],attrs);
				}
			}
		}
	}
}

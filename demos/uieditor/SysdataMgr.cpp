#include "stdafx.h"
#include "SysdataMgr.h"
#include <vector>
#include <algorithm>
#include "helpapi.h"
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
	SOUI::SXmlNode xmlCtrl = getCtrlDefNode().child(L"controls").first_child();
	while(xmlCtrl)
	{
		strAuto += xmlCtrl.Name();
		strAuto += L' ';
		xmlCtrl = xmlCtrl.NextSibling();
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
	m_xmlCtrlDef.load_file(m_strConfigDir + _T("/Ctrl.xml"));
}

SOUI::SXmlNode CSysDataMgr::getCtrlDefNode()
{
	return m_xmlCtrlDef.root().child(L"root");
}

void CSysDataMgr::InitSkinProp()
{
	m_xmlSkinProp.load_file(m_strConfigDir + _T("/Skin.xml"));
}

SOUI::SXmlNode CSysDataMgr::getSkinDefNode()
{
	return m_xmlSkinProp.root().child(L"root");
}

SStringW CSysDataMgr::GetUserWidgetParent(SStringW strWidgetName) const
{
    SXmlNode xmlUserWidgets = m_xmlCtrlDef.root().child(L"root").child(L"user_widgets");
	if(xmlUserWidgets){
		SXmlNode xmlWidget = xmlUserWidgets.child(strWidgetName);
		if(xmlWidget){
			return xmlWidget.attribute(L"parent").value();
		}
	}
	return L"";
}

SStringW CSysDataMgr::GetUserSkinParent(SStringW strSkinName) const
{
    SXmlNode xmlUserWidgets = m_xmlSkinProp.root().child(L"root").child(L"user_skins");
	if(xmlUserWidgets){
		SXmlNode xmlWidget = xmlUserWidgets.child(strSkinName);
		if(xmlWidget){
			return xmlWidget.attribute(L"parent").value();
		}
	}
	return L"";
}

SOUI::SStringT CSysDataMgr::GetConfigDir() const
{
	return m_strConfigDir;
}

SOUI::SStringA CSysDataMgr::GetSkinAutos()
{
	std::set<SStringW> skins;
	SOUI::SXmlNode xmlSkin = getSkinDefNode().child(L"skins").first_child();
	while(xmlSkin)
	{
		if(xmlSkin.attribute(L"visible", TRUE).as_bool(true))
		{
			skins.insert(xmlSkin.Name());
		}
		xmlSkin = xmlSkin.NextSibling();
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
	SOUI::SXmlNode xmlSkin = getSkinDefNode().child(L"skins").child(skinName);
	if(xmlSkin)
	{
		SOUI::SXmlNode skinAttr = xmlSkin.child(L"groups").child(L"propgroup").first_child();
		while(skinAttr)
		{
			if(skinAttr.attribute(L"name"))
			{
				attrs.insert(skinAttr.attribute(L"name").value());
			}
			skinAttr = skinAttr.NextSibling();
		}
		if(xmlSkin.attribute(L"parent"))
		{
			SStringW strParent = xmlSkin.attribute(L"parent").value();
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
	SOUI::SXmlNode xmlCtrl = getCtrlDefNode().child(L"controls").child(ctrlName);
	if(xmlCtrl)
	{
		SOUI::SXmlNode skinAttr = xmlCtrl.child(L"groups").child(L"propgroup").first_child();
		while(skinAttr)
		{
			if(skinAttr.attribute(L"name"))
			{
				attrs.insert(skinAttr.attribute(L"name").value());
			}
			skinAttr = skinAttr.NextSibling();
		}
		if(xmlCtrl.attribute(L"parent"))
		{
			SStringW strParent = xmlCtrl.attribute(L"parent").value();
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

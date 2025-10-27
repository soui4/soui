#pragma once

#include <set>
#include <xml/SXml.h>

class CSysDataMgr : public SOUI::SSingleton<CSysDataMgr>
{
public:
	struct CtrlAttrItem
	{
		SStringT attrname;
		SOUI::SXmlDoc* attrdoc;
		CtrlAttrItem() { ; }
		CtrlAttrItem(SStringT name, SOUI::SXmlNode xmlnode)
		{
			attrname = name;
			attrdoc = new SOUI::SXmlDoc();
			attrdoc->Root()->AppendCopyNode(&xmlnode);
		}
	};

	typedef SArray<CtrlAttrItem> CTRL_ATTR_VALUE;

	CSysDataMgr();
	~CSysDataMgr();

	// 读取系统数据, 控件属性表
	bool LoadSysData(LPCTSTR cfgDir);
	
	SStringT GetConfigDir() const;

	void InitSkinProp();

	void InitCtrlDef();

	// 获取控件名称列表
	SStringA GetCtrlAutos();

	// 获取指定控件的自动完成字串
	SStringA GetCtrlAttrAutos(SStringW ctrlname);

	SStringA GetSkinAutos();

	SStringA GetSkinAttrAutos(SStringW skinName);

	SOUI::SXmlNode getCtrlDefNode();

	SOUI::SXmlNode getSkinDefNode();

	SStringW GetUserWidgetParent(SStringW strWidgetName) const;
	SStringW GetUserSkinParent(SStringW strSkinName) const;
private:
	void _GetSkinAttrs(SStringW skinName,std::set<SStringW> &attrs);
	void _GetCtrlAttrs(SStringW skinName,std::set<SStringW> &attrs);

	SOUI::SXmlDoc m_xmlCtrlDef;

	SOUI::SXmlDoc m_xmlSkinProp;
	SStringT m_strConfigDir;
};
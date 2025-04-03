#pragma once

#include <set>

class CSysDataMgr : public SOUI::SSingleton<CSysDataMgr>
{
public:
	struct CtrlAttrItem
	{
		SStringT attrname;
		pugi::xml_document* attrdoc;
		CtrlAttrItem() { ; }
		CtrlAttrItem(SStringT name, pugi::xml_node xmlnode)
		{
			attrname = name;
			attrdoc = new pugi::xml_document();
			attrdoc->append_copy(xmlnode);
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

	pugi::xml_node getCtrlDefNode();

	pugi::xml_node getSkinDefNode();
private:
	void _GetSkinAttrs(SStringW skinName,std::set<SStringW> &attrs);
	void _GetCtrlAttrs(SStringW skinName,std::set<SStringW> &attrs);

	pugi::xml_document m_xmlCtrlDef;

	pugi::xml_document m_xmlSkinProp;

	SStringT m_strConfigDir;
};
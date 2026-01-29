#ifndef __RESMANGER_H__
#define __RESMANGER_H__

#include "monitor/PathMonitor.h"
#include <res.mgr/SUiDef.h>

class CMainDlg;

enum{
    FT_UNKNOWN=0,
    FT_DIR,
    FT_INDEX_XML,
    FT_LAYOUT_XML,
    FT_XML,
    FT_IMAGE,
    FT_SKIN,
    FT_COLOR,
    FT_STRING,
    FT_STYLE,
    FT_OBJATTR,
    FT_DIM,
    FT_FONT,
};

class ResManger : public CPathMonitor::IListener
{
public:
	ResManger(CMainDlg *pMainDlg);
	~ResManger();

	struct SkinItem
	{
		SStringT class_name;
		SStringT name;
		SStringT src;
		SkinItem() {}
		SkinItem(SStringT _classname, SStringT _name, SStringT _src, pugi::xml_node xmlnode)
		{
			class_name = _classname;
			name = _name;
			src = _src;
		}
	};

	struct ValueItem
	{
		SStringT class_name;
		SStringT value;

		ValueItem() { ; }
		ValueItem(SStringT _classname, SStringT _value)
		{
			class_name = _classname;
			value = _value;
		}
	};

	// 添加获取资源提供者的公共方法
	IResProvider* GetResProvider() const;
	
	// 添加获取UI定义信息的公共方法
	IUiDefInfo* GetUiDefInfo() const;
	
	// 添加设置资源提供者的方法
	void SetResProvider(IResProvider* pResProvider);
	
	// 添加设置UI定义信息的方法
	void SetUiDefInfo(IUiDefInfo* pUiDef);

	BOOL OpenProject(SStringT strPath);
	void CloseProject();

	void UpdateProject();
	
	void SaveRes();
	BOOL LoadUIRes();
	
	void LoadSkinFile();
	void LoadStringFile();
	void LoadColorFile();
	void LoadStyleFile();
	void LoadObjattrFile();
	void LoadDimensionFile();
	void LoadFontFile();

	void LoadSkinNode(pugi::xml_node xmlNode);
	
	SkinItem GetSkinByImg(SStringT srcimg);
	SkinItem GetSkinByName(SStringT skinname);
	ISkinObj *GetSkin(SStringW skinname,int scale);

	int GetImageList(std::vector<std::pair<SStringT,SStringT> > &lstSrc);
	int GetSkinList(std::vector<SStringT> &lstSkin);
	SStringA GetSkinAutos(SStringT prev);
	
	SStringA GetStyleAutos(SStringT prev);
	
	SStringA GetStringAutos(SStringT prev);
	
	COLORREF GetColor(SStringT colorname);
	int GetColorList(std::vector<SStringT> &lstColor);
	SStringA GetColorAutos(SStringT prev);
	
	int GetStringList(std::vector<SStringT> &lstString);
	SStringT GetString(SStringT stringname);

	int GetStyleList(std::vector<SStringT> &lstStyle);
	SStringW GetStyle(SStringT stylename);

	int GetDimensionList(std::vector<SStringT> &lstDimension);
	SStringW GetDimension(SStringT dimname);
	int GetFontList(std::vector<SStringT> &lstFont);
	SStringW GetFont(SStringT fontname);

	BOOL IsSkinXml(const SStringT & strPath) const;
	BOOL IsLayoutXml(const SStringT & strPath, SStringT & layoutId) const;
			
	BOOL NewLayout(SStringT strResName, SStringT strPath);

	BOOL AddToUIRes(SStringT strPath);

	int GetFileType(SStringT strPath) const;
protected:
	void OnFileChanged(LPCTSTR pszFile, CPathMonitor::Flag nFlag) override;
	void _OnFileChanged(tstring &pszFile, CPathMonitor::Flag nFlag);
protected:
	pugi::xml_node GetResFirstNode(const SStringT tagname);
	void LoadResFileEx(SStringT & filepath, pugi::xml_document &xmlDoc, SStringT tagname);

public:
	SStringT m_strProPath;			// 加载资源文件的根目录
	SStringT m_strUIResFile;		// uires.idx 完整文件名
	SStringT m_strInitFile;			// Init.xml 完整文件名

	SStringT m_strSkinFile;			// skin完整文件名
	SStringT m_strStringFile;		// string完整文件名
	SStringT m_strColorFile;		// color完整文件名
	SStringT m_strStyleFile;		// Style完整文件名
	SStringT m_strObjattrFile;		// Objattr完整文件名
	SStringT m_strDimensionFile;	// Dimension完整文件名
	SStringT m_strFontFile;

	pugi::xml_document m_xmlDocUiRes;			// uires.idx文件xml doc
	pugi::xml_node m_xmlNodeUiRes;				// uires.idx文件的根结点
	pugi::xml_document m_xmlDocSkin;			// skin定义文件xml doc

	pugi::xml_document m_xmlDocColor;			// Color定义文件xml doc
	pugi::xml_document m_xmlDocString;			// String定义文件xml doc
	pugi::xml_document m_xmlDocStyle;			// Style定义文件xml doc
	pugi::xml_document m_xmlDocObjattr;			// Objattr定义文件xml doc
	pugi::xml_document m_xmlDocDimension;		// Dimension定义文件xml doc
	pugi::xml_document m_xmlDocFont;			// Font定义文件xml doc

	SAutoRefPtr<IResProvider> m_pResProvider;
	SAutoRefPtr<IUiDefInfo> m_pUiDef;


	SMap<SStringT, SStringT> m_mapLayoutFile;	// 所有Layout文件信息

	SMap<SStringT, SkinItem> m_mapSkins;		//所有定义的Skin项目
	SMap<SStringT, SStringW> m_mapStyles;		//所有定义的Style项目
	SMap<SStringT, ValueItem> m_mapStrings;		//所有定义的String
	SMap<SStringT, ValueItem> m_mapColors;		//所有定义的Color
	SMap<SStringT, SStringW> m_mapDimensions;	//所有定义的Dimension
	SMap<SStringT, SStringW> m_mapFonts;		//所有定义的Font项目

	CMainDlg *m_pMainDlg;
};

#endif//__RESMANGER_H__
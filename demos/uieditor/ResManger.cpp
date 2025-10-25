#include "stdafx.h"
#include "ResManger.h"
#include "helpapi.h"
#include <vector>
#include <algorithm>
#include <helper/slog.h>
#include <helper/SFunctor.hpp>
#include <MainDlg.h>
#define kLogTag "ResManger"

ResManger::ResManger(CMainDlg *pMainDlg):m_pMainDlg(pMainDlg),m_pathMonitor(this)
{
}


ResManger::~ResManger()
{
}

// 添加获取资源提供者的公共方法
IResProvider* ResManger::GetResProvider() const
{
    return m_pResProvider;
}

// 添加获取UI定义信息的公共方法
IUiDefInfo* ResManger::GetUiDefInfo() const
{
    return m_pUiDef;
}

// 添加设置资源提供者的方法
void ResManger::SetResProvider(IResProvider* pResProvider)
{
    m_pResProvider = pResProvider;
}

// 添加设置UI定义信息的方法
void ResManger::SetUiDefInfo(IUiDefInfo* pUiDef)
{
    m_pUiDef = pUiDef;
}

BOOL ResManger::OpenProject(SStringT strPath)
{
	m_strProPath = strPath.Mid(0, strPath.ReverseFind(TPATH_SLASH));
	m_strUIResFile = strPath;

	if(!LoadUIRes())
		return FALSE;
	LoadSkinFile();
	LoadStringFile();
	LoadColorFile();
	LoadStyleFile();
	LoadObjattrFile();
	LoadDimensionFile();
	LoadFontFile();
	m_pathMonitor.SetPath(m_strProPath);

	
	SouiFactory souiFac;
	m_pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
	m_pResProvider->Init((WPARAM)m_strProPath.c_str(),0);
	SApplication::getSingletonPtr()->AddResProvider(m_pResProvider,NULL);
	m_pUiDef = SUiDef::CreateUiDefInfo();
	m_pUiDef->Init(m_pResProvider, _T("UIDEF:XML_INIT"));
	SApplication::getSingletonPtr()->RemoveResProvider(m_pResProvider);

	return TRUE;
}

void ResManger::CloseProject()
{
	m_pUiDef=NULL;
	m_pResProvider=NULL;
	m_pathMonitor.Stop();
	m_strUIResFile = m_strProPath = _T("");

	m_mapResFile.RemoveAll();
	m_mapXmlFile.RemoveAll();
	m_mapSkins.RemoveAll();
	m_mapStrings.RemoveAll();
	m_mapColors.RemoveAll();
	m_mapStyles.RemoveAll();
	m_mapLayoutFile.RemoveAll();
	m_mapDimensions.RemoveAll();
	m_mapFonts.RemoveAll();

	m_strSkinFile = _T("");
	m_strStringFile = _T("");
	m_strColorFile =  _T("");
	m_strStyleFile = _T("");
	m_strObjattrFile =  _T("");
	m_strDimensionFile = _T("");
	m_strFontFile = _T("");

	m_xmlDocSkin.reset();
	m_xmlDocString.reset();
	m_xmlDocColor.reset();
	m_xmlDocStyle.reset();
	m_xmlDocObjattr.reset();
	m_xmlDocDimension.reset();
	m_xmlDocFont.reset();
	m_xmlDocUiRes.reset();
}

void ResManger::SaveRes()
{
	if (!m_strSkinFile.IsEmpty())
		m_xmlDocSkin.save_file(m_strSkinFile);
	m_xmlDocUiRes.save_file(m_strUIResFile);

	//重新加载
	LoadUIRes();
	LoadSkinFile();
	LoadStringFile();
	LoadColorFile();
	LoadStyleFile();
	LoadObjattrFile();
}

pugi::xml_node ResManger::GetResFirstNode(const SStringT tagname)
{
	pugi::xml_document * xmlDoc = &m_xmlDocSkin;
	if (tagname == _T("string"))
		xmlDoc = &m_xmlDocString;
	if (tagname == _T("color"))
		xmlDoc = &m_xmlDocColor;
	if (tagname == _T("style"))
		xmlDoc = &m_xmlDocStyle;
	if (tagname == _T("objattr"))
		xmlDoc = &m_xmlDocObjattr;
	if(tagname == _T("skin"))
		xmlDoc = &m_xmlDocSkin;
	if(tagname == _T("dim"))
		xmlDoc = &m_xmlDocDimension;
	if(tagname == _T("font"))
		xmlDoc = &m_xmlDocFont;

	SStringW tagNameW=S_CT2W(tagname);
	pugi::xml_node xmlNode;
	if (xmlDoc->child(tagNameW))
	{
		xmlNode = xmlDoc->child(tagNameW).first_child();
	}
	else if (xmlDoc->child(L"UIDEF"))
	{
		xmlNode = xmlDoc->child(L"UIDEF").child(tagNameW).first_child();
	}
	return xmlNode;
}

BOOL ResManger::LoadUIRes()
{
	if (!m_xmlDocUiRes.load_file(m_strUIResFile, pugi::parse_full))
	{
		SLOGW()<<"加载uires文件失败";
		return FALSE;
	}

	m_xmlNodeUiRes = m_xmlDocUiRes.root();

	m_mapResFile.RemoveAll();
	m_mapXmlFile.RemoveAll();

	pugi::xml_node xmlNode = m_xmlNodeUiRes.child(L"resource").first_child();
	GetSubNodes(xmlNode, L"");
	// 获取 Init.xml 文件名
	pugi::xml_node xmlNode_init = m_xmlNodeUiRes.child(L"resource").child(L"UIDEF").first_child();

	while (xmlNode_init)
	{
		if (xmlNode_init.type() == pugi::node_element)
		{
			break;
		}
		else
		{
			xmlNode_init = xmlNode_init.next_sibling();
		}
	}

	{//enum xml files
		const LPCWSTR kXmlTypes[] = { L"uidef",L"xml",L"layout", L"values", L"smenu", L"smenuex", L"style", L"objattr"};
		pugi::xml_node xmlType = m_xmlNodeUiRes.child(L"resource").first_child();
		while (xmlType) {
			SStringW nodeName = xmlType.name();
			BOOL bXmlType = FALSE;
			BOOL bLayout = FALSE;
			for (int i = 0; i < ARRAYSIZE(kXmlTypes); i++)
			{
				if (nodeName.CompareNoCase(kXmlTypes[i]) == 0)
				{
					bXmlType = TRUE;
					if (nodeName.CompareNoCase(L"layout") == 0)
					{
						bLayout = TRUE;
					}
					break;
				}
			}
			if(!bLayout)
				bLayout = xmlType.attribute(L"layout").as_bool(false);
			if (bXmlType || bLayout)
			{
				pugi::xml_node xmlFile = xmlType.child(L"file");
				while (xmlFile)
				{
					SStringW strName = xmlFile.attribute(L"name").value();
					SStringW strPath = xmlFile.attribute(L"path").value();
					if (!strName.IsEmpty() && !strPath.IsEmpty())
					{
						m_mapXmlFile[S_CW2T(strName)] = S_CW2T(strPath);
					}
					if (nodeName.CompareNoCase(L"uidef") == 0)
					{
						m_strInitFile = S_CW2T(strPath);
					}
					else if (nodeName.CompareNoCase(L"values") == 0)
					{
						if(strName.CompareNoCase(L"skin") == 0)
							m_strSkinFile = S_CW2T(strPath);
						else if (strName.CompareNoCase(L"string") == 0)
							m_strStringFile = S_CW2T(strPath);
						else if (strName.CompareNoCase(L"color") == 0)
							m_strColorFile = S_CW2T(strPath);
						else if (strName.CompareNoCase(L"style") == 0)
							m_strStyleFile = S_CW2T(strPath);
						else if (strName.CompareNoCase(L"objattr") == 0)
							m_strObjattrFile = S_CW2T(strPath);
					}
					if(bLayout){
						SStringW strLayoutId = SStringW(xmlType.name())+L":"+strName;
						m_mapLayoutFile[S_CW2T(strLayoutId)] = S_CW2T(strPath);
					}
					xmlFile = xmlFile.next_sibling(L"file");
				}
			}
			xmlType = xmlType.next_sibling();
		}
	}
	return TRUE;
}

void ResManger::GetSubNodes(pugi::xml_node& parentNode, SStringW parentNodeName)
{
	while (parentNode)
	{
		if (parentNode.type() == pugi::node_element)
		{
			SStringW strParentName = parentNode.name();
			if (parentNode.first_child() != NULL)
			{
				pugi::xml_node childNode = parentNode.first_child();
				GetSubNodes(childNode, strParentName + L":");
			}
			else
			{
				if (strParentName == L"file")
				{
					SStringW strName, strPath;
					strName = parentNode.attribute(L"name").value();
					strPath = parentNode.attribute(L"path").value();
					if (!strName.IsEmpty() && !strPath.IsEmpty())
					{
						SStringT strKey = S_CW2T(parentNodeName + strName);
						strKey.MakeLower();
						SStringT strPathT=S_CW2T(strPath);
						SStringT extname = GetFileExtname(strPathT);
						if (extname.CompareNoCase(_T(".xml")) == 0)
						{
							m_mapXmlFile[strKey] = strPathT;
						}
						else
						{
							m_mapResFile[strKey] = strPathT;
						}
					}
				}
			}
		}
		parentNode = parentNode.next_sibling();
	}
}

// 删除资源类型名 如LAYOUT:sin_manm 将变成 sin_manm
SStringT ResManger::RemoveResTypename(const SStringT& resname)
{
	int nPos = resname.ReverseFind(':');
	if (nPos == -1)	nPos = 0;
	SStringT name = resname.Mid(nPos + 1);
	name.Trim();
	return name;
}

SStringT ResManger::GetResPathByName(const SStringT& resname)
{
	SStringT key = resname; 
	key.MakeLower();
	const SMap<SStringT, SStringT>::CPair * pFilePair = m_mapXmlFile.Lookup(key);
	if (pFilePair == NULL)
	{
		pFilePair = m_mapResFile.Lookup(key);
	}
	if (pFilePair == NULL)
		return _T("");

	return m_strProPath + TPATH_SLASH + pFilePair->m_value;
}

void ResManger::LoadResFileEx(SStringT &filepath, pugi::xml_document &xmlDoc, SStringT tagname)
{
	SStringT filePath2=filepath;
	filePath2.ReplaceChar(_T('\\'),_T('/'));
	if (!m_strInitFile.IsEmpty())
	{
		pugi::xml_parse_result result = xmlDoc.load_file(m_strProPath + TPATH_SLASH + filePath2, pugi::parse_full);
		if (!result){
			SStringT tmpstr;
			tmpstr.Format(_T("加载%s文件失败"), tagname.c_str());
			SLOGD()<<tmpstr.c_str();
		}
	}
}

void ResManger::LoadSkinNode(pugi::xml_node xmlNode)
{
	m_mapSkins.RemoveAll();
	while (xmlNode)
	{
		if (xmlNode.type() != pugi::node_element)
		{
			xmlNode = xmlNode.next_sibling();
			continue;
		}

		SStringT s1, s2, s3;
		s1 = S_CW2T(xmlNode.name());
		s2 = S_CW2T(xmlNode.attribute(L"name").value());
		s3 = S_CW2T(xmlNode.attribute(L"src").value());

		m_mapSkins[s2] = SkinItem(s1, s2, s3, xmlNode);
		xmlNode = xmlNode.next_sibling();
	}
}

void ResManger::LoadSkinFile()
{
	LoadResFileEx(m_strSkinFile, m_xmlDocSkin, _T("skin"));
	pugi::xml_node xmlNode = GetResFirstNode(_T("skin"));
	LoadSkinNode(xmlNode);
}

void ResManger::LoadStringFile()
{
	LoadResFileEx(m_strStringFile, m_xmlDocString, _T("string"));
	m_mapStrings.RemoveAll();
	pugi::xml_node xmlNode = GetResFirstNode(_T("string"));
	while (xmlNode)
	{
		if (xmlNode.type() != pugi::node_element)
		{
			xmlNode = xmlNode.next_sibling();
			continue;
		}

		SStringT s1, s2;
		s1 = S_CW2T(xmlNode.name());
		s2 = S_CW2T(xmlNode.attribute(L"value").value());

		m_mapStrings[s1] = ValueItem(s1, s2);
		xmlNode = xmlNode.next_sibling();
	}
}

void ResManger::LoadColorFile()
{
	LoadResFileEx(m_strColorFile, m_xmlDocColor, _T("color"));
	m_mapColors.RemoveAll();
	pugi::xml_node xmlNode = GetResFirstNode(_T("color"));
	while (xmlNode)
	{
		if (xmlNode.type() != pugi::node_element)
		{
			xmlNode = xmlNode.next_sibling();
			continue;
		}

		SStringT s1, s2;
		s1 = S_CW2T(xmlNode.name());
		s2 = S_CW2T(xmlNode.attribute(L"value").value());

		m_mapColors[s1] = ValueItem(s1, s2);
		xmlNode = xmlNode.next_sibling();
	}
}

void ResManger::LoadStyleFile()
{
	LoadResFileEx(m_strStyleFile, m_xmlDocStyle, _T("style"));
	m_mapStyles.RemoveAll();
	pugi::xml_node xmlNode = GetResFirstNode(_T("style"));
	while (xmlNode)
	{
		if (xmlNode.type() != pugi::node_element)
		{
			xmlNode = xmlNode.next_sibling();
			continue;
		}
		pugi::xml_writer_buff writer;
		xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
		SStringW strXml = SStringW(writer.buffer(), writer.size());
		SStringT strName = S_CW2T(xmlNode.attribute(L"name").value());
		m_mapStyles[strName] = strXml;
		xmlNode = xmlNode.next_sibling();
	}
}

void ResManger::LoadDimensionFile()
{
	LoadResFileEx(m_strDimensionFile, m_xmlDocDimension, _T("dim"));
	m_mapDimensions.RemoveAll();
	pugi::xml_node xmlNode = GetResFirstNode(_T("dim"));
	while (xmlNode)
	{
		if (xmlNode.type() != pugi::node_element)
		{
			xmlNode = xmlNode.next_sibling();
			continue;
		}
		pugi::xml_writer_buff writer;
		xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
		SStringW strXml = SStringW(writer.buffer(), writer.size());
		SStringT strName = S_CW2T(xmlNode.attribute(L"name").value());
		m_mapDimensions[strName] = strXml;
		xmlNode = xmlNode.next_sibling();
	}
}

void ResManger::LoadFontFile()
{
	LoadResFileEx(m_strFontFile, m_xmlDocFont, _T("font"));
	m_mapFonts.RemoveAll();
	pugi::xml_node xmlNode = GetResFirstNode(_T("font"));
	while (xmlNode)
	{
		if (xmlNode.type() != pugi::node_element)
		{
			xmlNode = xmlNode.next_sibling();
			continue;
		}
		pugi::xml_writer_buff writer;
		xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
		SStringW strXml = SStringW(writer.buffer(), writer.size());
		SStringT strName = S_CW2T(xmlNode.attribute(L"name").value());
		m_mapFonts[strName] = strXml;
		xmlNode = xmlNode.next_sibling();
	}
}

void ResManger::LoadObjattrFile()
{
	LoadResFileEx(m_strObjattrFile, m_xmlDocObjattr, _T("objattr"));
}

ResManger::SkinItem ResManger::GetSkinByName(SStringT skinname)
{
	SPOSITION pos = m_mapSkins.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, SkinItem>::CPair* item = m_mapSkins.GetAt(pos);
		if (skinname.CompareNoCase(item->m_value.name) == 0)
		{
			return item->m_value;
		}

		m_mapSkins.GetNext(pos);
	}

	return SkinItem();
}

ISkinObj *ResManger::GetSkin(SStringW skinname,int scale)
{
	return m_pUiDef->GetSkinPool()->GetSkin(skinname,scale);
}

ResManger::SkinItem ResManger::GetSkinByImg(SStringT srcimg)
{
	SPOSITION pos = m_mapSkins.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, SkinItem>::CPair* item = m_mapSkins.GetAt(pos);
		if (srcimg.CompareNoCase(item->m_value.src) == 0)
		{
			return item->m_value;
		}

		m_mapSkins.GetNext(pos);
	}

	return SkinItem();
}

int ResManger::GetImageList(std::vector< std::pair<SStringT,SStringT> > &lstSrc)
{
	static const wchar_t* kImageTypes[] ={
		L"image",
		L"img",
		L"png",
		L"imgx",
	};
	pugi::xml_node xmlNode = m_xmlNodeUiRes.child(L"resource").first_child();
	while(xmlNode){
		SStringW strNodeName = xmlNode.name();
		BOOL bImgNode = wcsicmp(xmlNode.attribute(L"type").value(),L"image") == 0;
		if(!bImgNode){
			for(int i=0;i<ARRAYSIZE(kImageTypes);i++)
			{
				if(wcsicmp(strNodeName,kImageTypes[i]) == 0)
				{
					bImgNode = TRUE;
					break;
				}
			}
		}
		if(bImgNode)
		{
			pugi::xml_node fileNode = xmlNode.child(L"file");
			while(fileNode){
				SStringW src = strNodeName + L":" + fileNode.attribute(L"name").value();
				SStringT strPath = m_strProPath + TPATH_SLASH + S_CW2T(fileNode.attribute(L"path").value());
				strPath.ReplaceChar(_T('\\'),_T('/'));
				lstSrc.push_back(std::make_pair(S_CW2T(src),strPath));
				fileNode = fileNode.next_sibling();
			}
		}
		xmlNode = xmlNode.next_sibling();
	}
    return 0;
}

int ResManger::GetSkinList(std::vector<SStringT> &lstSkin)
{
	lstSkin.clear();
	SPOSITION pos = m_mapSkins.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, SkinItem>::CPair* item = m_mapSkins.GetAt(pos);
		lstSkin.push_back(item->m_key);

		m_mapSkins.GetNext(pos);
	}
	std::sort(lstSkin.begin(), lstSkin.end(), SortSStringNoCase);
	return lstSkin.size();
}
SStringA ResManger::GetSkinAutos(SStringT prev)
{
	std::vector<SStringT> vecTemp;
	GetSkinList(vecTemp);

	SStringT strAuto;
	std::vector<SStringT>::iterator it = vecTemp.begin();
	for (; it != vecTemp.end(); it++)
	{
		strAuto += prev;
		strAuto += *it + _T(" ");
	}
	strAuto.TrimRight(' ');

	SStringA str = S_CT2A(strAuto, CP_UTF8);
	return str;
}

SStringA ResManger::GetStyleAutos(SStringT prev)
{
	std::vector<SStringT> vecTemp;
	SPOSITION pos = m_mapStyles.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, SStringW>::CPair* item = m_mapStyles.GetAt(pos);
		vecTemp.push_back(item->m_key);

		m_mapStyles.GetNext(pos);
	}
	std::sort(vecTemp.begin(), vecTemp.end(), SortSStringNoCase);

	SStringT strAuto;
	std::vector<SStringT>::iterator it = vecTemp.begin();
	for (; it != vecTemp.end(); it++)
	{
		strAuto += prev;
		strAuto += *it + _T(" ");
	}
	strAuto.TrimRight(' ');

	SStringA str = S_CT2A(strAuto, CP_UTF8);
	return str;
}

SStringA ResManger::GetStringAutos(SStringT prev)
{
	std::vector<SStringT> vecTemp;
	SPOSITION pos = m_mapStrings.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, ValueItem>::CPair* item = m_mapStrings.GetAt(pos);
		vecTemp.push_back(item->m_key);

		m_mapStrings.GetNext(pos);
	}
	std::sort(vecTemp.begin(), vecTemp.end(), SortSStringNoCase);

	SStringT strAuto;
	std::vector<SStringT>::iterator it = vecTemp.begin();
	for (; it != vecTemp.end(); it++)
	{
		strAuto += prev;
		strAuto += *it + _T(" ");
	}
	strAuto.TrimRight(' ');

	SStringA str = S_CT2A(strAuto, CP_UTF8);
	return str;
}

COLORREF ResManger::GetColor(SStringT colorname)
{
	COLORREF cr = CR_INVALID;
	m_pUiDef->GetNamedColor().Get(S_CT2W(colorname),cr);
	return cr;
}
int ResManger::GetColorList(std::vector<SStringT> &lstColor)
{
	lstColor.clear();
	SPOSITION pos = m_mapColors.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, ValueItem>::CPair* item = m_mapColors.GetAt(pos);
		lstColor.push_back(item->m_key);

		m_mapColors.GetNext(pos);
	}
	std::sort(lstColor.begin(), lstColor.end(), SortSStringNoCase);
	return lstColor.size();
}
SStringA ResManger::GetColorAutos(SStringT prev)
{
	std::vector<SStringT> vecTemp;
	GetColorList(vecTemp);

	SStringT strAuto;
	std::vector<SStringT>::iterator it = vecTemp.begin();
	for (; it != vecTemp.end(); it++)
	{
		strAuto += prev;
		strAuto += *it + _T(" ");
	}
	strAuto.TrimRight(' ');

	SStringA str = S_CT2A(strAuto, CP_UTF8);
	return str;
}
int ResManger::GetStringList(std::vector<SStringT> &lstString)
{
	lstString.clear();
	SPOSITION pos = m_mapStrings.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, ValueItem>::CPair* item = m_mapStrings.GetAt(pos);
		lstString.push_back(item->m_key);

		m_mapStrings.GetNext(pos);
	}
	std::sort(lstString.begin(), lstString.end(), SortSStringNoCase);
	return lstString.size();
}

SStringT ResManger::GetString(SStringT stringname)
{
	ValueItem item;
	if (m_mapStrings.Lookup(stringname, item))
	{
		return item.value;
	}

	return SStringT();
}

int ResManger::GetStyleList(std::vector<SStringT> &lstStyle)
{
	lstStyle.clear();
	SPOSITION pos = m_mapStyles.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, SStringW>::CPair* item = m_mapStyles.GetAt(pos);
		lstStyle.push_back(item->m_key);

		m_mapStyles.GetNext(pos);
	}
	std::sort(lstStyle.begin(), lstStyle.end(), SortSStringNoCase);
	return lstStyle.size();
}

SStringW ResManger::GetStyle(SStringT stylename)
{
	SStringW strStyle;
	if (m_mapStyles.Lookup(stylename, strStyle))
	{
		return strStyle;
	}

	return SStringW();
}

int ResManger::GetDimensionList(std::vector<SStringT> &lstDimension)
{
	lstDimension.clear();
	SPOSITION pos = m_mapDimensions.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, SStringW>::CPair* item = m_mapDimensions.GetAt(pos);
		lstDimension.push_back(item->m_key);
	}
	std::sort(lstDimension.begin(), lstDimension.end(), SortSStringNoCase);
	return lstDimension.size();
}

SStringW ResManger::GetDimension(SStringT dimname)
{
	SStringW strDim;
	if (m_mapDimensions.Lookup(dimname, strDim))
	{
		return strDim;
	}

	return SStringW();
}

int ResManger::GetFontList(std::vector<SStringT> &lstFont)
{
	lstFont.clear();
	SPOSITION pos = m_mapFonts.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, SStringW>::CPair* item = m_mapFonts.GetAt(pos);
		lstFont.push_back(item->m_key);
	}
	std::sort(lstFont.begin(), lstFont.end(), SortSStringNoCase);
	return lstFont.size();
}

SStringW ResManger::GetFont(SStringT fontname)
{
	SStringW strFont;
	if (m_mapFonts.Lookup(fontname, strFont))
	{
		return strFont;
	}

	return SStringW();
}
BOOL ResManger::IsSkinXml(const SStringT &strPath) const
{
	return m_strSkinFile.CompareNoCase(strPath) == 0;
}

BOOL ResManger::NewLayout(SStringT strResName, SStringT strPath)
{
    SStringT strShortPath = strPath.Mid(m_strProPath.GetLength() + 1);

    pugi::xml_node xmlNode = m_xmlDocUiRes.child(L"resource").child(L"LAYOUT");

    if (xmlNode)
    {
        pugi::xml_node Child = xmlNode.append_child(L"file");
        Child.append_attribute(L"name").set_value(S_CT2W(strResName));
        Child.append_attribute(L"path").set_value(S_CT2W(strShortPath));

        m_xmlDocUiRes.save_file(m_strUIResFile);
    }
    SStringT strType = SStringT().Format(_T("layout:%s"), strResName.c_str());
    m_mapXmlFile[strType] = strPath;
    return TRUE;
}

void ResManger::OnFileChanged(LPCTSTR pszFile, CPathMonitor::Flag nFlag)
{
	STaskHelper::post(m_pMainDlg->GetMsgLoop(),this,&ResManger::_OnFileChanged,tstring(pszFile),nFlag);
}

void ResManger::_OnFileChanged(tstring & strFile, CPathMonitor::Flag nFlag)
{
	SStringT strFile2(strFile.c_str(),strFile.length());
	strFile2 = strFile2.Mid(m_strProPath.GetLength() + 1);
	strFile2.ReplaceChar(_T('/'), _T('\\'));
	if(nFlag == CPathMonitor::FILE_MODIFIED)
	{
		if(m_strUIResFile.CompareNoCase(strFile2.c_str()) == 0)
		{
			LoadUIRes();
		}
		else if(m_strSkinFile.CompareNoCase(strFile2.c_str()) == 0)
		{
			LoadSkinFile();
		}
		else if(m_strStringFile.CompareNoCase(strFile2.c_str()) == 0)
		{
			LoadStringFile();
		}
		else if(m_strColorFile.CompareNoCase(strFile2.c_str()) == 0)
		{
			LoadColorFile();
		}
		else if(m_strStyleFile.CompareNoCase(strFile2.c_str()) == 0)
		{
			LoadStyleFile();
		}
		else if(m_strObjattrFile.CompareNoCase(strFile2.c_str()) == 0)
		{
			LoadObjattrFile();
		}
	}
	//todo: handle other flags

}
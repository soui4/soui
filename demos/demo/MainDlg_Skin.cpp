/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg_Skin.cpp
* @brief      CMainDlg 换肤演示
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    演示 SOUI 的运行时换肤能力:
*               - SDemoSkin(demoskinbk):承载主窗口背景的自定义皮肤,
*                 支持内置图片/纯色/系统皮肤三种类型;
*               - 换肤配置持久化到 /themes/skin_config.xml;
*               - 主题色变化通过 g_dwSkinChangeMessage 广播,主窗口收到后:
*                   1. 由主题色经 HSL 推导整套界面命名色(clr_* 槽位:保留各
*                      槽位明度,采用主题色色相,饱和度按主题色/出厂主色缩放);
*                   2. 遍历控件树,把窗口样式色中等于"上一次已应用值"的替换
*                      为新值,支持在任意主题之间连续切换;
*                   3. 位图皮肤统一 DoColorize 染色;主题色无效(CR_INVALID,
*                      内置皮肤)时回退出厂配色并取消染色。
*/

#include "stdafx.h"
#include "MainDlg.h"
#include "skin/SSkinLoader.h"
#include "skin/SetSkinWnd2.h"

#define SKIN_CFG _T("/themes/skin_config.xml")

#define kLogTag "skindlg"

namespace
{
	//主题色槽位表:与 uires/values/color.xml 的现代蓝色系配色一一对应。
	//crBase 为出厂值(XML 解析期 ATTR_COLOR 经 GETCOLOR 固化到窗口样式,
	//格式 RGBA(r,g,b,255));crCur 记录当前已应用到控件树的值,换肤时用
	//它做树上精确匹配,从而支持在任意主题之间连续切换。
	struct ThemeSlot
	{
		LPCWSTR  pszName;
		COLORREF crBase;
		COLORREF crCur;
	};

	ThemeSlot s_themeSlots[] = {
		{ L"clr_window_bg",		RGBA(0xF1,0xF5,0xF9,0xFF), RGBA(0xF1,0xF5,0xF9,0xFF) },
		{ L"clr_card_bg",		RGBA(0xFF,0xFF,0xFF,0xFF), RGBA(0xFF,0xFF,0xFF,0xFF) },
		{ L"clr_card_hover",	RGBA(0xEF,0xF6,0xFF,0xFF), RGBA(0xEF,0xF6,0xFF,0xFF) },
		{ L"clr_primary",		RGBA(0x25,0x63,0xEB,0xFF), RGBA(0x25,0x63,0xEB,0xFF) },
		{ L"clr_primary_dark",	RGBA(0x1D,0x4E,0xD8,0xFF), RGBA(0x1D,0x4E,0xD8,0xFF) },
		{ L"clr_title",			RGBA(0x0F,0x17,0x2A,0xFF), RGBA(0x0F,0x17,0x2A,0xFF) },
		{ L"clr_text",			RGBA(0x33,0x41,0x55,0xFF), RGBA(0x33,0x41,0x55,0xFF) },
		{ L"clr_text_sub",		RGBA(0x64,0x74,0x8B,0xFF), RGBA(0x64,0x74,0x8B,0xFF) },
		{ L"clr_nav_bg",		RGBA(0xFF,0xFF,0xFF,0xFF), RGBA(0xFF,0xFF,0xFF,0xFF) },
		{ L"clr_divider",		RGBA(0xE2,0xE8,0xF0,0xFF), RGBA(0xE2,0xE8,0xF0,0xFF) },
	};
	const int THEME_SLOT_COUNT = sizeof(s_themeSlots) / sizeof(s_themeSlots[0]);
	const int THEME_SLOT_PRIMARY = 3;//clr_primary 在表中的下标,推导饱和度比例的基准

	//COLORREF(不透明) -> HSL(h:0-360, s/l:0-1)
	void Rgb2Hsl(COLORREF cr, float &h, float &s, float &l)
	{
		float r = GetRValue(cr) / 255.0f;
		float g = GetGValue(cr) / 255.0f;
		float b = GetBValue(cr) / 255.0f;
		float fMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
		float fMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
		l = (fMax + fMin) / 2.0f;
		float d = fMax - fMin;
		if (d < 1e-6f)
		{
			s = 0.0f;
			h = 0.0f;
			return;
		}
		s = l > 0.5f ? d / (2.0f - fMax - fMin) : d / (fMax + fMin);
		if (fMax == r)
			h = (g - b) / d + (g < b ? 6.0f : 0.0f);
		else if (fMax == g)
			h = (b - r) / d + 2.0f;
		else
			h = (r - g) / d + 4.0f;
		h *= 60.0f;
	}

	float Hue2Rgb(float p, float q, float t)
	{
		if (t < 0.0f) t += 1.0f;
		if (t > 1.0f) t -= 1.0f;
		if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
		if (t < 0.5f) return q;
		if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
		return p;
	}

	//HSL -> 不透明 COLORREF(高字节 alpha=0xFF)
	COLORREF Hsl2Rgb(float h, float s, float l)
	{
		float r, g, b;
		if (s <= 0.0f)
		{
			r = g = b = l;
		}
		else
		{
			float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
			float p = 2.0f * l - q;
			float hr = h / 360.0f;
			r = Hue2Rgb(p, q, hr + 1.0f / 3.0f);
			g = Hue2Rgb(p, q, hr);
			b = Hue2Rgb(p, q, hr - 1.0f / 3.0f);
		}
		return RGBA((BYTE)(r * 255.0f + 0.5f), (BYTE)(g * 255.0f + 0.5f), (BYTE)(b * 255.0f + 0.5f), 0xFF);
	}

	//由主题色推导整套界面色:各槽位保留自身明度,采用主题色色相,
	//饱和度按 主题色饱和度/出厂主色饱和度 缩放。
	void DeriveThemeColors(COLORREF crTheme, COLORREF crNew[])
	{
		float hTheme = 0.0f, sTheme = 0.0f, lTheme = 0.0f;
		Rgb2Hsl(crTheme, hTheme, sTheme, lTheme);

		float h = 0.0f, s = 0.0f, l = 0.0f;
		Rgb2Hsl(s_themeSlots[THEME_SLOT_PRIMARY].crBase, h, s, l);
		float fSatScale = s > 1e-6f ? sTheme / s : 0.0f;

		for (int i = 0; i < THEME_SLOT_COUNT; i++)
		{
			Rgb2Hsl(s_themeSlots[i].crBase, h, s, l);
			float fSat = s * fSatScale;
			if (fSat > 1.0f) fSat = 1.0f;
			crNew[i] = Hsl2Rgb(hTheme, fSat, l);
		}
	}

	//递归遍历控件树,把窗口样式色(colorBkgnd/colorBorder/colorText*)中
	//等于"当前已应用值"的替换为新值,返回发生替换的窗口数。
	int ApplyColorsToTree(SWindow *pWin, const COLORREF crNew[])
	{
		int nChanged = 0;
		SwndStyle &style = pWin->GetStyle();
		for (int i = 0; i < THEME_SLOT_COUNT; i++)
		{
			COLORREF crOld = s_themeSlots[i].crCur;
			COLORREF crNewVal = crNew[i];
			if (crOld == crNewVal)
				continue;
			bool bHit = false;
			for (int iState = 0; iState < 4; iState++)
			{
				if (style.GetTextColor(iState) == crOld)
				{
					style.SetTextColor(iState, crNewVal);
					bHit = true;
				}
			}
			if (style.m_crBg == crOld)
			{
				style.m_crBg = crNewVal;
				bHit = true;
			}
			if (style.m_crBorder == crOld)
			{
				style.m_crBorder = crNewVal;
				bHit = true;
			}
			if (bHit)
			{
				pWin->Invalidate();
				nChanged++;
			}
		}
		SWindow *pChild = pWin->GetWindow(GSW_FIRSTCHILD);
		while (pChild)
		{
			nChanged += ApplyColorsToTree(pChild, crNew);
			pChild = pChild->GetWindow(GSW_NEXTSIBLING);
		}
		return nChanged;
	}

	//换肤配色应用入口:由主题色推导新配色并整树替换;
	//crTheme 为 CR_INVALID(内置皮肤)时回退出厂配色并取消位图染色。
	void ApplyThemeColors(SWindow *pRoot, COLORREF crTheme)
	{
		if (!pRoot)
			return;
		COLORREF crNew[THEME_SLOT_COUNT];
		bool bFactory = (crTheme == CR_INVALID);
		if (bFactory)
		{
			for (int i = 0; i < THEME_SLOT_COUNT; i++)
				crNew[i] = s_themeSlots[i].crBase;
		}
		else
		{
			DeriveThemeColors(crTheme, crNew);
		}
		DWORD tm1 = GetTickCount();
		int nChanged = ApplyColorsToTree(pRoot, crNew);
		for (int i = 0; i < THEME_SLOT_COUNT; i++)
			s_themeSlots[i].crCur = crNew[i];
		//位图皮肤(背景/图标等)统一用 DoColorize 染色,出厂配色时取消染色
		pRoot->DoColorize(bFactory ? 0 : (crTheme | 0xff000000));
		SLOGI()<<"ApplyThemeColors theme="<<(bFactory?"builtin":"color")<<" changedWnd="<<nChanged<<" spend "<<GetTickCount()-tm1<<" ms";
	}
}

/**
* @brief      把换肤配置写入 XML 文件(供下次启动恢复)
* @param      skinType    皮肤类型(color/sys/builtin)
* @param      skinSaveInf 皮肤数据(纯色值或系统皮肤文件路径+边距)
*/
void SaveSkinInf2File(SkinType skinType, SkinSaveInf &skinSaveInf)
{
	SXmlDoc docSave;
	SXmlNode rootNode = docSave.root().append_child(L"DEMO_SKIN_CONFIG");
	SXmlNode childSkinType = rootNode.append_child(L"skinInf");
	childSkinType.append_attribute(L"type").set_value(skinType);
	SStringT strSkinConfigPath = SApplication::getSingleton().GetAppDir() + SKIN_CFG;
	switch (skinType)
	{
	case color://纯色只有SkinSaveInf的color有效
		childSkinType.append_attribute(L"color").set_value((int)skinSaveInf.color);
		break;
	case sys://此处为系统皮肤，只需要给文件路径和margin
		{
			childSkinType.append_attribute(L"skin_path").set_value(skinSaveInf.filepath.c_str());
			SStringW margin;
			margin.Format(L"%d,%d,%d,%d", skinSaveInf.margin.left, skinSaveInf.margin.top, skinSaveInf.margin.right, skinSaveInf.margin.bottom);
			childSkinType.append_attribute(L"skin_margin").set_value(margin);
		}
		break;
	case builtin:
	default:
		break;
	}
	docSave.save_file(strSkinConfigPath);
}

/**
* @brief      ISetOrLoadSkinHandler 回调:换肤窗口确定新皮肤后保存配置
*/
bool CMainDlg::SaveSkin(SkinType skinType, SkinSaveInf &skinSaveInf)
{
	HRESULT hr = S_OK;
	SaveSkinInf2File(skinType, skinSaveInf);
	return hr == S_OK;
}

/**
* @brief      从 XML 配置读取上一次保存的皮肤参数
* @param      skin      目标皮肤对象
* @param      skinType  [out] 读出的皮肤类型
* @param      skininf   [out] 读出的皮肤数据
*/
void LoadSkinFormXml(SDemoSkin *skin, SkinType *skinType, SkinLoadInf *skininf)
{
	SStringT strSkinConfigPath = SApplication::getSingleton().GetAppDir() + SKIN_CFG;

	SXmlDoc docLoad;
	bool bLoad = docLoad.load_file(strSkinConfigPath);
	if (bLoad)
	{
		SXmlNode skinInf = docLoad.root().child(L"DEMO_SKIN_CONFIG").child(L"skinInf");
		*skinType = (SkinType)skinInf.attribute(L"type").as_int();
		switch (*skinType)
		{
			//纯色只有SkinSaveInf的color有效
		case color:
			skininf->color = skinInf.attribute(L"color").as_int();
			break;
			//此处为系统皮肤，只需要给文件路径和margin
		case sys:
			skininf->filepath = skinInf.attribute(L"skin_path").as_string();
			int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
			swscanf(skinInf.attribute(L"skin_margin").as_string(), L"%d,%d,%d,%d", &v1, &v2, &v3, &v4);
			skininf->margin.left = v1;
			skininf->margin.top = v2;
			skininf->margin.right = v3;
			skininf->margin.bottom = v4;
			break;
		}
	}
}

/**
* @brief      启动时恢复上一次保存的皮肤
* @return     是否成功应用皮肤
*/
bool CMainDlg::LoadSkin()
{
	SDemoSkin *skin = (SDemoSkin *)GETSKIN(L"demoskinbk",100);
	if (skin)
	{
		SkinLoadInf loadInf = {0};
		SkinType type = builtin;
		LoadSkinFormXml(skin, &type, &loadInf);
		skin->SetHander(this);
		bool bRet = skin->LoadSkin(type, loadInf);
		if (bRet)
		{
			//启动时按恢复的皮肤同步整树配色(主题色推导 + 树上替换)
			ApplyThemeColors(GetRoot(), skin->GetThemeColor());
		}
		return bRet;
	}
	return false;
}

/**
* @brief      主题色变化消息(g_dwSkinChangeMessage)处理
*
* Describe    换肤窗口(SetSkinWnd2)选定新主题色后广播该消息,主窗口响应:
*               1. 刷新 9527 皮肤背景载体(cache=1,需显式 Invalidate);
*               2. 取 demoskinbk 的主题色,推导整套界面配色并整树替换,
*                  主题色无效(CR_INVALID,内置皮肤)时回退出厂配色;
*               3. 位图皮肤统一通过 DoColorize 染色(见 ApplyThemeColors)。
*/
HRESULT CMainDlg::OnSkinChangeMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled)
{
	SWindow *pBg = FindChildByID(9527);
	if(pBg)
		pBg->Invalidate();

	SDemoSkin *skin = (SDemoSkin *) GETSKIN(L"demoskinbk",100);
	COLORREF crTheme = skin ? skin->GetThemeColor() : CR_INVALID;
	ApplyThemeColors(GetRoot(), crTheme);
	return S_OK;
}

/**
* @brief      标题栏"换肤"按钮:打开换肤设置对话框
*
* Describe    CSetSkinWnd2 内置三种换肤方式(内置图片/纯色/系统图片),
*             确定后通过 ISetOrLoadSkinHandler 回调(SaveSkin)持久化,
*             并广播主题色变化消息。
*/
void CMainDlg::OnBtnSkin()
{
	CSetSkinWnd pSetSkinWnd;
	pSetSkinWnd.DoModal(m_hWnd);
}

#include "stdafx.h"
#include "SetSkinWnd2.h"
#include "SDemoSkin.h"
#include <winuser.h>
#include "SSkinLoader.h"
#include <helper/SHostMgr.h>

#define SKIN_CHANGE_MSG _T("{D17D208B-25FD-412C-8071-68816D4B1F9B}")
//注册皮肤改变消息
UINT g_dwSkinChangeMessage = RegisterWindowMessage(SKIN_CHANGE_MSG);

HRESULT CSetSkinWnd::OnSkinChangeMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled)
{
	FindChildByID(9527)->Invalidate();
	return S_OK;
}

long CSetSkinWnd::NotifUpdataWindow()
{
    SHostMgr::getSingletonPtr()->DispatchMessage(g_dwSkinChangeMessage, MagicNumber, MagicNumber);
    return 0;
}

void CSetSkinWnd::LoadSkinConfigFormXml()
{
#ifdef _WIN32
    SStringT strSkinConfigPath = SApplication::getSingleton().GetAppDir() + _T("\\themes\\themes_config.xml");
#else
    SStringT strSkinConfigPath = SApplication::getSingleton().GetAppDir() + _T("/themes/themes_config.xml");
#endif //_WIN32
    SXmlDoc docLoad;
    bool bLoad = docLoad.load_file(strSkinConfigPath);
    if (bLoad)
    {
        SXmlNode skinInf = docLoad.root().child(L"DEMO_SKIN_CONFIG").child(L"skinInf");
        while (skinInf)
        {
            SKIN_CONFIG_INF inf;
            inf.id = skinInf.attribute(L"id").as_int();
            int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
            swscanf(skinInf.attribute(L"skin_margin").as_string(), L"%d,%d,%d,%d", &v1, &v2, &v3, &v4);
            inf.margin.left = v1;
            inf.margin.top = v2;
            inf.margin.right = v3;
            inf.margin.bottom = v4;
            m_skinConfigInf.AddTail(inf);
            skinInf = skinInf.next_sibling();
        }
    }
}

CSetSkinWnd::CSetSkinWnd() :SHostDialog(_T("LAYOUT:dlg_set_skin"))
{	
	LoadSkinConfigFormXml();
}

CSetSkinWnd::~CSetSkinWnd()
{
}

CRect CSetSkinWnd::GetMargin(int id)
{
	SPOSITION headPos= m_skinConfigInf.GetHeadPosition();
	while (headPos)
	{
		SKIN_CONFIG_INF inf= m_skinConfigInf.GetNext(headPos);
		if (inf.id == id)
		{
			return inf.margin;
		}
	}
	return CRect();
}

void CSetSkinWnd::OnSetSkin(IEvtArgs * e)
{
	SWindow *sender = (SWindow*) e->Sender();
	int nIndex = sender->GetID();
	SDemoSkin *skin = (SDemoSkin *) GETSKIN(L"demoskinbk",GetScale());
	SStringT strSkinFile;
	SStringT strLoadSkin;
	#ifdef _WIN32
	strSkinFile.Format(_T("%s\\themes\\%d.png"), SApplication::getSingleton().GetAppDir().c_str(),nIndex - 9);
	strLoadSkin.Format(_T("themes\\skin%d"), ((nIndex - 9)%3)+1);
	#else
	strSkinFile.Format(_T("%s/themes/%d.png"), SApplication::getSingleton().GetAppDir().c_str(),nIndex - 9);
	strLoadSkin.Format(_T("themes/skin%d"), ((nIndex - 9)%3)+1);
	#endif//_WIN32

	SSkinLoader::getSingleton().LoadSkin(strLoadSkin);

	DWORD dwAttr = GetFileAttributes(strSkinFile);
    if (dwAttr == INVALID_FILE_ATTRIBUTES)
    {
		SMessageBox(0, _T("无法设置当前主题,找不到系统主题文件。复制demo\\themes\\文件夹到soui根目录!"), _T("警告"), MB_OK);
		return;
	}
	if (skin)
	{
		skin->SetImage(S_CT2W(strSkinFile));
		skin->SetMargin(GetMargin(nIndex-9));
		NotifUpdataWindow();
	}
}


void CSetSkinWnd::OnColor(IEvtArgs * e)
{
	SWindow *sender = (SWindow*)e->Sender();
	SDemoSkin *skin = (SDemoSkin *)GETSKIN(L"demoskinbk", GetScale());
	if (skin)
	{
		skin->SetColor(sender->GetStyle().m_crBg);
		NotifUpdataWindow();
	}
}

void CSetSkinWnd::OnBuiltinSkin()
{
	SSkinLoader::getSingleton().LoadDefSkin();
	SDemoSkin *skin = (SDemoSkin *)GETSKIN(L"demoskinbk", GetScale());
	if (skin)
	{
		skin->ClearSkin();
		NotifUpdataWindow();
	}

}

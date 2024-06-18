#include "stdafx.h"
#include "SSkinLoader.h"
#include "res.mgr\SResProvider.h"
#include <SouiFactory.h>

template<>
SSkinLoader * SSingleton<SSkinLoader>::ms_Singleton = NULL;

SSkinLoader::SSkinLoader(SApplication* theApp):m_pResProvider(NULL),m_theApp(theApp)
{
	m_privateSkinPool.Attach(SUiDef::CreateSkinPool(TRUE));
	GETUIDEF->PushSkinPool(m_privateSkinPool);
}


SSkinLoader::~SSkinLoader()
{
	GETUIDEF->PopSkinPool(m_privateSkinPool);
}

void SOUI::SSkinLoader::LoadSkinFormZip(SStringT respath, const TCHAR *strXmlSkin/*=_T("SkinXml:LoadSkinXml")*/)
{
	//if (m_pResProvider == NULL)
	//{
	//	if(CreateResProvider_ZIP((IObjRef**)&m_pResProvider))
	//		m_theApp->AddResProvider(m_pResProvider,NULL);
	//}
	//SASSERT(m_pResProvider);
	//ZIPRES_PARAM param;
	//param.ZipFile(m_theApp->GetRenderFactory(), respath, "www.bukengnikengshui.com");
	//if (!m_pResProvider->Init((WPARAM)&param, 0))
	//{
	//	SASSERT(0);
	//}
	//m_privateSkinPool->RemoveAll();

	//SXmlDoc xmlDoc;
	//if (LOADXML(xmlDoc, strXmlSkin))
	//{
	//	m_privateSkinPool->LoadSkins(xmlDoc.root().child(L"skin"));
	//}
}

void SOUI::SSkinLoader::LoadSkin(SStringT respath,const TCHAR *strXmlSkin /*= _T("SkinXml:LoadSkinXml")*/)
{
	if (m_pResProvider == NULL)
	{
		SouiFactory souiFac;
		m_pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
		m_theApp->AddResProvider(m_pResProvider, NULL);
	}
	SASSERT(m_pResProvider);
	if (!m_pResProvider->Init((WPARAM)respath.GetBuffer(0), NULL))
		return;
	m_privateSkinPool->RemoveAll();
	SXmlDoc xmlDoc;
	if (LOADXML(xmlDoc,strXmlSkin))
	{
		SXmlNode xmlSkin = xmlDoc.root().child(L"skin");
		m_privateSkinPool->LoadSkins(&xmlSkin);
	}
}

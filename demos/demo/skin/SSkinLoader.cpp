#include "stdafx.h"
#include "SSkinLoader.h"
#include "res.mgr/SResProvider.h"
#include <SouiFactory.h>

SNSBEGIN
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

void SSkinLoader::LoadSkin(SStringT respath,const TCHAR *strXmlSkin /*= _T("SkinXml:LoadSkinXml")*/)
{
	if (m_pResProvider == NULL)
	{
		SouiFactory souiFac;
		m_pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
		m_theApp->AddResProvider(m_pResProvider, NULL);
	}
	SASSERT(m_pResProvider);
	if (!m_pResProvider->Init((WPARAM)respath.GetBuffer(0), 0))
		return;
	m_privateSkinPool->RemoveAll();
	SXmlDoc xmlDoc;
	if (LOADXML(xmlDoc,strXmlSkin))
	{
		SXmlNode xmlSkin = xmlDoc.root().child(L"skin");
		m_privateSkinPool->LoadSkins(&xmlSkin);
	}
}

void SSkinLoader::LoadDefSkin()
{
#ifdef _WIN32
		LoadSkin(_T("themes\\skin1"));
#else
		LoadSkin(_T("themes/skin1"));
#endif//_WIN32
}

SNSEND
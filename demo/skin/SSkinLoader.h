#ifndef _SKIN_LOADER_H_
#define _SKIN_LOADER_H_

#include <interface/SSkinPool-i.h>
#include <core/SSingleton.h>
SNSBEGIN

	class SSkinLoader:public SSingleton<SSkinLoader>
	{
		SAutoRefPtr<IResProvider> m_pResProvider; 
		SAutoRefPtr<ISkinPool>  m_privateSkinPool;
		SApplication *m_theApp;
	public:
		SSkinLoader(SApplication*);
		~SSkinLoader();
		void LoadSkin(SStringT respath,const TCHAR *strXmlSkin = _T("SkinXml:LoadSkinXml"));
		void LoadDefSkin();
	};

SNSEND

#endif//_SKIN_LOADER_H_
#pragma once

#include <interface/SSkinPool-i.h>
#include <core/SSingleton.h>

namespace SOUI
{
#define RT_SKIN _T("SkinXml")
	class SSkinLoader :public SSingleton<SSkinLoader>
	{
		SAutoRefPtr<IResProvider> m_pResProvider; 
		SAutoRefPtr<ISkinPool>  m_privateSkinPool;
		SApplication *m_theApp;
	public:
		SSkinLoader(SApplication*);
		~SSkinLoader();
		void LoadSkinFormZip(SStringT respath, const TCHAR *strXmlSkin=_T("SkinXml:LoadSkinXml"));
		void LoadSkin(SStringT respath,const TCHAR *strXmlSkin = _T("SkinXml:LoadSkinXml"));
	};
}//END SOUI

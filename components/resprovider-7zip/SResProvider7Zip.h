﻿#pragma once

#include <interface/SResProvider-i.h>
#include <helper/obj-ref-impl.hpp>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <souicoll.h>
#include <helper/SResID.h>
#include <interface/SRender-i.h>

#include "zip7resprovider-param.h"

namespace SevenZip
{
	class CZipArchive;
}
SNSBEGIN

class SResProvider7Zip : public TObjRefImpl<IResProvider>
{
public:
	SResProvider7Zip();
	~SResProvider7Zip(void);
	
	STDMETHOD_(BOOL,Init)(THIS_ WPARAM wParam,LPARAM lParam);
	STDMETHOD_(BOOL,HasResource)(THIS_ LPCTSTR pszType,LPCTSTR pszResName);
	STDMETHOD_(HICON,LoadIcon)(THIS_ LPCTSTR pszResName,int cx,int cy);
	STDMETHOD_(HBITMAP,LoadBitmap)(THIS_ LPCTSTR pszResName);
	STDMETHOD_(HCURSOR,LoadCursor)(THIS_ LPCTSTR pszResName);
	STDMETHOD_(IBitmapS*, LoadImage)(THIS_ LPCTSTR pszType,LPCTSTR pszResName);
	STDMETHOD_(IImgX*, LoadImgX)(THIS_ LPCTSTR pszType,LPCTSTR pszResName);
	STDMETHOD_(size_t,GetRawBufferSize)(THIS_ LPCTSTR pszType,LPCTSTR pszResName);
	STDMETHOD_(BOOL,GetRawBuffer)(THIS_ LPCTSTR pszType,LPCTSTR pszResName,LPVOID pBuf,size_t size);
	STDMETHOD_(void,EnumResource)(THIS_ EnumResCallback funEnumCB,LPARAM lp);
	STDMETHOD_(void, EnumFile)(THIS_ EnumFileCallback funEnumCB, LPARAM lp);
protected:
    BOOL _Init(LPCTSTR pszZipFile ,LPCSTR pszPsw);
    BOOL _Init(HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszType ,LPCSTR pszPsw);
	BOOL _InitFileMap();
	SStringT _GetFilePath(LPCTSTR pszResName,LPCTSTR pszType);
	
	SMap<SResID,SStringT> m_mapFiles;
    SAutoRefPtr<IRenderFactory> m_renderFactory;
    SevenZip::CZipArchive * m_zipFile;
	SStringT m_childDir;
};

namespace RESPROVIDER_7ZIP
{
    SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef ** ppObj);
}

SNSEND

EXTERN_C BOOL SOUI_COM_API Resp_7zip_SCreateInstance(IObjRef ** ppObj);

#include "stdafx.h"
#pragma warning(disable:4251)

#include "SResProvider7Zip.h"
#include <xml/SXml.h>


using namespace SevenZip;
SNSBEGIN

SResProvider7Zip::SResProvider7Zip():m_renderFactory(NULL)
{
}

SResProvider7Zip::~SResProvider7Zip(void)
{
}

HBITMAP SResProvider7Zip::LoadBitmap(LPCTSTR pszResName )
{
	SStringT strPath=_GetFilePath(pszResName,_T("BITMAP"));
	if(strPath.IsEmpty()) return NULL;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) return NULL;

	HDC hDC = GetDC(NULL);
	//读取位图头
	BITMAPFILEHEADER *pBmpFileHeader=(BITMAPFILEHEADER *)zf.GetData(); 
	//检测位图头
	if (pBmpFileHeader->bfType != ((WORD) ('M'<<8)|'B')) 
	{
		return NULL; 
	} 
	//判断位图长度
	if (pBmpFileHeader->bfSize > (UINT)zf.GetSize()) 
	{ 
		return NULL; 
	} 
	LPBITMAPINFO lpBitmap=(LPBITMAPINFO)(pBmpFileHeader+1); 
	LPVOID lpBits=(LPBYTE)zf.GetData()+pBmpFileHeader->bfOffBits;
	HBITMAP hBitmap= CreateDIBitmap(hDC,&lpBitmap->bmiHeader,CBM_INIT,lpBits,lpBitmap,DIB_RGB_COLORS);
	ReleaseDC(NULL,hDC);

	return hBitmap;
}

HICON SResProvider7Zip::LoadIcon(LPCTSTR pszResName ,int cx/*=0*/,int cy/*=0*/)
{
	SStringT strPath=_GetFilePath(pszResName,_T("ICON"));
	if(strPath.IsEmpty()) return NULL;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) return NULL;

	return (HICON)LoadIconFromMemory(zf.GetData(),zf.GetSize(),TRUE,cx,cy,LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
}

HCURSOR SResProvider7Zip::LoadCursor( LPCTSTR pszResName )
{
	SStringT strPath=_GetFilePath(pszResName,_T("CURSOR"));
	if(strPath.IsEmpty()) return NULL;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) return NULL;
	return (HCURSOR)LoadIconFromMemory(zf.GetData(),zf.GetSize(),FALSE,0,0,LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
}

IBitmapS * SResProvider7Zip::LoadImage( LPCTSTR strType,LPCTSTR pszResName)
{
	if(!m_renderFactory)
		return NULL;
	SStringT strPath=_GetFilePath(pszResName,strType);
	if(strPath.IsEmpty()) return NULL;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) return NULL;
	IBitmapS * pBmp=NULL;
	m_renderFactory->CreateBitmap(&pBmp);
	if(!pBmp) return NULL;
	pBmp->LoadFromMemory(zf.GetData(),zf.GetSize());
	return pBmp;
}

IImgX   * SResProvider7Zip::LoadImgX( LPCTSTR strType,LPCTSTR pszResName )
{
	if(!m_renderFactory)
		return NULL;

	SStringT strPath=_GetFilePath(pszResName,strType);
	if(strPath.IsEmpty()) return NULL;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) return NULL;

	IImgX *pImgX=NULL;
	m_renderFactory->GetImgDecoderFactory()->CreateImgX(&pImgX);
	if(!pImgX) return NULL;

	if(0==pImgX->LoadFromMemory(zf.GetData(),zf.GetSize()))
	{
		pImgX->Release();
		pImgX=NULL;
	}
	return pImgX;
}

BOOL SResProvider7Zip::_Init( LPCTSTR pszZipFile ,LPCSTR pszPsw)
{
	if (!m_zipFile.Open(pszZipFile, pszPsw)) return FALSE;
	_InitFileMap();
	return TRUE;
}

BOOL SResProvider7Zip::_Init( HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszType  ,LPCSTR pszPsw)
{
	if(!m_zipFile.Open(hInst,pszResName,pszPsw,pszType)) return FALSE;
	_InitFileMap();
	return TRUE;
}

BOOL SResProvider7Zip::Init( WPARAM wParam,LPARAM lParam )
{
	ZIP7RES_PARAM *zipParam = (ZIP7RES_PARAM*)wParam;
	m_renderFactory = zipParam->pRenderFac;
	m_childDir = zipParam->pszChildDir;
	if (!m_childDir.IsEmpty())
	{
		m_childDir.TrimRight(_T('\\'));
		m_childDir.TrimRight(_T('/'));
		m_childDir += _T("\\");
	}
	if (zipParam->type == ZIP7_FILE)
		return _Init(zipParam->pszZipFile,zipParam->pszPsw);
	else
		return _Init(zipParam->peInfo.hInst,zipParam->peInfo.pszResName,zipParam->peInfo.pszResType,zipParam->pszPsw);
}

SStringT SResProvider7Zip::_GetFilePath( LPCTSTR pszResName,LPCTSTR pszType )
{
	if(!pszType){
		//pszResName is a path
		if(!m_zipFile.IsFileExist(pszResName))
			return _T("");
		return pszResName;
	}
	SResID resID(pszType,pszResName);
	SMap<SResID,SStringT>::CPair *p = m_mapFiles.Lookup(resID);
	if(!p) return _T("");
	return p->m_value;
}

size_t SResProvider7Zip::GetRawBufferSize( LPCTSTR strType,LPCTSTR pszResName )
{
	SStringT strPath=_GetFilePath(pszResName,strType);
	if(strPath.IsEmpty()) return FALSE;

	return m_zipFile.GetFileSize(strPath);
}

BOOL SResProvider7Zip::GetRawBuffer( LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size )
{
	SStringT strPath=_GetFilePath(pszResName,strType);
	if(strPath.IsEmpty()) return FALSE;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) 
		return FALSE;
	if(size<zf.GetSize())
	{
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return FALSE;
	}
	memcpy(pBuf,zf.GetData(),zf.GetSize());
	return TRUE;
}

void SResProvider7Zip::EnumResource(EnumResCallback funEnumCB,LPARAM lp)
{
	SPOSITION pos = m_mapFiles.GetStartPosition();
	while(pos)
	{
		const SResID &id = m_mapFiles.GetNextKey(pos);
		if(!funEnumCB(id.szType,id.szName,lp))
			break;
	}
}

BOOL SResProvider7Zip::HasResource( LPCTSTR strType,LPCTSTR pszResName )
{
	SStringT strPath = _GetFilePath(pszResName,strType);
	return !strPath.IsEmpty();
}

BOOL SResProvider7Zip::_InitFileMap()
{
	CZipFile zf;
	BOOL bIdx=m_zipFile.GetFile(m_childDir + UIRES_INDEX,zf);
	if(!bIdx) return FALSE;

	SXmlDoc xmlDoc;
	SStringA strFileName;
	if(!xmlDoc.load_buffer_inplace(zf.GetData(),zf.GetSize(),xml_parse_default,enc_utf8))
		return FALSE;
	SXmlNode xmlElem=xmlDoc.root().child(L"resource");
	if(!xmlElem) return FALSE;
	SXmlNode resType=xmlElem.first_child();
	while(resType)
	{
		SXmlNode resFile=resType.first_child();
		while(resFile)
		{
			SResID id(S_CW2T(resType.name()),S_CW2T(resFile.attribute(L"name").value()));
			m_mapFiles[id] = m_childDir + S_CW2T(resFile.attribute(L"path").value());
			resFile=resFile.next_sibling();
		}
		resType = resType.next_sibling();
	}
	return TRUE;
}

void SResProvider7Zip::EnumFile(THIS_ EnumFileCallback funEnumCB, LPARAM lp)
{
	unsigned int pos = m_zipFile.GetFirstFilePos();
	while(!m_zipFile.Eof(pos)){
		std::string name = m_zipFile.GetFileName(pos);
		SStringT str = S_CA2T(name.c_str());
		if(!funEnumCB(str.c_str(),lp))
			break;
		pos = m_zipFile.GetNextFilePos(pos);
	}
}

namespace RESPROVIDER_7ZIP
{
	BOOL SCreateInstance( IObjRef ** ppObj )
	{
		*ppObj = new SResProvider7Zip;
		return TRUE;
	}
}
SNSEND


EXTERN_C BOOL Resp_7zip_SCreateInstance(IObjRef ** ppObj)
{
	return SOUI::RESPROVIDER_7ZIP::SCreateInstance(ppObj);
}


#include "stdafx.h"
#pragma warning(disable:4251)

#include "SResProviderZip.h"
#include <xml/SXml.h>

SNSBEGIN

SResProviderZip::SResProviderZip():m_renderFactory(NULL)
{
}

SResProviderZip::~SResProviderZip(void)
{
}

HBITMAP SResProviderZip::LoadBitmap(LPCTSTR pszResName )
{
	SStringT strPath=_GetFilePath(pszResName,_T("BITMAP"));
	if(strPath.IsEmpty()) return NULL;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) return NULL;

	HDC hDC = GetDC(0);
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
	ReleaseDC(0,hDC);

	return hBitmap;
}

HICON SResProviderZip::LoadIcon(LPCTSTR pszResName ,int cx/*=0*/,int cy/*=0*/)
{
	SStringT strPath=_GetFilePath(pszResName,_T("ICON"));
	if(strPath.IsEmpty()) return NULL;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) return NULL;

	return (HICON)LoadIconFromMemory(zf.GetData(),zf.GetSize(),TRUE,cx,cy,LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
}

HCURSOR SResProviderZip::LoadCursor( LPCTSTR pszResName )
{
	SStringT strPath=_GetFilePath(pszResName,_T("CURSOR"));
	if(strPath.IsEmpty()) return NULL;
	CZipFile zf;
	if(!m_zipFile.GetFile(strPath,zf)) return NULL;
	return (HCURSOR)LoadIconFromMemory(zf.GetData(),zf.GetSize(),FALSE,0,0,LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
}

IBitmapS * SResProviderZip::LoadImage( LPCTSTR strType,LPCTSTR pszResName)
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

IImgX   * SResProviderZip::LoadImgX( LPCTSTR strType,LPCTSTR pszResName )
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

void SResProviderZip::EnumResource(EnumResCallback funEnumCB,LPARAM lp)
{
	SPOSITION pos = m_mapFiles.GetStartPosition();
	while(pos){
		const SResID & resId = m_mapFiles.GetNextKey(pos);
		if(!funEnumCB(resId.szType,resId.szName,lp))
			break;
	}
}


BOOL SResProviderZip::_Init( LPCTSTR pszZipFile ,LPCSTR pszPsw)
{
	if(!m_zipFile.Open(pszZipFile)) return FALSE;
	m_zipFile.SetPassword(pszPsw);
	_InitFileMap();
	return TRUE;
}

BOOL SResProviderZip::_Init( LPBYTE pBytes, DWORD dwByteCounts, LPCSTR pszPsw ) 
{
	if(!m_zipFile.Open(pBytes, dwByteCounts)) return FALSE;
	m_zipFile.SetPassword(pszPsw);
	_InitFileMap();
	return TRUE;
}

BOOL SResProviderZip::_Init( HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszType, LPCSTR pszPsw)
{
	if(!m_zipFile.Open(hInst,pszResName,pszType)) return FALSE;
	m_zipFile.SetPassword(pszPsw);
	_InitFileMap();
	return TRUE;
}

BOOL SResProviderZip::Init( WPARAM wParam,LPARAM lParam )
{
	ZIPRES_PARAM *zipParam=(ZIPRES_PARAM*)wParam;
	m_renderFactory = zipParam->pRenderFac;
	m_childDir = zipParam->pszChildDir;

	if (!m_childDir.IsEmpty())
	{
		m_childDir.TrimRight(_T('\\'));
		m_childDir.TrimRight(_T('/'));
		m_childDir += _T("\\");
	}

	BOOL bResult = FALSE;
	switch (zipParam->type) 
	{
	case ZIPFILE:
		bResult = _Init(zipParam->pszZipFile, zipParam->pszPsw);
		break;
	case PEDATA:
		bResult =_Init(zipParam->peInfo.hInst,
			zipParam->peInfo.pszResName,
			zipParam->peInfo.pszResType,
			zipParam->pszPsw);
		break;
	case MEMORYDATA:
		bResult =_Init(zipParam->Memory.pByteBuffer,
			zipParam->Memory.dwByteCounts,
			zipParam->pszPsw);
		break;
	}

	return bResult;
}

SStringT SResProviderZip::_GetFilePath( LPCTSTR pszResName,LPCTSTR pszType )
{
	if(!pszType)
	{//no type, enum files
		int idx = m_zipFile.GetFileIndex(pszResName);
		if(idx!=-1) 
			return pszResName;
		else
			return _T("");
	}
	SResID resID(pszType,pszResName);
	SMap<SResID,SStringT>::CPair *p = m_mapFiles.Lookup(resID);
	if(!p) return _T("");
	return p->m_value;
}

size_t SResProviderZip::GetRawBufferSize( LPCTSTR strType,LPCTSTR pszResName )
{
	SStringT strPath=_GetFilePath(pszResName,strType);
	if(strPath.IsEmpty()) return FALSE;
	ZIP_FIND_DATA zfd;
	HANDLE hf=m_zipFile.FindFirstFile(strPath,&zfd);
	if(INVALID_HANDLE_VALUE==hf) return 0;
	m_zipFile.FindClose(hf);
	return zfd.nFileSizeUncompressed;
}

BOOL SResProviderZip::GetRawBuffer( LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size )
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

BOOL SResProviderZip::HasResource( LPCTSTR strType,LPCTSTR pszResName )
{
	SStringT strPath = _GetFilePath(pszResName, strType);
	return !strPath.IsEmpty();
}

BOOL SResProviderZip::_InitFileMap()
{
	CZipFile zf;
	BOOL bIdx=m_zipFile.GetFile(m_childDir+UIRES_INDEX,zf);
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

void SResProviderZip::EnumFile(THIS_ EnumFileCallback funEnumCB, LPARAM lp)
{
	_EnumFile(NULL,funEnumCB,lp);
}

void SResProviderZip::_EnumFile(LPCTSTR pszPath,EnumFileCallback funEnumCB, LPARAM lp)
{
	ZIP_FIND_DATA wfd;
	SStringT strFilter;
	if(pszPath)
		strFilter = SStringT(pszPath)+_T("\\*.*");
	else
		strFilter = _T("*.*");
	HANDLE hFind= m_zipFile.FindFirstFile(strFilter.c_str(),&wfd);
	if(hFind!=INVALID_HANDLE_VALUE){
		do{
			SStringT strPath;
			if(pszPath==NULL)
				strPath = wfd.szFileName;
			else
				strPath = SStringT().Format(_T("%s\\%s"),pszPath,wfd.szFileName);
			if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
				if(!funEnumCB(strPath.c_str(),lp))
					break;
			}
		}while(m_zipFile.FindNextFile(hFind,&wfd));
		m_zipFile.FindClose(hFind);
	}
}


namespace RESPROVIDER_ZIP
{
	BOOL SCreateInstance( IObjRef ** ppObj )
	{
		*ppObj = new SResProviderZip;
		return TRUE;
	}
}


SNSEND

EXTERN_C BOOL Resp_Zip_SCreateInstance(IObjRef ** ppObj)
{
	return SOUI::RESPROVIDER_ZIP::SCreateInstance(ppObj);
}

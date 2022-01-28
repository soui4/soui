#pragma once

SNSBEGIN

	struct IRenderFactory;
	typedef enum _ZipType{ ZIPFILE, PEDATA, MEMORYDATA } ZipType;
	struct ZIPRES_PARAM
	{
		ZipType type;
		IRenderFactory *pRenderFac;
		union {
			LPCTSTR pszZipFile;
			struct {
				HINSTANCE hInst;
				LPCTSTR pszResName;
				LPCTSTR pszResType;
			} peInfo;
			struct {
				LPBYTE  pByteBuffer;
				DWORD   dwByteCounts;				
			} Memory;
		};
		LPCSTR          pszPsw; //ZIP密码
		LPCTSTR			pszChildDir;
	};

	inline void ZipFile(ZIPRES_PARAM *param,IRenderFactory *_pRenderFac, LPCTSTR _pszFile, LPCSTR _pszPsw = NULL, LPCTSTR _pszChildDir = NULL)
	{
		param->type = ZIPFILE;
		param->pszZipFile = _pszFile;
		param->pszChildDir = _pszChildDir;
		param->pRenderFac = _pRenderFac;
		param->pszPsw = _pszPsw;
	}
	inline void ZipResource(ZIPRES_PARAM *param,IRenderFactory *_pRenderFac, HINSTANCE hInst, LPCTSTR pszResName, LPCTSTR pszResType = _T("zip"), LPCSTR _pszPsw = NULL, LPCTSTR _pszChildDir = NULL)
	{
		param->type = PEDATA;
		param->pRenderFac = _pRenderFac;
		param->pszChildDir = _pszChildDir;
		param->peInfo.hInst = hInst;
		param->peInfo.pszResName = pszResName;
		param->peInfo.pszResType = pszResType;
		param->pszPsw = _pszPsw;
	}
	inline void ZipMemory(ZIPRES_PARAM *param,IRenderFactory *_pRenderFac, LPBYTE  pByteBuffer, DWORD dwByteCounts, LPCSTR _pszPsw = NULL, LPCTSTR _pszChildDir = NULL)
	{
		param->type = MEMORYDATA;
		param->Memory.pByteBuffer = pByteBuffer;
		param->Memory.dwByteCounts = dwByteCounts;
		param->pszChildDir = _pszChildDir;
		param->pRenderFac = _pRenderFac;
		param->pszPsw = _pszPsw;			
	}
SNSEND
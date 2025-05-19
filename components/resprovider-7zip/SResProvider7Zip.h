#pragma once

#include <interface/SResProvider-i.h>
#include <helper/obj-ref-impl.hpp>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <souicoll.h>
//#define _COLL_NS SOUI
#include <helper/SResID.h>
#include <interface/SRender-i.h>

#ifndef _WIN32
#define VT_FILETIME VT_FILETIME_R
#define VariantClear VariantClear_R
#define SysAllocStringLen SysAllocStringLen_R
#define SysStringLen SysStringLen_R
#define GetLastError GetLastError_R
#define CompareFileTime CompareFileTime_R
#define DWORD_PTR DWORD_PTR_R
#define GUID GUID_R
#define CLSID CLSID_R
#define IID_IUnknown IID_IUnknown_R
#define LARGE_INTEGER LARGE_INTEGER_R
#define ULARGE_INTEGER ULARGE_INTEGER_R
#define TCHAR TCHAR_R
#define FILETIME FILETIME_R
#define IUnknown IUnknown_R
#define LPUNKNOWN LPUNKNOWN_R
#define VARENUM VARENUM_R
#define tagVARIANT tagVARIANT_R
#define VARIANT VARIANT_R
#define VARIANTARG VARIANTARG_R
#define VariantCopy VariantCopy_R
#define SysAllocStringByteLen SysAllocStringByteLen_R
#define SysAllocStringByteLen SysAllocStringByteLen_R
#define SysAllocString SysAllocString_R
#define SysFreeString SysFreeString_R
#define SysStringByteLen SysStringByteLen_R
#define SetLastError SetLastError_R
#define GetCurrentThreadId GetCurrentThreadId_R
#define STREAM_SEEK_SET STREAM_SEEK_SET_R
#define STREAM_SEEK_CUR STREAM_SEEK_CUR_R
#define STREAM_SEEK_END STREAM_SEEK_END_R
#define tagSTREAM_SEEK tagSTREAM_SEEK_R
#define STREAM_SEEK STREAM_SEEK_R
#define SYSTEMTIME SYSTEMTIME_R

#define VT_HRESULT VT_HRESULT_R
#define VT_VOID VT_VOID_R
#define VT_UINT VT_UINT_R
#define VT_INT VT_INT_R
#define VT_UI8 VT_UI8_R
#define UVT_UI8 UVT_UI8_R
#define VT_I8 VT_I8_R
#define VT_UI4 VT_UI4_R
#define VT_UI2 VT_UI2_R
#define VT_UI1 VT_UI1_R
#define VT_I1 VT_I1_R
#define VT_DECIMAL VT_DECIMAL_R
#define VT_UNKNOWN VT_UNKNOWN_R
#define VT_UNKNOWN VT_UNKNOWN_R
#define VT_VARIANT VT_VARIANT_R
#define VT_BOOL VT_BOOL_R
#define VT_ERROR VT_ERROR_R
#define VT_DISPATCH VT_DISPATCH_R
#define VT_BSTR VT_BSTR_R
#define VT_DATE VT_DATE_R
#define VT_CY VT_CY_R
#define VT_R8 VT_R8_R
#define VT_R4 VT_R4_R
#define VT_I4 VT_I4_R
#define VT_I2 VT_I2_R
#define VT_NULL VT_NULL_R
#define VT_EMPTY VT_EMPTY_R
#define DWORD DWORD_R
#define LPCWSTR LPCWSTR_R

#endif // !_WIN32


#include "Zip7Archive.h"
#include "zip7resprovider-param.h"

#ifndef _WIN32
#undef VT_FILETIME
#undef VariantClear
#undef SysAllocStringLen
#undef SysStringLen
#undef GetLastError
#undef CompareFileTime
#undef DWORD_PTR
#undef GUID
#undef CLSID
#undef IID_IUnknown
#undef LARGE_INTEGER
#undef ULARGE_INTEGER
#undef TCHAR
#undef FILETIME
#undef IUnknown
#undef LPUNKNOWN
#undef VARENUM
#undef tagVARIANT
#undef VARIANTARG
#undef VARIANT
#undef VariantCopy
#undef SysAllocStringByteLen
#undef SysAllocStringByteLen
#undef SysAllocString
#undef SysFreeString
#undef SysStringByteLen
#undef SetLastError
#undef GetCurrentThreadId
#undef STREAM_SEEK_SET
#undef STREAM_SEEK_CUR
#undef STREAM_SEEK_END
#undef tagSTREAM_SEEK
#undef STREAM_SEEK
#undef SYSTEMTIME
#undef VT_HRESULT
#undef VT_VOID
#undef VT_UINT
#undef VT_INT
#undef VT_UI8
#undef UVT_UI8
#undef VT_I8
#undef VT_UI4
#undef VT_UI2
#undef VT_UI1
#undef VT_I1
#undef VT_DECIMAL
#undef VT_UNKNOWN
#undef VT_UNKNOWN
#undef VT_VARIANT
#undef VT_BOOL
#undef VT_ERROR
#undef VT_DISPATCH
#undef VT_BSTR
#undef VT_DATE
#undef VT_CY
#undef VT_R8
#undef VT_R4
#undef VT_I4
#undef VT_I2
#undef VT_NULL
#undef VT_EMPTY
#undef DWORD
#undef LPCWSTR

#endif // !_WIN32

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
	SevenZip::CZipArchive m_zipFile;
	SStringT m_childDir;
};

namespace RESPROVIDER_7ZIP
{
    SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef ** ppObj);
}

SNSEND

EXTERN_C BOOL SOUI_COM_API Resp_7zip_SCreateInstance(IObjRef ** ppObj);

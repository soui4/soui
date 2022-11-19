#pragma once
#include <interface/SResProvider-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IResProviderMgr
DECLARE_INTERFACE(IResProviderMgr)
{
    STDMETHOD_(void, AddResProvider)(THIS_ IResProvider * pResProvider, LPCTSTR pszUidef DEF_VAL(_T("uidef:xml_init"))) PURE;

	STDMETHOD_(void, AddResProviderA)(THIS_ IResProvider * pResProvider, LPCSTR pszUidef DEF_VAL("uidef:xml_init")) PURE;

    STDMETHOD_(void, RemoveResProvider)(THIS_ IResProvider * pResProvider) PURE;

    STDMETHOD_(void, RemoveAll)(THIS) PURE;

    STDMETHOD_(IResProvider *, GetHeadResProvider)(CTHIS) SCONST PURE;
    STDMETHOD_(IResProvider *, GetTailResProvider)(CTHIS) SCONST PURE;

    STDMETHOD_(void, SetFilePrefix)(THIS_ LPCTSTR pszFilePrefix) PURE;

    //////////////////////////////////////////////////////////////////////////
    STDMETHOD_(BOOL, HasResource)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    STDMETHOD_(HICON, LoadIcon)(THIS_ LPCTSTR pszResName, int cx DEF_VAL(0), int cy DEF_VAL(0), BOOL bFromFile DEF_VAL(FALSE)) PURE;

    STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName, BOOL bFromFile DEF_VAL(FALSE)) PURE;

    STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName, BOOL bFromFile DEF_VAL(FALSE)) PURE;

    STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    STDMETHOD_(IImgX *, LoadImgX)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    STDMETHOD_(size_t, GetRawBufferSize)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    STDMETHOD_(BOOL, GetRawBuffer)(THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size) PURE;
};

SNSEND
#ifndef __SSTRING_I__H__
#define __SSTRING_I__H__
#include <windows.h>
#include <utilities-def.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE IStringA
DECLARE_INTERFACE(IStringA)
{
	STDMETHOD_(int, GetLength)() SCONST PURE;
	STDMETHOD_(BOOL, IsEmpty)(CTHIS) SCONST PURE;
	STDMETHOD_(void, Empty)(THIS) PURE;

	STDMETHOD_(char, GetAt)(CTHIS_ int nIndex) SCONST PURE;
	STDMETHOD_(void, SetAt)(THIS_ int nIndex, char ch)PURE;
	STDMETHOD_(const char *, c_str)(CTHIS) SCONST PURE;

	// string comparison
	STDMETHOD_(int, Compare)(CTHIS_ const char* psz) SCONST PURE;
	STDMETHOD_(int, CompareNoCase)(CTHIS_ const char* psz) SCONST PURE;

	STDMETHOD_(void, TrimBlank)(THIS) PURE;
	STDMETHOD_(int, InsertChar)(THIS_ int nIndex, char ch) PURE;
	STDMETHOD_(int, Insert)(THIS_ int nIndex, const char* psz)PURE;
	STDMETHOD_(int, Delete)(THIS_ int nIndex, int nCount)PURE;
	STDMETHOD_(int, ReplaceChar)(THIS_ char chOld, char chNew)PURE;
	STDMETHOD_(int, Replace)(THIS_ const char* pszOld, const char* pszNew)PURE;
	STDMETHOD_(int, Remove)(THIS_ char chRemove)PURE;

	STDMETHOD_(int, FindChar)(CTHIS_ char ch, int nStart DEF_VAL(0)) SCONST PURE;
	STDMETHOD_(int, ReverseFind)(CTHIS_ char ch) SCONST PURE;

	// find a sub-string (like strstr)
	STDMETHOD_(int, Find)(CTHIS_ const char* pszSub, int nStart DEF_VAL(0)) SCONST PURE;
	// Access to string implementation buffer as "C" character array
	STDMETHOD_(char*, GetBuffer)(THIS_ int nMinBufLength DEF_VAL(-1))PURE;
	STDMETHOD_(void ,ReleaseBuffer)(THIS_ int nNewLength DEF_VAL(-1))PURE;
	STDMETHOD_(char* ,GetBufferSetLength)(THIS_ int nNewLength)PURE;
	STDMETHOD_(void ,SetLength)(THIS_ int nLength)PURE;
	STDMETHOD_(void ,Copy)(THIS_ const IStringA * src) PURE;
	STDMETHOD_(void ,Assign)(THIS_ LPCSTR src) PURE;
	STDMETHOD_(void ,Assign2)(THIS_ LPCSTR src,int nLen) PURE;
	STDMETHOD_(LPVOID,GetPrivData)(CTHIS) SCONST PURE;

	STDMETHOD_(void,ToUpper)(THIS) PURE;
	STDMETHOD_(void,ToLower)(THIS) PURE;
	STDMETHOD_(void,TrimRight)(THIS_ char chTarget DEF_VAL(VK_SPACE)) PURE;
	STDMETHOD_(void,TrimLeft)(THIS_ char chTarget DEF_VAL(VK_SPACE)) PURE;
	STDMETHOD_(void,Trim)(THIS_ char chTarget DEF_VAL(VK_SPACE)) PURE;
	STDMETHOD_(void,AppendChar)(THIS_ char ch) PURE;
	STDMETHOD_(void,AppendStr)(THIS_ const char *pszStr, int nLen DEF_VAL(-1)) PURE;
	STDMETHOD_(void,Release)(THIS) PURE;
};


#undef INTERFACE
#define INTERFACE IStringW
DECLARE_INTERFACE(IStringW)
{
	STDMETHOD_(int, GetLength)(CTHIS) SCONST PURE;
	STDMETHOD_(BOOL, IsEmpty)(CTHIS) SCONST PURE;
	STDMETHOD_(void, Empty)(THIS) PURE;

	STDMETHOD_(wchar_t, GetAt)(CTHIS_ int nIndex) SCONST PURE;
	STDMETHOD_(void, SetAt)(THIS_ int nIndex, wchar_t ch)PURE;
	STDMETHOD_(const wchar_t *, c_str)(CTHIS) SCONST PURE;

	// string comparison
	STDMETHOD_(int, Compare)(CTHIS_ const wchar_t* psz) SCONST PURE;
	STDMETHOD_(int, CompareNoCase)(CTHIS_ const wchar_t* psz) SCONST PURE;

	STDMETHOD_(void, TrimBlank)(THIS) PURE;
	STDMETHOD_(int, InsertChar)(THIS_ int nIndex, wchar_t ch) PURE;
	STDMETHOD_(int, Insert)(THIS_ int nIndex, const wchar_t* psz)PURE;
	STDMETHOD_(int, Delete)(THIS_ int nIndex, int nCount)PURE;
	STDMETHOD_(int, ReplaceChar)(THIS_ wchar_t chOld, wchar_t chNew)PURE;
	STDMETHOD_(int, Replace)(THIS_ const wchar_t* pszOld, const wchar_t* pszNew)PURE;
	STDMETHOD_(int, Remove)(THIS_ wchar_t chRemove)PURE;

	STDMETHOD_(int, FindChar)(CTHIS_ wchar_t ch, int nStart DEF_VAL(0)) SCONST PURE;
	STDMETHOD_(int, ReverseFind)(CTHIS_ wchar_t ch) SCONST PURE;

	// find a sub-string (like strstr)
	STDMETHOD_(int, Find)(CTHIS_ const wchar_t* pszSub, int nStart DEF_VAL(0)) SCONST PURE;
	// Access to string implementation buffer as "C" character array
	STDMETHOD_(wchar_t*, GetBuffer)(THIS_ int nMinBufLength DEF_VAL(-1))PURE;
	STDMETHOD_(void ,ReleaseBuffer)(THIS_ int nNewLength DEF_VAL(-1))PURE;
	STDMETHOD_(wchar_t* ,GetBufferSetLength)(THIS_ int nNewLength)PURE;
	STDMETHOD_(void ,SetLength)(THIS_ int nLength) PURE;
	STDMETHOD_(void ,Copy)(THIS_ const IStringW * src) PURE;
	STDMETHOD_(void ,Assign)(THIS_ LPCWSTR src) PURE;
	STDMETHOD_(void ,Assign2)(THIS_ LPCWSTR src,int nLen) PURE;
	STDMETHOD_(LPVOID,GetPrivData)(CTHIS) SCONST PURE;

	STDMETHOD_(void,ToUpper)(THIS) PURE;
	STDMETHOD_(void,ToLower)(THIS) PURE;
	STDMETHOD_(void,TrimRight)(THIS_ wchar_t chTarget DEF_VAL(VK_SPACE)) PURE;
	STDMETHOD_(void,TrimLeft)(THIS_ wchar_t chTarget DEF_VAL(VK_SPACE)) PURE;
	STDMETHOD_(void,Trim)(THIS_ wchar_t chTarget DEF_VAL(VK_SPACE)) PURE;
	STDMETHOD_(void,AppendChar)(THIS_ wchar_t ch) PURE;
	STDMETHOD_(void,AppendStr)(THIS_ const wchar_t *pszStr, int nLen DEF_VAL(-1)) PURE;
	STDMETHOD_(void,Release)(THIS) PURE;
};

#ifdef _UNICODE
#define IStringT IStringW
#else
#define IStringT IStringA
#endif

SNSEND
#endif // __SSTRING_I__H__
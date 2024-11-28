#ifndef __UTILITIES__H__
#define __UTILITIES__H__

#include <utilities-def.h>
#include <windows.h>
#include <interface/sstring-i.h>
#include <interface/sxml-i.h>
#ifdef __cplusplus
extern "C" {
#define StringA SNS::IStringA
#define StringW SNS::IStringW
#define XmlDoc SNS::IXmlDoc
#else
#define StringA IStringA
#define StringW IStringW
#define XmlDoc IXmlDoc
#endif//__cplusplus

	UTILITIES_API int RectWidth(LPCRECT rc);
	UTILITIES_API int RectHeight(LPCRECT rc);

#ifdef _WIN32
	UTILITIES_API void SHiMetricToPixel(const SIZEL* lpSizeInHiMetric, LPSIZEL lpSizeInPix);
	UTILITIES_API void SPixelToHiMetric(const SIZEL* lpSizeInPix, LPSIZEL lpSizeInHiMetric);
	BOOL UTILITIES_API UpdateDIBPixmap(HBITMAP bmp, int wid, int hei, int bitsPixel, int stride, CONST VOID* pjBits);
#endif

	UTILITIES_API int Str2Int(LPCWSTR src, BOOL supportHex);

	UTILITIES_API StringA* CreateIStringA(LPCSTR src);
	UTILITIES_API StringW* CreateIStringW(LPCWSTR src);

	UTILITIES_API XmlDoc* CreateIXmlDoc();

	int  UTILITIES_API GetWindowScale(HWND hWnd);
	BOOL UTILITIES_API IsFilePathValid(LPCTSTR path);
	BOOL UTILITIES_API HasFont(LPCTSTR fontName);
	BOOL UTILITIES_API IsSupportMaxMove();
	long UTILITIES_API file_length_A(const char* path);
	long UTILITIES_API file_length_W(const wchar_t* path);
#ifdef _UNICODE
#define file_length file_length_W
#else
#define file_length file_length_A
#endif

	HANDLE UTILITIES_API LoadIconFromMemory(const void* buf, UINT cbSize, BOOL fIcon, int width, int height, UINT cFlag);

#ifdef __cplusplus
}
#endif//__cplusplus
#endif // __UTILITIES__H__
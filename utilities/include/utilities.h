#ifndef __UTILITIES__H__
#define __UTILITIES__H__

#include <utilities-def.h>
#include <windows.h>
#include <interface/sstring-i.h>
#include <interface/sxml-i.h>

SNSBEGIN

UTILITIES_API int RectWidth(LPCRECT rc) ;
UTILITIES_API int RectHeight(LPCRECT rc) ;

UTILITIES_API void SHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix);
UTILITIES_API void SPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric);

UTILITIES_API int Str2Int(LPCWSTR src,BOOL supportHex);

UTILITIES_API IStringA * CreateIStringA(LPCSTR src);
UTILITIES_API IStringW * CreateIStringW(LPCWSTR src);

UTILITIES_API IXmlDoc * CreateIXmlDoc();

SNSEND
#endif // __UTILITIES__H__
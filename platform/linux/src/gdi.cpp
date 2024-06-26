#include <platform.h>
#include <gdi.h>


HPEN ExtCreatePen(DWORD iPenStyle, DWORD cWidth, const LOGBRUSH *plbrush, DWORD cStyle, const DWORD *pstyle)
{
    return 0;
}

int GetObject(HANDLE h, int c, LPVOID pv)
{
    return 0;
}

HPEN CreatePen(int iStyle, int cWidth, COLORREF color)
{
    LOGPEN logPen = {iStyle,cWidth,color};
    return CreatePenIndirect(&logPen);
}

HPEN CreatePenIndirect(const LOGPEN *plpen)
{
    return 0;
}

HFONT CreateFontIndirect(const LOGFONT *lplf)
{
    return 0;
}

HFONT CreateFont(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, 
DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, 
DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName)
{
    LOGFONT lf;
    lf.lfCharSet = iCharSet;
    lf.lfHeight = cHeight;
    lf.lfWidth = cWidth;
    lf.lfEscapement = cEscapement;
    lf.lfOrientation = cOrientation;
    lf.lfWeight = cWeight;
    lf.lfItalic = bItalic;
    lf.lfUnderline = bUnderline;
    lf.lfStrikeOut = bStrikeOut;
    lf.lfClipPrecision = iClipPrecision;
    lf.lfOutPrecision = iOutPrecision;
    lf.lfQuality = iQuality;
    lf.lfPitchAndFamily=iPitchAndFamily;
    strcpy_s(lf.lfFaceName,ARRAYSIZE(lf.lfFaceName),pszFaceName);
    return CreateFontIndirect(&lf);
}

HBITMAP CreateDIBitmap(HDC hdc, const BITMAPINFOHEADER *pbmih, DWORD flInit, const VOID *pjBits, const BITMAPINFO *pbmi, UINT iUsage)
{
    return 0;
}

HBRUSH  CreateDIBPatternBrush(HGLOBAL h, UINT iUsage)
{
    return 0;
}

HBRUSH CreateDIBPatternBrushPt(const VOID *lpPackedDIB, UINT iUsage)
{
    return 0;
}

HBRUSH CreateSolidBrush(COLORREF color)
{
    return 0;
}

HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO *lpbmi, UINT usage, VOID **ppvBits, HANDLE hSection, DWORD offset)
{
    return HBITMAP(0);
}

HDC CreateCompatibleDC(HDC hdc)
{
    return HDC(0);
}

BOOL DeleteDC(HDC hdc)
{
    return 0;
}

int SetBkMode(HDC hdc, int mode)
{
    return 0;
}

int SetGraphicsMode(HDC hdc, int iMode)
{
    return 0;
}

HBITMAP CreateCompatibleBitmap(HDC hdc, int cx, int cy)
{
    return HBITMAP(0);
}

HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h)
{
    return HGDIOBJ(0);
}

BOOL DeleteObject(HGDIOBJ hObj)
{
    return FALSE;
}

int SaveDC(HDC hdc)
{
    return 0;
}

int SelectClipRgn(HDC hdc, HRGN hrgn)
{
    return 0;
}

int ExtSelectClipRgn(HDC hdc, HRGN hrgn, int mode)
{
    return 0;
}

int ExcludeClipRect(HDC hdc, int left, int top, int right, int bottom)
{
    return 0;
}

int IntersectClipRect(HDC hdc, int left, int top, int right, int bottom)
{
    return 0;
}

BOOL RestoreDC(HDC hdc, int nSavedDC)
{
    return 0;
}

int  GetClipRgn(HDC hdc, HRGN hrgn)
{
    return 0;
}

HGDIOBJ  GetCurrentObject(HDC hdc, UINT type)
{
    return HGDIOBJ(0);
}

int  GetDIBits(HDC hdc, HBITMAP hbm, UINT start, UINT cLines, LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT usage)
{
    return 0;
}

int DrawText(HDC hdc, LPCSTR lpchText, int cchText, LPRECT lprc, UINT format)
{
    return 0;
}

BOOL GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int c, LPSIZE psizl)
{
    return 0;
}

HGDIOBJ  GetStockObject(int i)
{
    return HGDIOBJ(0);
}

BOOL  Rectangle(HDC hdc, int left, int top, int right, int bottom)
{
    return 0;
}

BOOL  RoundRect(HDC hdc, int left, int top, int right, int bottom, int width, int height)
{
    return 0;
}

BOOL  Polyline(HDC hdc, const POINT *apt, int cpt)
{
    return 0;
}

BOOL  AlphaBlend(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
    return 0;
}

BOOL  TextOut(HDC hdc, int x, int y, LPCSTR lpString, int c)
{
    return 0;
}

BOOL  SetViewportOrgEx(HDC hdc, int x, int y, LPPOINT lppt)
{
    return 0;
}

BOOL  OffsetViewportOrgEx(HDC hdc, int x, int y, LPPOINT lppt)
{
    return 0;
}

int  FillRect(HDC hDC, const RECT *lprc, HBRUSH hbr)
{
    return 0;
}

int  FrameRect(HDC hDC, const RECT *lprc, HBRUSH hbr)
{
    return 0;
}

BOOL  InvertRect(HDC hDC, const RECT *lprc)
{
    return 0;
}

BOOL  Ellipse(HDC hdc, int left, int top, int right, int bottom)
{
    return 0;
}

BOOL  Pie(HDC hdc, int left, int top, int right, int bottom, int xr1, int yr1, int xr2, int yr2)
{
    return 0;
}

BOOL  Arc(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    return 0;
}

BOOL  GetWorldTransform(HDC hdc, LPXFORM lpxf)
{
    return 0;
}

BOOL  SetWorldTransform(HDC hdc, const XFORM *lpxf)
{
    return 0;
}

int  SetROP2(HDC hdc, int rop2)
{
    return 0;
}

COLORREF  SetTextColor(HDC hdc, COLORREF color)
{
    return COLORREF(0);
}

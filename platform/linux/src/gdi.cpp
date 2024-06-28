#include <platform.h>
#include <gdi.h>
#include <cairo/cairo.h>

typedef struct _GdiObj{
    int type;
    void *ptr;
    _GdiObj(int _type,void * _ptr):type(_type),ptr(_ptr){

    }
}* HGDIOBJ;

HGDIOBJ InitGdiObj(int type,void *ptr){
    return new _GdiObj(type,ptr);
}

int GetGdiObjType(HGDIOBJ hgdiobj){
    return hgdiobj->type;
}
void* GetGdiObjPtr(HGDIOBJ hgdiobj){
    return hgdiobj->ptr;
}

HPEN ExtCreatePen(DWORD iPenStyle, DWORD cWidth, const LOGBRUSH *plbrush, DWORD cStyle, const DWORD *pstyle)
{
    return 0;
}

int GetObject(HGDIOBJ h, int c, LPVOID pv)
{
    if(!h->ptr)
        return 0;
    switch(h->type){
        case go_dib:
        if(c>=sizeof(BITMAP)){
            BITMAP * bm = (BITMAP*)pv;
            cairo_surface_t * pixmap = (cairo_surface_t*)h->ptr;
            bm->bmWidth = cairo_image_surface_get_width(pixmap);
            bm->bmHeight = cairo_image_surface_get_height(pixmap);
            bm->bmPlanes = 1;
            bm->bmBitsPixel=32;
            bm->bmWidthBytes = bm->bmWidth*4;
            bm->bmType = BI_RGB;
            bm->bmBits = cairo_image_surface_get_data(pixmap);
            return sizeof(BITMAP);
        }
        break;
    }
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
    if(lpbmi->bmiHeader.biBitCount!=32)
        return 0;
    cairo_surface_t *ret = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,lpbmi->bmiHeader.biWidth,lpbmi->bmiHeader.biHeight);
    if(!ret)
        return 0;
    if(ppvBits){
        *ppvBits = cairo_image_surface_get_data(ret);
    }
    return InitGdiObj(go_dib,ret);
}

BOOL   UpdateDIBPixmap(HBITMAP bmp,int wid,int hei,int bitsPixel,int stride,CONST VOID*pjBits){
    BITMAP bm={0};
    GetObject(bmp,sizeof(bm),&bm);
    if(!bm.bmBits)
        return FALSE;
    if(bm.bmWidth != wid || bm.bmHeight!=hei || bm.bmBitsPixel != bitsPixel)
        return FALSE;
    if(pjBits)
        memcpy(bm.bmBits,pjBits,hei*stride);
    else
        memset(bm.bmBits,0,hei*stride);
    MarkPixmapDirty(bmp);
    return TRUE;
}

void   MarkPixmapDirty(HBITMAP bmp){
    if(bmp && bmp->type == go_dib){
        cairo_surface_mark_dirty((cairo_surface_t*)bmp->ptr);
    }
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
    if(!hObj)
        return FALSE;
    switch (hObj->type)
    {
    case go_dib:
        {
            cairo_surface_t* pixmap = (cairo_surface_t*)hObj->ptr;
            cairo_surface_destroy(pixmap);
        }
        break;
    default:
        break;
    }
    delete hObj;
    return TRUE;
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

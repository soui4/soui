#ifndef _LINUX_GDI_H_
#define _LINUX_GDI_H_

typedef struct _GdiObj{
    enum{
        go_pen=0,
        go_brush,
        go_dib,
        go_ddb,
        go_font,
        go_rgn,
    };
    int type;
    void *ptr;
    _GdiObj(int _type,void * _ptr):type(_type),ptr(_ptr){

    }
}* HGDIOBJ;

typedef struct  _XFORM { 
	FLOAT eM11; 
	FLOAT eM12; 
	FLOAT eM21; 
	FLOAT eM22; 
	FLOAT eDx; 
	FLOAT eDy; 
} XFORM, *PXFORM, *LPXFORM; 

/* Logical Brush (or Pattern) */
typedef struct tagLOGBRUSH
  {
    UINT        lbStyle;
    COLORREF    lbColor;
    ULONG_PTR    lbHatch;    // Sundown: lbHatch could hold a HANDLE
  } LOGBRUSH, *PLOGBRUSH, NEAR *NPLOGBRUSH, FAR *LPLOGBRUSH;

#define BS_SOLID            0
#define BS_NULL             1
#define BS_HOLLOW           BS_NULL
#define BS_HATCHED          2
#define BS_PATTERN          3
#define BS_INDEXED          4
#define BS_DIBPATTERN       5
#define BS_DIBPATTERNPT     6
#define BS_PATTERN8X8       7
#define BS_DIBPATTERN8X8    8
#define BS_MONOPATTERN      9

HPEN WINAPI ExtCreatePen(  DWORD iPenStyle,
                                     DWORD cWidth,
                                     CONST LOGBRUSH *plbrush,
                                     DWORD cStyle,
                                    CONST DWORD *pstyle);

int   WINAPI GetObject( HGDIOBJ h,  int c, LPVOID pv);

/* Logical Pen */
typedef struct tagLOGPEN
  {
    UINT        lopnStyle;
    LONG        lopnWidth;
    COLORREF    lopnColor;
  } LOGPEN, *PLOGPEN, NEAR *NPLOGPEN, FAR *LPLOGPEN;

HPEN    WINAPI CreatePen(  int iStyle,  int cWidth,  COLORREF color);
HPEN    WINAPI CreatePenIndirect(  CONST LOGPEN *plpen);


HFONT   WINAPI CreateFontIndirect(  CONST LOGFONT *lplf);

HFONT   WINAPI CreateFont(  int cHeight,  int cWidth,  int cEscapement,  int cOrientation,  int cWeight,  DWORD bItalic,
                              DWORD bUnderline,  DWORD bStrikeOut,  DWORD iCharSet,  DWORD iOutPrecision,  DWORD iClipPrecision,
                              DWORD iQuality,  DWORD iPitchAndFamily,  LPCSTR pszFaceName);

/* Bitmap Header Definition */
typedef struct tagBITMAP
  {
    LONG        bmType;
    LONG        bmWidth;
    LONG        bmHeight;
    LONG        bmWidthBytes;
    WORD        bmPlanes;
    WORD        bmBitsPixel;
    LPVOID      bmBits;
  } BITMAP, *PBITMAP, NEAR *NPBITMAP, FAR *LPBITMAP;


  
typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

/* constants for the biCompression field */
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO, FAR *LPBITMAPINFO, *PBITMAPINFO;

#define DIB_RGB_COLORS      0 /* color table in RGBs */
#define DIB_PAL_COLORS      1 /* color table in palette indices */

HBITMAP WINAPI CreateDIBitmap(  HDC hdc,  CONST BITMAPINFOHEADER *pbmih,  DWORD flInit,  CONST VOID *pjBits,  CONST BITMAPINFO *pbmi,  UINT iUsage);
HBRUSH  WINAPI CreateDIBPatternBrush(  HGLOBAL h,  UINT iUsage);
HBRUSH  WINAPI CreateDIBPatternBrushPt(  CONST VOID *lpPackedDIB,  UINT iUsage);
HBRUSH  WINAPI CreateSolidBrush(  COLORREF color);
HBITMAP WINAPI CreateDIBSection( HDC hdc,  CONST BITMAPINFO *lpbmi,  UINT usage,  VOID **ppvBits,  HANDLE hSection,  DWORD offset);
BOOL    WINAPI UpdateDIBPixmap(HBITMAP bmp,int wid,int hei,int bitsPixel,int stride,CONST VOID*pjBits);
HDC     WINAPI CreateCompatibleDC(  HDC hdc);
BOOL    WINAPI DeleteDC( HDC hdc);

/* Background Modes */
#define TRANSPARENT         1
#define OPAQUE              2
#define BKMODE_LAST         2

/* Graphics Modes */

#define GM_COMPATIBLE       1
#define GM_ADVANCED         2
#define GM_LAST             2

int   WINAPI SetBkMode( HDC hdc,  int mode);
int   WINAPI SetGraphicsMode( HDC hdc,  int iMode);

 
 HBITMAP WINAPI CreateCompatibleBitmap(  HDC hdc,  int cx,  int cy);
 HGDIOBJ WINAPI SelectObject( HDC hdc,  HGDIOBJ h);
 BOOL WINAPI DeleteObject(  HGDIOBJ ho);
 int  WINAPI SaveDC( HDC hdc);
 int  WINAPI SelectClipRgn( HDC hdc,  HRGN hrgn);
 int  WINAPI ExtSelectClipRgn( HDC hdc,  HRGN hrgn,  int mode);
 int  WINAPI ExcludeClipRect(  HDC hdc,  int left,  int top,  int right,  int bottom);
 int  WINAPI IntersectClipRect(  HDC hdc,  int left,  int top,  int right,  int bottom);
 BOOL WINAPI RestoreDC( HDC hdc,  int nSavedDC);

 int   WINAPI GetClipBox(  HDC hdc,   LPRECT lprect);
 int   WINAPI GetClipRgn(  HDC hdc,  HRGN hrgn);
 HGDIOBJ WINAPI GetCurrentObject(  HDC hdc,  UINT type);
 int   WINAPI GetDIBits(  HDC hdc,  HBITMAP hbm,  UINT start,  UINT cLines,   LPVOID lpvBits, LPBITMAPINFO lpbmi,  UINT usage);  // SAL actual size of lpbmi is computed from structure elements

 BOOL  WINAPI BitBlt(  HDC hdc,  int x,  int y,  int cx,  int cy,  HDC hdcSrc,  int x1,  int y1,  DWORD rop);
int
WINAPI
DrawText(
     HDC hdc,
     LPCSTR lpchText,
     int cchText,
     LPRECT lprc,
     UINT format);

BOOL WINAPI
GetTextExtentPoint32(
     HDC hdc,
     LPCSTR lpString,
     int c,
     LPSIZE psizl
    );

/* Stock Logical Objects */
#define WHITE_BRUSH         0
#define LTGRAY_BRUSH        1
#define GRAY_BRUSH          2
#define DKGRAY_BRUSH        3
#define BLACK_BRUSH         4
#define NULL_BRUSH          5
#define HOLLOW_BRUSH        NULL_BRUSH
#define WHITE_PEN           6
#define BLACK_PEN           7
#define NULL_PEN            8
#define OEM_FIXED_FONT      10
#define ANSI_FIXED_FONT     11
#define ANSI_VAR_FONT       12
#define SYSTEM_FONT         13
#define DEVICE_DEFAULT_FONT 14
#define DEFAULT_PALETTE     15
#define SYSTEM_FIXED_FONT   16

 HGDIOBJ WINAPI GetStockObject(  int i);

 BOOL WINAPI Rectangle( HDC hdc,  int left,  int top,  int right,  int bottom);

 BOOL  WINAPI RoundRect( HDC hdc,  int left,  int top,  int right,  int bottom,  int width,  int height);

 BOOL  WINAPI Polyline( HDC hdc, CONST POINT *apt,  int cpt);


 BOOL WINAPI AlphaBlend(  HDC hdcDest,  int xoriginDest,  int yoriginDest,  int wDest,  int hDest,  HDC hdcSrc,  int xoriginSrc,  int yoriginSrc,  int wSrc,  int hSrc,  BLENDFUNCTION ftn);

 BOOL  WINAPI TextOut(  HDC hdc,  int x,  int y, LPCSTR lpString,  int c);

 BOOL  WINAPI SetViewportOrgEx(  HDC hdc,  int x,  int y,  LPPOINT lppt);
 BOOL  WINAPI OffsetViewportOrgEx(  HDC hdc,  int x,  int y,  LPPOINT lppt);


int
WINAPI
FillRect(
     HDC hDC,
     CONST RECT *lprc,
     HBRUSH hbr);


int
WINAPI
FrameRect(
     HDC hDC,
     CONST RECT *lprc,
     HBRUSH hbr);


BOOL
WINAPI
InvertRect(
     HDC hDC,
     CONST RECT *lprc);

 BOOL WINAPI Ellipse(  HDC hdc,  int left,  int top,   int right,  int bottom);

 BOOL WINAPI Pie( HDC hdc,  int left,  int top,  int right,  int bottom,  int xr1,  int yr1,  int xr2,  int yr2);

 BOOL  WINAPI Arc(  HDC hdc,  int x1,  int y1,  int x2,  int y2,  int x3,  int y3,  int x4,  int y4);

 BOOL WINAPI GetWorldTransform(  HDC hdc,  LPXFORM lpxf);
 BOOL WINAPI SetWorldTransform(  HDC hdc,  CONST XFORM * lpxf);

 int   WINAPI SetROP2( HDC hdc,  int rop2);
 COLORREF WINAPI SetTextColor( HDC hdc,  COLORREF color);

#endif//_LINUX_GDI_H_
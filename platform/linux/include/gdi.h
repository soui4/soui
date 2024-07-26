#ifndef _LINUX_GDI_H_
#define _LINUX_GDI_H_

#define OBJ_PEN             1
#define OBJ_BRUSH           2
#define OBJ_DC              3
#define OBJ_FONT            6
#define OBJ_BITMAP          7
#define OBJ_REGION          8

typedef void (*fun_gdi_free)(void*);
HGDIOBJ InitGdiObj(int type, void* ptr);
HGDIOBJ InitGdiObj2(int type,void *ptr, fun_gdi_free cbFree);
void* GetGdiObjPtr(HGDIOBJ hgdiobj);
void SetGdiObjPtr(HGDIOBJ hgdiObj, void* ptr);
int  RefGdiObj(HGDIOBJ hgdiObj);

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
    UINT_PTR    lbHatch;    // Sundown: lbHatch could hold a HANDLE
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
int   WINAPI GetObjectType(HGDIOBJ hgdiobj);
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

typedef struct tagBITMAPFILEHEADER{
    WORD bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
}BITMAPFILEHEADER,*PBITMAPFILEHEADER;

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

#define CBM_INIT        0x04L   /* initialize bitmap */


HBITMAP WINAPI CreateDIBitmap(  HDC hdc,  CONST BITMAPINFOHEADER *pbmih,  DWORD flInit,  CONST VOID *pjBits,  CONST BITMAPINFO *pbmi,  UINT iUsage);
HBRUSH  WINAPI CreateDIBPatternBrush(  HGLOBAL h,  UINT iUsage);
HBRUSH  WINAPI CreateDIBPatternBrushPt(  CONST VOID *lpPackedDIB,  UINT iUsage);
HBRUSH  WINAPI CreateSolidBrush(  COLORREF color);
HBRUSH  WINAPI CreatePatternBrush(HBITMAP bmp);
HBITMAP WINAPI CreateDIBSection( HDC hdc,  CONST BITMAPINFO *lpbmi,  UINT usage,  VOID **ppvBits,  HANDLE hSection,  DWORD offset);
BOOL    WINAPI UpdateDIBPixmap(HBITMAP bmp,int wid,int hei,int bitsPixel,int stride,CONST VOID*pjBits);
void    WINAPI MarkPixmapDirty(HBITMAP bmp);

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

 BOOL WINAPI InvertRgn(
			   HDC hdc,    // handle to device context
			   HRGN hrgn   // handle to region to be inverted
			   );
 int   WINAPI GetClipBox(  HDC hdc,   LPRECT lprect);
 int   WINAPI GetClipRgn(  HDC hdc,  HRGN hrgn);

 BOOL WINAPI FillRgn(
			 HDC hdc,    // handle to device context
			 HRGN hrgn,  // handle to region to be filled
			 HBRUSH hbr  // handle to brush used to fill the region
			 );

BOOL WINAPI PaintRgn(  HDC hdc,    // handle to device context
			  HRGN hrgn   // handle to region to be painted
			  );
              
BOOL WINAPI FrameRgn(
			  HDC hdc,     // handle to device context
			  HRGN hrgn,   // handle to region to be framed
			  HBRUSH hbr,  // handle to brush used to draw border
			  int nWidth,  // width of region frame
			  int nHeight  // height of region frame
			  );

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
#define DEFAULT_GUI_FONT    17
#define NULL_BITMAP         20

 HGDIOBJ WINAPI GetStockObject(  int i);

 #define COLOR_INFOBK 1
 COLORREF GetSysColor(int i);

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


#define IMAGE_BITMAP 	0
#define IMAGE_CURSOR 	1
#define IMAGE_ICON      2



#define LR_CREATEDIBSECTION  0x00002000 \
	//When the uType parameter specifies IMAGE_BITMAP, causes the function to return a DIB section bitmap rather than a compatible bitmap. This flag is useful for loading a bitmap without mapping it to the colors of the display device.

#define LR_DEFAULTCOLOR \
0x00000000 \
	//The default flag; it does nothing. All it means is "not LR_MONOCHROME".

#define LR_DEFAULTSIZE \
0x00000040 \
	//Uses the width or height specified by the system metric values for cursors or icons, if the cxDesired or cyDesired values are set to zero. If this flag is not specified and cxDesired and cyDesired are set to zero, the function uses the actual resource size. If the resource contains multiple images, the function uses the size of the first image.

#define LR_LOADFROMFILE \
0x00000010 

HANDLE LoadImage(
  HINSTANCE hInst,
             LPCSTR    name,
            UINT      type,
             int       cx,
             int       cy,
             UINT      fuLoad
);

typedef USHORT COLOR16;

typedef struct _TRIVERTEX
{
    LONG    x;
    LONG    y;
    COLOR16 Red;
    COLOR16 Green;
    COLOR16 Blue;
    COLOR16 Alpha;
}TRIVERTEX, * PTRIVERTEX, * LPTRIVERTEX;

typedef struct _GRADIENT_RECT {
    ULONG    UpperLeft;
    ULONG    LowerRight;
}GRADIENT_RECT, * PGRADIENT_RECT;

#define GRADIENT_FILL_RECT_H    0x00000000
#define GRADIENT_FILL_RECT_V    0x00000001
#define GRADIENT_FILL_TRIANGLE  0x00000002
#define GRADIENT_FILL_OP_FLAG   0x000000ff

BOOL WINAPI GradientFill(
    HDC hdc,
    TRIVERTEX* pVertices,
    ULONG nVertices,
    void* pMesh,
    ULONG nMeshElements,
    DWORD dwMode
);

#endif//_LINUX_GDI_H_
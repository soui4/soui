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

typedef LONG FXPT16DOT16, *LPFXPT16DOT16;
typedef LONG FXPT2DOT30, *LPFXPT2DOT30;
typedef LONG LCSCSTYPE;
typedef LONG LCSGAMUTMATCH;

typedef struct tagCIEXYZ
{
  FXPT2DOT30 ciexyzX;
  FXPT2DOT30 ciexyzY;
  FXPT2DOT30 ciexyzZ;
} CIEXYZ, *LPCIEXYZ;

typedef struct tagCIEXYZTRIPLE
{
  CIEXYZ ciexyzRed;
  CIEXYZ ciexyzGreen;
  CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE, *LPCIEXYZTRIPLE;

typedef struct
{
    DWORD        bV4Size;
    LONG         bV4Width;
    LONG         bV4Height;
    WORD         bV4Planes;
    WORD         bV4BitCount;
    DWORD        bV4V4Compression;
    DWORD        bV4SizeImage;
    LONG         bV4XPelsPerMeter;
    LONG         bV4YPelsPerMeter;
    DWORD        bV4ClrUsed;
    DWORD        bV4ClrImportant;
    DWORD        bV4RedMask;
    DWORD        bV4GreenMask;
    DWORD        bV4BlueMask;
    DWORD        bV4AlphaMask;
    DWORD        bV4CSType;
    CIEXYZTRIPLE bV4Endpoints;
    DWORD        bV4GammaRed;
    DWORD        bV4GammaGreen;
    DWORD        bV4GammaBlue;
} BITMAPV4HEADER, *PBITMAPV4HEADER;

typedef struct {
    DWORD        bV5Size;
    LONG         bV5Width;
    LONG         bV5Height;
    WORD         bV5Planes;
    WORD         bV5BitCount;
    DWORD        bV5Compression;
    DWORD        bV5SizeImage;
    LONG         bV5XPelsPerMeter;
    LONG         bV5YPelsPerMeter;
    DWORD        bV5ClrUsed;
    DWORD        bV5ClrImportant;
    DWORD        bV5RedMask;
    DWORD        bV5GreenMask;
    DWORD        bV5BlueMask;
    DWORD        bV5AlphaMask;
    DWORD        bV5CSType;
    CIEXYZTRIPLE bV5Endpoints;
    DWORD        bV5GammaRed;
    DWORD        bV5GammaGreen;
    DWORD        bV5GammaBlue;
    DWORD        bV5Intent;
    DWORD        bV5ProfileData;
    DWORD        bV5ProfileSize;
    DWORD        bV5Reserved;
} BITMAPV5HEADER, *LPBITMAPV5HEADER, *PBITMAPV5HEADER;

typedef struct tagBITMAPFILEHEADER{
    WORD bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
}BITMAPFILEHEADER,*PBITMAPFILEHEADER;


typedef struct
{
    DWORD   bcSize;
    WORD    bcWidth;
    WORD    bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAPCOREHEADER, *PBITMAPCOREHEADER, *LPBITMAPCOREHEADER;


typedef struct tagRGBTRIPLE {
  BYTE rgbtBlue;
  BYTE rgbtGreen;
  BYTE rgbtRed;
} RGBTRIPLE;

typedef struct
{
    BITMAPCOREHEADER bmciHeader;
    RGBTRIPLE        bmciColors[1];
} BITMAPCOREINFO, *PBITMAPCOREINFO, *LPBITMAPCOREINFO;

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

typedef struct
{
	BITMAP		dsBm;
	BITMAPINFOHEADER	dsBmih;
	DWORD			dsBitfields[3];
	HANDLE		dshSection;
	DWORD			dsOffset;
} DIBSECTION, *PDIBSECTION, *LPDIBSECTION;

#define DIB_RGB_COLORS      0 /* color table in RGBs */
#define DIB_PAL_COLORS      1 /* color table in palette indices */

#define CBM_INIT        0x04L   /* initialize bitmap */

  /* StretchBlt() modes */
#define BLACKONWHITE         1
#define WHITEONBLACK         2
#define COLORONCOLOR	     3
#define HALFTONE             4
#define MAXSTRETCHBLTMODE    4

#define STRETCH_ANDSCANS     BLACKONWHITE
#define STRETCH_ORSCANS      WHITEONBLACK
#define STRETCH_DELETESCANS  COLORONCOLOR
#define STRETCH_HALFTONE     HALFTONE

HBITMAP WINAPI CreateBitmap(
    int nWidth,         // bitmap width, in pixels
    int nHeight,        // bitmap height, in pixels
    UINT cPlanes,       // number of color planes
    UINT cBitsPerPel,   // number of bits to identify color
    CONST VOID* lpvBits // color data array
);
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

HICON WINAPI CreateIconIndirect(PICONINFO iconinfo);

BOOL  WINAPI BitBlt(  HDC hdc,  int x,  int y,  int cx,  int cy,  HDC hdcSrc,  int x1,  int y1,  DWORD rop);

BOOL  WINAPI StretchBlt(  HDC hdc,  int x,  int y,  int cx,  int cy,  HDC hdcSrc,  int x1,  int y1, int cx2,  int cy2,  DWORD rop);

BOOL WINAPI DrawIcon( HDC hdc, INT x, INT y, HICON hIcon );

BOOL WINAPI DrawIconEx( HDC hdc, INT x0, INT y0, HICON icon, INT width,
                              INT height, UINT step, HBRUSH brush, UINT flags );

INT WINAPI  StretchDIBits( HDC hdc, INT x_dst, INT y_dst, INT width_dst,
                                            INT height_dst, INT x_src, INT y_src, INT width_src,
                                            INT height_src, const void *bits, const BITMAPINFO *bmi,
                                            UINT coloruse, DWORD rop );

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

 COLORREF GetSysColor(int i);

 BOOL WINAPI Rectangle( HDC hdc,  int left,  int top,  int right,  int bottom);

 BOOL  WINAPI RoundRect( HDC hdc,  int left,  int top,  int right,  int bottom,  int width,  int height);

 BOOL  WINAPI Polyline( HDC hdc, CONST POINT *apt,  int cpt);


 BOOL WINAPI AlphaBlend(  HDC hdcDest,  int xoriginDest,  int yoriginDest,  int wDest,  int hDest,  HDC hdcSrc,  int xoriginSrc,  int yoriginSrc,  int wSrc,  int hSrc,  BLENDFUNCTION ftn);

 BOOL  WINAPI TextOut(  HDC hdc,  int x,  int y, LPCSTR lpString,  int c);

 BOOL  WINAPI SetViewportOrgEx(  HDC hdc,  int x,  int y,  LPPOINT lppt);
 BOOL  WINAPI GetViewportOrgEx(HDC hdc, LPPOINT lpPoint);
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


/* types of LoadImage */
#define IMAGE_BITMAP	0
#define IMAGE_ICON	1
#define IMAGE_CURSOR	2
#define IMAGE_ENHMETAFILE	3

/* loadflags to LoadImage */
#define LR_DEFAULTCOLOR		0x0000
#define LR_MONOCHROME		0x0001
#define LR_COLOR		0x0002
#define LR_COPYRETURNORG	0x0004
#define LR_COPYDELETEORG	0x0008
#define LR_LOADFROMFILE		0x0010
#define LR_LOADTRANSPARENT	0x0020
#define LR_DEFAULTSIZE		0x0040
#define LR_VGACOLOR		0x0080
#define LR_LOADMAP3DCOLORS	0x1000
#define	LR_CREATEDIBSECTION	0x2000
#define LR_COPYFROMRESOURCE	0x4000
#define LR_SHARED		0x8000

/* Flags for DrawIconEx.  */
#define DI_MASK                 0x0001
#define DI_IMAGE                0x0002
#define DI_NORMAL               (DI_MASK | DI_IMAGE)
#define DI_COMPAT               0x0004
#define DI_DEFAULTSIZE          0x0008
#define DI_NOMIRROR             0x0010


#define BITSPIXEL 1
#define PLANES 2
int WINAPI GetDeviceCaps(HDC hdc,int cap);

HANDLE WINAPI LoadImageW( HINSTANCE hinst, LPCWSTR name, UINT type,
                INT desiredx, INT desiredy, UINT loadflags );
HANDLE WINAPI LoadImageA( HINSTANCE hinst, LPCSTR name, UINT type,
                INT desiredx, INT desiredy, UINT loadflags );

#ifdef _UNICODE
#define LoadImage LoadImageW
#else
#define LoadImage LoadImageA
#endif

HANDLE WINAPI LoadImageBuf(const void* buf, UINT length, UINT type,
    INT desiredx, INT desiredy, UINT loadflags);

HBITMAP WINAPI CreateBitmap( INT width, INT height, UINT planes,
                                  UINT bpp, const void *bits );

void WINAPI SetStretchBltMode(HDC hdc,int mode);

HICON WINAPI CreateIconIndirect(PICONINFO piconinfo);

BOOL WINAPI GetIconInfo(HICON hIcon,
    PICONINFO piconinfo
);

BOOL WINAPI DrawIcon(HDC hDC,
    int X,
    int Y,
    HICON hIcon
);

BOOL WINAPI DrawIconEx(HDC hdc,
    int xLeft,
    int yTop,
    HICON hIcon,
    int cxWidth,
    int cyWidth,
    UINT istepIfAniCur,
    HBRUSH hbrFlickerFreeDraw,
    UINT diFlags
);

BOOL WINAPI DestroyIcon(HICON hIcon);

HICON WINAPI CreateIconFromResourceEx(LPBYTE bits, UINT cbSize,
    BOOL bIcon, DWORD dwVersion,
    INT width, INT height,
    UINT cFlag);

#endif//_LINUX_GDI_H_
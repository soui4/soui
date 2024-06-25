#ifndef _REGION_H_
#define _REGION_H_

#include <platform.h>

#ifndef CONST 
#define CONST const
#endif

int CombineRgn(
   HRGN hrgnDst,
   HRGN hrgnSrc1,
   HRGN hrgnSrc2,
   int  iMode
);

HRGN CreateEllipticRgn(
					   int nLeftRect,   // x-coord of upper-left corner of rectangle
					   int nTopRect,    // y-coord of upper-left corner of rectangle
					   int nRightRect,  // x-coord of lower-right corner of rectangle
					   int nBottomRect  // y-coord of lower-right corner of rectangle
					   );

HRGN CreateEllipticRgnIndirect(  CONST RECT *lprc   /*bounding rectangle*/);

 HRGN CreatePolygonRgn(  CONST POINT *lppt,  // array of points
  int cPoints,        // number of points in array
  int fnPolyFillMode  // polygon-filling mode
  );

 HRGN CreateRectRgn(
	 int nLeftRect,   // x-coordinate of upper-left corner
	 int nTopRect,    // y-coordinate of upper-left corner
	 int nRightRect,  // x-coordinate of lower-right corner
	 int nBottomRect  // y-coordinate of lower-right corner
	 );
HRGN CreateRectRgnIndirect(  CONST RECT *lprc   /*rectangle*/);

HRGN CreateRoundRectRgn(
						int nLeftRect,      // x-coordinate of upper-left corner
						int nTopRect,       // y-coordinate of upper-left corner
						int nRightRect,     // x-coordinate of lower-right corner
						int nBottomRect,    // y-coordinate of lower-right corner
						int nWidthEllipse,  // height of ellipse
						int nHeightEllipse  // width of ellipse
						);
BOOL EqualRgn(
			  HRGN hSrcRgn1,  // handle to first region
			  HRGN hSrcRgn2   // handle to second region
			  );

typedef struct _RGNDATAHEADER { 
	DWORD dwSize; 
	DWORD iType; 
	DWORD nCount; 
	DWORD nRgnSize; 
	RECT  rcBound; 
} RGNDATAHEADER, *PRGNDATAHEADER; 


typedef struct _RGNDATA { 
	RGNDATAHEADER rdh; 
	char          Buffer[1]; 
} RGNDATA, *PRGNDATA; 

typedef struct  _XFORM { 
	FLOAT eM11; 
	FLOAT eM12; 
	FLOAT eM21; 
	FLOAT eM22; 
	FLOAT eDx; 
	FLOAT eDy; 
} XFORM, *PXFORM; 

HRGN ExtCreateRegion(
					 CONST XFORM *lpXform,     // transformation data
					 DWORD nCount,             // size of region data
					 CONST RGNDATA *lpRgnData  // region data buffer
					 );

BOOL FillRgn(
			 HDC hdc,    // handle to device context
			 HRGN hrgn,  // handle to region to be filled
			 HBRUSH hbr  // handle to brush used to fill the region
			 );

BOOL FrameRgn(
			  HDC hdc,     // handle to device context
			  HRGN hrgn,   // handle to region to be framed
			  HBRUSH hbr,  // handle to brush used to draw border
			  int nWidth,  // width of region frame
			  int nHeight  // height of region frame
			  );

DWORD GetRegionData(
					HRGN hRgn,            // handle to region
					DWORD dwCount,        // size of region data buffer
					LPRGNDATA lpRgnData   // region data buffer
					);

enum {
	NULLREGION=1, // Region is empty. 
SIMPLEREGION,// Region is a single rectangle. 
COMPLEXREGION,// Region is more than a single rectangle. 
};

int GetRgnBox(
			  HRGN hrgn,   // handle to a region
			  LPRECT lprc  // bounding rectangle
			  );

BOOL InvertRgn(
			   HDC hdc,    // handle to device context
			   HRGN hrgn   // handle to region to be inverted
			   );

int OffsetRgn(  HRGN hrgn,     // handle to region
			  int nXOffset,  // offset along x-axis  
			  int nYOffset   // offset along y-axis
			  );

BOOL PaintRgn(  HDC hdc,    // handle to device context
			  HRGN hrgn   // handle to region to be painted
			  );

BOOL PtInRegion(  HRGN hrgn,  // handle to region
				int X,      // x-coordinate of point  
				int Y       // y-coordinate of point
				);
BOOL RectInRegion(
				  HRGN hrgn,         // handle to region
				  CONST RECT *lprc   // pointer to rectangle
				  );

int SetPolyFillMode(  HDC hdc,            // handle to device context
					int iPolyFillMode   // polygon fill mode
					);

BOOL SetRectRgn(  HRGN hrgn,       // handle to region
				int nLeftRect,   // x-coordinate of upper-left corner of rectangle
				int nTopRect,    // y-coordinate of upper-left corner of rectangle
				int nRightRect,  // x-coordinate of lower-right corner of rectangle
				int nBottomRect  // y-coordinate of lower-right corner of rectangle
				);

#endif//_REGION_H_
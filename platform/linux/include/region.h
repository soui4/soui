#ifndef _REGION_H_
#define _REGION_H_
#include <gdi.h>
#ifndef CONST 
#define CONST const
#endif

int RgnComplexity(HRGN hRgn);

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

HRGN ExtCreateRegion(
					 CONST XFORM *lpXform,     // transformation data
					 DWORD nCount,             // size of region data
					 CONST RGNDATA *lpRgnData  // region data buffer
					 );


DWORD GetRegionData(
					HRGN hRgn,            // handle to region
					DWORD dwCount,        // size of region data buffer
					PRGNDATA lpRgnData   // region data buffer
					);

enum {
	NULLREGION=1, // Region is empty. 
SIMPLEREGION,// Region is a single rectangle. 
COMPLEXREGION,// Region is more than a single rectangle. 
ERROR
};

int GetRgnBox(
			  HRGN hrgn,   // handle to a region
			  LPRECT lprc  // bounding rectangle
			  );


int OffsetRgn(  HRGN hrgn,     // handle to region
			  int nXOffset,  // offset along x-axis  
			  int nYOffset   // offset along y-axis
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
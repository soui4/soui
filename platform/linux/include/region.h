#ifndef _REGION_H_
#define _REGION_H_

#include <platform.h>

int CombineRgn(
   HRGN hrgnDst,
   HRGN hrgnSrc1,
   HRGN hrgnSrc2,
   int  iMode
);

#endif//_REGION_H_
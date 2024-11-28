#ifndef LSTXTBRS_DEFINED
#define LSTXTBRS_DEFINED

#include "lsidefs.h"
#include "pdobj.h"
#include "plocchnk.h"
#include "pobjdim.h"
#include "lstflow.h"
#include "lschnke.h"
#include "txtils.h"
#include "txtobj.h"

typedef struct hyphout
{
	LONG durHyphen;
	LONG dupHyphen;
	LONG durPrev;
	LONG dupPrev;
	LONG durPrevPrev;
	LONG dupPrevPrev;
	LONG ddurDnodePrev;
	LONG ddurDnodePrevPrev;
	LONG durChangeTotal;
	LONG iwchLim;
	LONG dwchYsr;
	WCHAR wchPrev;
	WCHAR wchPrevPrev;
	LONG igindHyphen;
	LONG igindPrev;
	LONG igindPrevPrev;

	GINDEX gindHyphen;
	GINDEX gindPrev;
	GINDEX gindPrevPrev;
} HYPHOUT;

typedef struct ysrinf
{
	WORD kysr;							/* Kind of Ysr - see "lskysr.h" */
	WCHAR wchYsr;						/* YSR char code  */
} YSRINF;


#define FCanBreak(pilsobj,b1, b2) \
	((pilsobj)->plsbrk[(pilsobj)->pilsbrk[(pilsobj)->cBreakingClasses * (b1) + (b2)]].fBreak)

#define FCanBreakAcrossSpaces(pilsobj, b1, b2) \
	((pilsobj)->plsbrk[(pilsobj)->pilsbrk[pilsobj->cBreakingClasses * (b1) + (b2)]].fBreakAcrossSpaces)

BOOL FindNonSpaceBefore(PCLSCHNK rglschnk, LONG itxtobjCur, LONG iwchCur,
									LONG* pitxtobjBefore, LONG* piwchBefore);
BOOL FindNonSpaceAfter(PCLSCHNK rglschnk, DWORD clschnk, LONG itxtobjCur, LONG iwchCur,
									LONG* pitxtobjAfter, LONG* piwchAfter);
BOOL FindPrevChar(PCLSCHNK rglschnk, LONG itxtobjCur, LONG iwchCur,
																LONG* pitxtobjBefore, LONG* piwchBefore);
BOOL FindNextChar(PCLSCHNK rglschnk, DWORD clschnk, LONG itxtobjSpace, LONG iwchSpace,
									LONG* pitxtobjAfter, LONG* piwchAfter);
LSERR CalcPartWidths(PTXTOBJ ptxtobj, LONG dwchLim, POBJDIM pobjdim, LONG* pdur);
LSERR CalcPartWidthsGlyphs(PTXTOBJ ptxtobj, LONG dwchLim, POBJDIM pobjdim, LONG* pdur);
LSERR CheckHotZone(PCLOCCHNK plocchnk, LONG itxtobj, LONG iwch, BOOL* pfInHyphenZone);
LSERR ProcessYsr(PCLOCCHNK plocchnk, LONG itxtobjYsr, LONG dwchYsr, LONG itxtobjPrev, LONG itxtobjPrevPrev, 
														YSRINF ysrinf, BOOL* pfSuccess, HYPHOUT* phyphout);
LSERR GetPbrkinf(PILSOBJ pilsobj, PDOBJ pdobj, BRKKIND brkkind, BREAKINFO** ppbrkinf);


#endif  /* !LSTXTBRS_DEFINED                           */


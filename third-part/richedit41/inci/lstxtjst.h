#ifndef LSTXTJST_DEFINED
#define LSTXTJST_DEFINED

#include "lsidefs.h"
#include "plnobj.h"
#include "lskjust.h"
#include "plocchnk.h"
#include "pposichn.h"
#include "lsgrchnk.h"
#include "lstflow.h"

LSERR AdjustText(
					LSKJUST, 			/* IN: justification type			*/
					LONG,				/* IN: durColumnMax	(from the last
											   tab	position)				*/
					LONG,				/* IN: durTotal	(from the last tab
										   position without trailing area)	*/
					LONG,				/* IN: dup available				*/
					const LSGRCHNK*,	/* IN: Group of chunks				*/
					PCPOSICHNK pposichnkBeforeTrailing,
										/* Information about last
												 cp before trailing area	*/
					LSTFLOW,			/* IN: Text flow					*/
					BOOL,				/* IN: compression?					*/
					DWORD,				/* IN: Number of non-text objects	*/
					BOOL,				/* IN: Suppress wiggling?			*/
					BOOL,				/* IN: Exact synchronization?		*/
					BOOL,				/* IN: fForcedBreak?				*/
					BOOL,				/* IN: Suppress trailing spaces?	*/
					LONG*,				/* OUT: dup of text in chunk		*/
					LONG*,				/* OUT: dup of trailing part		*/
					LONG*,				/* OUT: additional dup of non-text	*/
					DWORD*);			/* OUT: pcExtNonTextObjects			*/

void GetTrailInfoText(
					PDOBJ,				/* IN: pdobj						*/
					LSDCP,				/* IN: dcp in dobj					*/
					DWORD*,				/* OUT: number of trailing spaces	*/
					LONG*);				/* OUT: dur of the trailing area	*/


BOOL FSuspectDeviceDifferent(
					PLNOBJ);				/* IN: Text plnobj	
					*/
/* Returns True if: no dangerous Visi Characters, no non-req hyphens, opt. non-break, opt. break */

BOOL FQuickScaling(
					PLNOBJ,				/* IN: Text plnobj					*/
					BOOL,				/* IN: fVertical					*/
					LONG);				/* IN: durTotal						*/
/* Returns True if: no dangerous Visi Characters, no additional allocations for DOBJ's, durTotal is
	less than accepatable for fast scaling
*/

void QuickAdjustExact(
					PDOBJ*,				/* IN: array of PDOBJs				*/
					DWORD,				/* IN: number of elements in array	*/
					DWORD,				/* IN: number of trailing spaces	*/
					BOOL,				/* IN: fVertical					*/
					LONG*,				/* OUT: dup of text in chunk		*/
					LONG*);				/* OUT: dup of trailing part		*/


LSERR CanCompressText(
					const LSGRCHNK*,	/* IN: Group of chunks				*/
					PCPOSICHNK pposichnkBeforeTrailing,
										/* Information about last
												 cp before trailing area	*/
					LSTFLOW,			/* IN: Text flow					*/
					LONG,				/* IN: dur to compress				*/
					BOOL*,				/* OUT: can compress?				*/
					BOOL*,				/* OUT: actual compression?			*/
					LONG*);				/* OUT: pdurNonSufficient			*/


LSERR DistributeInText(					/* 									*/
					const LSGRCHNK*,	/* IN: group chunk of text			*/
					LSTFLOW,			/* IN: Text flow					*/
					DWORD,				/* IN: Number of non-text objects	*/
				   	LONG,	            /* IN: durToDistribute				*/
					LONG*);				/*OUT: additional dur of  non-text  */

#endif  /* !LSTXTJST_DEFINED                           */


#ifndef BREAK_DEFINED
#define BREAK_DEFINED

#include "lsdefs.h"
#include "fmtres.h"
#include "objdim.h"
#include "plssubl.h"
#include "breakrec.h"
#include "brkpos.h"
#include "brkkind.h"
#include "endres.h"
#include "plsdnode.h"



LSERR BreakGeneralCase(
					  PLSC,				/* IN: LineServices context		*/
					  BOOL,  			/* fHardStop				*/
					  DWORD,			/* IN: size of the output array			*/
					  BREAKREC*,		/* OUT: output array of break records	*/
					  DWORD*,			/* OUT:actual number of records in array*/
					  LSDCP*,			/* OUT: dcpDepend				*/
					  LSCP*,			/* OUT: cpLim					*/
					  ENDRES*,			/* OUT: how line ended			*/
					  BOOL*);			/* OUT fSuccessful: false means insufficient fetch */

LSERR BreakQuickCase(
					  PLSC,			/* IN: LineServices context		*/
					  BOOL,  		/* fHardStop				*/
					  LSDCP*,		/* OUT: dcpDepend				*/
					  LSCP*,		/* OUT: cpLim					*/
					  BOOL* ,		/* OUT: fSuccessful?			*/
					  ENDRES*);		/* OUT: how line ended			*/

LSERR TruncateSublineCore(
							PLSSUBL,		/* IN: subline context			*/
							LONG,			/* IN: urColumnMax				*/
							LSCP*);			/* OUT: cpTruncate 				*/

LSERR FindPrevBreakSublineCore(
							PLSSUBL,		/* IN: subline context			*/
							BOOL,			/* IN: fFirstSubline					*/
							LSCP,			/* IN: truncation cp			*/
							LONG,			/* IN: urColumnMax				*/
							BOOL*,			/* OUT: fSuccessful?			*/
							LSCP*,			/* OUT: cpBreak					*/
							POBJDIM,		/* OUT: objdimSub up to break	*/
							BRKPOS*);		/* OUT: Before/Inside/After		*/


LSERR FindNextBreakSublineCore(
							PLSSUBL,		/* IN: subline context			*/
							BOOL,			/* IN: fFirstSubline					*/
							LSCP,			/* IN: truncation cp			*/
							LONG,			/* IN: urColumnMax				*/
							BOOL*,			/* OUT: fSuccessful?			*/
							LSCP*,			/* OUT: cpBreak					*/
							POBJDIM,		/* OUT: objdimSub up to break	*/			
							BRKPOS*);		/* OUT: Before/Inside/After		*/

LSERR ForceBreakSublineCore(
							PLSSUBL,		/* IN: subline context			*/
							BOOL,			/* IN: fFirstSubline					*/
							LSCP,			/* IN: truncation cp			*/
							LONG,			/* IN: urColumnMax				*/
							LSCP*,			/* OUT: cpBreak					*/
							POBJDIM,		/* OUT: objdimSub up to break	*/			
							BRKPOS*);		/* OUT: Before/Inside/After		*/

LSERR SetBreakSublineCore(
							PLSSUBL,		/* IN: subline context			*/
							BRKKIND,		/* IN: Prev/Next/Force/Imposed	*/					
							DWORD,			/* IN: size of the output array			*/
							BREAKREC*,		/* OUT: output array of break records	*/
							DWORD*);		/* OUT:actual number of records in array*/

LSERR WINAPI SqueezeSublineCore(
							  PLSSUBL,		/* IN: subline context		*/
							  LONG,			/* IN: durTarget			*/
							  BOOL*,		/* OUT: fSuccessful?		*/
							  LONG*);		/* OUT: if nof successful, 
													extra dur 			*/

LSERR  GetMinDurBreaksCore	(PLSC plsc, /* IN: LineServices context		*/
							 LONG* pdurMinInclTrail, /* OUT: min dur between breaks including trailing area */
							 LONG* pdurMinExclTrail);/* OUT: min dur between breaks excluding trailing area */

LSERR  GetLineDurCore		(PLSC plsc, /* IN: LineServices context		*/
							 LONG* pdurInclTrail, /* OUT: dur of line incl. trailing area */
							 LONG* pdurExclTrail);/* OUT: dur of line excl. trailing area */

LSERR FCanBreakBeforeNextChunkCore(PLSC  plsc,  /* IN: LineServices context		*/
								   PLSDNODE plsdn,	/* IN: Last DNODE of the current chunk */
								   BOOL* pfCanBreakBeforeNextChunk); /* OUT: Can break before next chunk ? */



#endif /* BREAK_DEFINED */



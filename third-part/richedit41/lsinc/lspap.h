#ifndef LSPAP_DEFINED
#define LSPAP_DEFINED

#include "lsdefs.h"
#include "plspap.h"
#include "lskjust.h"
#include "lskalign.h"
#include "lsbrjust.h"
#include "lskeop.h"
#include "lstflow.h"

/* ---------------------------------------------------------------------- */

struct lspap
{
	LSCP cpFirst;						/* 1st cp for this paragraph */
	LSCP cpFirstContent;				/* 1st cp of "content" in the para */

	DWORD grpf;							/* line services format flags (lsffi.h)*/

	LONG uaLeft;						/* left boundary for line				*/
	LONG uaRightBreak;					/* right boundary for break */
	LONG uaRightJustify;					/* right boundary for justification */
	LONG duaIndent;
	LONG duaHyphenationZone;

	LSBREAKJUST lsbrj;					/* Break/Justification behavior	*/
	LSKJUST lskj;						/* Justification type */
	LSKALIGN lskal;						/* Alignment type */

	LONG duaAutoDecimalTab;

	LSKEOP lskeop;						/* kind of paragraph ending */
	
	LSTFLOW lstflow;					/* Main text flow direction */

};

typedef struct lspap LSPAP;

#endif /* !LSPAP_DEFINED */

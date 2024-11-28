#ifndef LSQIN_DEFINED
#define LSQIN_DEFINED

#include "lsdefs.h"
#include "heights.h"
#include "lstflow.h"
#include "plsrun.h"
#include "plsqin.h"

typedef struct lsqin			
{
	LSTFLOW	lstflowSubline;
	PLSRUN plsrun;					/* PLSRUN this cp belongs to */
	LSCP cpFirstRun;
	LSDCP dcpRun;
	HEIGHTS	heightsPresRun;			/* In direction lstflowSubline */
	LONG dupRun;					/* In direction lstflowSubline			*/
	LONG dvpPosRun;					/* in direction of lstflowSubline	*/

} LSQIN;


#endif 

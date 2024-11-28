#ifndef LSQSINFO_DEFINED
#define LSQSINFO_DEFINED

#include "lsdefs.h"
#include "heights.h"
#include "lstflow.h"
#include "plsrun.h"
#include "plsqsinf.h"


typedef struct lsqsubinfo			
{
	LSTFLOW	lstflowSubline;
	LSCP cpFirstSubline;
	LSDCP dcpSubline;
 	POINTUV pointUvStartSubline;	/* In coordinate system of main line/subline */
	HEIGHTS	heightsPresSubline;		/* In direction lstflowSubline */
	LONG dupSubline;				/* In direction lstflowSubline			*/


	DWORD idobj;
	PLSRUN plsrun;
	LSCP cpFirstRun;
	LSDCP dcpRun;
 	POINTUV pointUvStartRun;		/* In coordinate system of main line/subline */
	HEIGHTS	heightsPresRun;			/* In direction lstflowSubline */
	LONG dupRun;					/* In direction lstflowSubline			*/
	LONG dvpPosRun;					/* in direction of lstflowSubline	*/

	LONG dupBorderBefore;			/* in direction of lstflowSubline	*/
	LONG dupBorderAfter;			/* in direction of lstflowSubline	*/

 	POINTUV pointUvStartObj;		/* Set by Object, translated to coord system of main line/subline */
	HEIGHTS	heightsPresObj;			/* Set by Object, in direction lstflowSubline */
	LONG dupObj;					/* Set by Object, in direction lstflowSubline			*/


} LSQSUBINFO;

#endif 

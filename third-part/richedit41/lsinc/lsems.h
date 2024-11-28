#ifndef LSEMS_DEFINED
#define LSEMS_DEFINED

#include "lsdefs.h"

typedef struct lsems
{
	LONG em;		/* one em			*/
	LONG em2;		/* half em			*/
	LONG em3;		/* third em			*/
	LONG em4;		/* quater em		*/
	LONG em8;		/* eighth em		*/
	LONG em16;		/* 15/16 of em		*/
	LONG udExp;		/* user defined expansion	*/
	LONG udComp;	/* user defined compression*/
} LSEMS;



#endif /* !LSEMS_DEFINED */



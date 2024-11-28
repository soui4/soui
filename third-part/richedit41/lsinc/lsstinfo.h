#ifndef LSSTINFO_DEFINED
#define LSSTINFO_DEFINED

#include "lsdefs.h"
#include "plsstinf.h"

/* 
 * all strikethrough offsets are relative to the baseline and positive upwards (filled page direction),
 * so normally dvpLowerStrikethroughOffset > 0 and if cNumberOfLines == 2
 * dvpLowerStrikethroughOffset < dvpUpperStrikethroughOffset
 */

struct lsstinfo
{
    UINT  kstbase;						/* base kind of strikethrough  */
    DWORD cNumberOfLines;				/* number of lines: possible values 1,2*/

	LONG dvpLowerStrikethroughOffset ;	/* if NumberOfLines != 2 only data for 
											lower line should be filled in */
	LONG dvpLowerStrikethroughSize;
	LONG dvpUpperStrikethroughOffset;	
	LONG dvpUpperStrikethroughSize;

};
typedef struct lsstinfo LSSTINFO;


#endif /* !LSSTINFO_DEFINED */



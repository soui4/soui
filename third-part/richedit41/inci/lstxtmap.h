#ifndef LSTXTMAP_DEFINED
#define LSTXTMAP_DEFINED

#include "lsidefs.h"
#include "pilsobj.h"
#include "txtobj.h"

LONG IgindFirstFromIwchVeryFirst (PTXTOBJ ptxtobj, LONG igindVeryFirst, LONG iwch);
LONG IgindLastFromIwchVeryFirst (PTXTOBJ ptxtobj, LONG igindVeryFirst, LONG iwch);

LONG IgindFirstFromIwch(PTXTOBJ ptxtobj, LONG iwch); /* iwch can be Lim for ptxtobj */
LONG IgindLastFromIwch(PTXTOBJ ptxtobj, LONG iwch);
LONG IgindBaseFromIgind(PILSOBJ pilsobj, LONG igind); /* Last glyph with non-zero width before igind */
LONG IwchFirstFromIgind(PTXTOBJ ptxtobj, LONG igind);
LONG IwchLastFromIwch(PTXTOBJ ptxtobj, LONG iwch);
LONG IwchPrevLastFromIwch(PTXTOBJ ptxtobj, LONG iwch);
BOOL FIwchOneToOne(PILSOBJ pilsobj, LONG iwch);
BOOL FIwchLastInContext(PILSOBJ pilsobj, LONG iwch);
BOOL FIwchFirstInContext(PILSOBJ pilsobj, LONG iwch);
BOOL FIgindLastInContext(PILSOBJ pilsobj, LONG igind);
BOOL FIgindFirstInContext(PILSOBJ pilsobj, LONG igind);
LONG DcpAfterContextFromDcp(PTXTOBJ ptxtobj, LONG dcp);
void InterpretMap(PLNOBJ pilsobj, LONG iwchFirst, LONG dwch, LONG igindFirst, LONG cgind);

#endif  /* !LSTXTMAP_DEFINED                           */


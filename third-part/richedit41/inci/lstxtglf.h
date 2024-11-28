#ifndef LSTXTGLF_DEFINED
#define LSTXTGLF_DEFINED

#include "lsidefs.h"
#include "pilsobj.h"
#include "lsgrchnk.h"
#include "lsdevice.h"
#include "lstflow.h"

LSERR ApplyGlyphExpand(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LSDEVICE lsdev,
						LONG itxtobjFirst, LONG iwchFirst, LONG itxtobjLast, LONG iwchLast,
						LONG duToDistribute, LONG* rgdu, LONG* rgduGind, LONG* rgduRight, LONG* rgduGright,
						BOOL* pfFullyJustified);


#endif  /* !LSTXTGLF_DEFINED                           */


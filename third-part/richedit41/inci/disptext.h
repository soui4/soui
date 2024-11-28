#ifndef DISPTEXT_DEFINED
#define DISPTEXT_DEFINED

#include "lsidefs.h"
#include "pdobj.h"
#include "pdispi.h"
#include "lskjust.h"

LSERR WINAPI DisplayText(PDOBJ, PCDISPIN);
LSERR WINAPI CalcPresentationText(PDOBJ, LONG, LSKJUST, BOOL);

#endif /* ndef DISPTEXT_DEFINED */

#ifndef LSTXTCMP_DEFINED
#define LSTXTCMP_DEFINED

#include "lsidefs.h"
#include "pilsobj.h"
#include "lsgrchnk.h"
#include "lstflow.h"
#include "mwcls.h"

LSERR FetchCompressInfo(const LSGRCHNK* plsgrchnk, BOOL fFirstOnLine, LSTFLOW lstflow,
							LONG itxtobjFirst, LONG iwchFirst, LONG itxtobjLast, LONG iwchLim,
							LONG durCompressMaxStop, LONG* pdurCompressTotal);
void GetCompLastCharInfo(PILSOBJ pilsobj, LONG iwchLast, MWCLS* pmwcls,
														LONG* pdurCompRight, LONG* pdurCompLeft);

void CompressLastCharRight(PILSOBJ pilsobj, LONG iwchLast, LONG durToAdjustRight);
LSERR ApplyCompress(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, 
					LONG itxtobjFirst, LONG iwchFirst, LONG itxtobjLast, LONG iwchLim, LONG durToCompress);
LSERR ApplyExpand(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, BOOL fScaled,
				LONG itxtobjFirst, LONG iwchFirst, LONG itxtobjLast, LONG iwchLim,
				DWORD cNonTextObjects, LONG durToExpand, LONG* pdurExtNonText, BOOL* pfFinalAdjustNeeded);
void ApplyDistribution(const LSGRCHNK* plsgrchnk, DWORD cNonText,
									   LONG durToDistribute, LONG* pdurNonTextObjects);

#endif  /* !LSTXTCMP_DEFINED                           */


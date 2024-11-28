#ifndef LSTXTSCL_DEFINED
#define LSTXTSCL_DEFINED

#include "lsidefs.h"
#include "pilsobj.h"
#include "lsgrchnk.h"
#include "lskjust.h"
#include "lstflow.h"

void ApplyWysi(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow);
void ApplyNonExactWysi(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow);
void ScaleSpaces(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LONG itxtobjLast, LONG iwchLast);
void ScaleCharSides(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, BOOL* pfLeftSideAffected, BOOL* pfGlyphsDetected);
void ScaleExtNonText(PILSOBJ pilsobj, LSTFLOW lstflow, LONG durExtNonText, LONG* pdupExtNonText);
void GetDupLastChar(const LSGRCHNK* plsgrchnk, LONG iwchLast, LONG* pdupHangingChar);
void ScaleGlyphSides(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow);
void UpdateGlyphOffsets(const LSGRCHNK* plsgrchnk);
void SetBeforeJustCopy(const LSGRCHNK* plsgrchnk);
LSERR FillDupPen(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, LONG itxtobjLast, LONG iwchLast);
LSERR FinalAdjustmentOnPres(const LSGRCHNK* plsgrchnk, LONG itxtobjLast, LONG iwchLast,
			LONG dupAvailable, BOOL fFinalAdjustNeeded, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			LONG* pdupText, LONG* pdupTail);

#endif  /* !LSTXTSCL_DEFINED                           */


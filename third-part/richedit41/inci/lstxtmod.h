#ifndef LSTXTMOD_DEFINED
#define LSTXTMOD_DEFINED

#include "lsidefs.h"

#include "pilsobj.h"
#include "plsems.h"
#include "lsact.h"

void GetChanges(LSACT lsact, PLSEMS plsems, LONG durCur, BOOL fByIsPlus, BYTE* pside, LONG* pddurChange);
void TranslateChanges(BYTE sideRecom, LONG durAdjustRecom, LONG durCur, LONG durRight, LONG durLeft,
														 BYTE* psideFinal, LONG* pdurChange);
void InterpretChanges(PILSOBJ pilsobj, LONG iwch, BYTE side, LONG ddurChange, LONG* pddurChangeLeft, LONG* pddurChangeRight);
void ApplyChanges(PILSOBJ pilsobj, LONG iwch, BYTE side, LONG ddurChange);
void UndoAppliedChanges(PILSOBJ pilsobj, LONG iwch, BYTE side, LONG* pddurChange);
void ApplyGlyphChanges(PILSOBJ pilsobj, LONG igind, LONG ddurChange);
#endif  /* !LSTXTMOD_DEFINED                           */


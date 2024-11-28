#ifndef LSSTRING_DEFINED
#define LSSTRING_DEFINED

#include "lsidefs.h"
#include "pilsobj.h"
#include "plnobj.h"
#include "plsrun.h"
#include "lstflow.h"
#include "txtobj.h"

LSERR GetWidths(PLNOBJ, PLSRUN, LONG, LPWSTR, LSCP, LONG, LONG, LSTFLOW, LONG*, LONG*);
/*
  PLNOBJ (IN) - pointer to lnobj
  PLSRUN (IN) - plsrun
  LONG (IN) - first index in dur array to use
  LPWSTR (IN) - array of characters
  LSCP (IN) - cpFirst
  LONG(IN) - number of characters
  LONG (IN) - width until right margin
  LSTFLOW (IN) - text flow
  LONG* (OUT) - number of characters for which width has been fetched
  LONG* (OUT) - total width of these characters
*/


LSERR FormatString(PLNOBJ, PTXTOBJ, WCHAR*, LONG, LONG*, LONG, LONG);
/* function is called to format  a Local Run
  PLNOBJ (IN) - pointer to lnobj
  PTXTOBJ (IN) - pointer to dobj
  WCHAR* (IN) - pointer to the character array
  LONG (IN) - number of characters
  LONG* (IN) - pointer to the spaces array
  LONG (IN) - number of spaces
  LONG (IN) - width of all chars
*/

LSERR FillRegularPresWidths(PLNOBJ, PLSRUN, LSTFLOW, PTXTOBJ);
/*
  PLNOBJ (IN) - pointer to lnobj
  PLSRUN (IN) - plsrun
  LSTFLOW (IN) - lstflow
  PTXTOBJ (IN) - pointer to dobj
*/


LSERR GetOneCharDur(PILSOBJ, PLSRUN, WCHAR, LSTFLOW, LONG*);
/*
  PILSOBJ (IN) - pointer to the ilsobj 
  PLSRUN (IN) - plsrun
  WCHAR (IN) - character code
  LSTFLOW (IN) - text flow
  LONG* (OUT) - presentation width of the character
*/

LSERR GetOneCharDup(PILSOBJ, PLSRUN, WCHAR, LSTFLOW, LONG, LONG*);
/*
  PILSOBJ (IN) - pointer to the ilsobj 
  PLSRUN (IN) - plsrun
  WCHAR (IN) - character code
  LSTFLOW (IN) - text flow
  LONG (IN) - reference width of the character
  LONG* (OUT) - presentation width of the character
*/

LSERR GetVisiCharDup(PILSOBJ, PLSRUN, WCHAR, LSTFLOW, LONG*);
/*
  PILSOBJ (IN) - pointer to the ilsobj 
  PLSRUN (IN) - plsrun
  WCHAR (IN) - visi character code
  LSTFLOW (IN) - text flow
  LONG* (OUT) - presentation width of the character
*/

LSERR AddCharacterWithWidth(PLNOBJ, PTXTOBJ, WCHAR, LONG, WCHAR, LONG);
/* adds character with specified width in the display list
  PLNOBJ (IN) - pointer to lnobj
  PTXTOBJ (IN) - pointer to dobj
  WCHAR (IN)  - character for rgwchOrig
  LONG (IN) - width in reference units
  WCHAR (IN)  - character for rgwch
  LONG (IN) - width in preview units
*/

void FixSpaces(PLNOBJ, PTXTOBJ, WCHAR);
/*
  PLNOBJ (IN) - pointer to lnobj
  PTXTOBJ (IN) - pointer to dobj
  WCHAR (IN) - VisiSpace code
*/

LSERR AddSpaces(PLNOBJ, PTXTOBJ, LONG, LONG);
/*
  PLNOBJ (IN) - pointer to the lnobj
  PTXTOBJ (IN) - poiter to dobj
  LONG (IN) - reference width of space;
  LONG (IN) - number of trailing spaces to be added 
*/

void FlushStringState(PILSOBJ);
/*
  PILSOBJ (IN) - pointer to the ilsobj 
*/

LSERR IncreaseWchMacBy2(PLNOBJ);
/*
  PLNOBJ (IN) - pointer to the lnobj
*/

#endif /* !LSSTRING_DEFINED                                                */




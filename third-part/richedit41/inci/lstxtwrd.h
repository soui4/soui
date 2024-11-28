#ifndef LSTXTWRD_DEFINED
#define LSTXTWRD_DEFINED

#include "lsidefs.h"
#include "lsgrchnk.h"

void FullPositiveSpaceJustification(
		const LSGRCHNK*,				/* IN: Group chunk to justify   */
		LONG,							/* IN: itxtobjAfterStartSpaces  */
		LONG,							/* IN: iwchAfterStartSpaces     */
		LONG,						 	/* IN: itxtobjLast				*/
		LONG,							/* IN: iwchLast					*/
		LONG*, 							/* IN: rgdu						*/
 		LONG*,							/* IN: rgduGind					*/
		LONG,							/* IN: duToDistribute			*/
		BOOL*);							/* OUT: pfSpaceFound			*/

void NegativeSpaceJustification(
		const LSGRCHNK*,				/* IN: Group chunk to justify   */
		LONG,							/* IN: itxtobjAfterStartSpaces  */
		LONG,							/* IN: iwchAfterStartSpaces     */
		LONG,						 	/* IN: itxtobjLast				*/
		LONG,							/* IN: iwchLast					*/
		LONG*, 							/* IN: rgdu						*/
 		LONG*,							/* IN: rgduGind					*/
		LONG);							/* IN: duSqueeze				*/

#endif  /* !LSTXTWRD_DEFINED                           */


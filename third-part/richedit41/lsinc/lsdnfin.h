#ifndef LSDNFIN_DEFINED
#define LSDNFIN_DEFINED

/* Access routines for contents of DNODES */

#include "lsdefs.h"
#include "plsrun.h"
#include "plsfrun.h"
#include "plschp.h"
#include "pobjdim.h"
#include "pdobj.h"


LSERR WINAPI LsdnFinishRegular(
							  PLSC,				/* IN: Pointer to LS Context */
							  LSDCP,     		/* IN: dcp adopted           */
							  PLSRUN,   		/* IN: PLSRUN  		         */
							  PCLSCHP,  		/* IN: CHP          	     */
							  PDOBJ,    		/* IN: PDOBJ             	 */ 
							  PCOBJDIM);		/* IN: OBJDIM      		     */

LSERR WINAPI LsdnFinishRegularAddAdvancePen(
							  PLSC,				/* IN: Pointer to LS Context */
							  LSDCP,     		/* IN: dcp adopted           */
							  PLSRUN,   		/* IN: PLSRUN  		         */
							  PCLSCHP,  		/* IN: CHP          	     */
							  PDOBJ,    		/* IN: PDOBJ             	 */ 
							  PCOBJDIM,			/* IN: OBJDIM      		     */
							  LONG,				/* IN: durPen				 */
							  LONG,				/* IN: dvrPen				 */
							  LONG);			/* IN: dvpPen 				 */

LSERR WINAPI LsdnFinishByPen(PLSC,				/* IN: Pointer to LS Context */
						   LSDCP, 	    		/* IN: dcp	adopted          */
						   PLSRUN,		   		/* IN: PLSRUN  		         */
						   PDOBJ,	    		/* IN: PDOBJ             	 */ 
						   LONG,    	 		/* IN: dur         		     */
						   LONG,     			/* IN: dvr             		 */
						   LONG);   			/* IN: dvp          	     */

LSERR WINAPI LsdnFinishDeleteAll(PLSC,			/* IN: Pointer to LS Context */
					  			LSDCP);			/* IN: dcp adopted			 */

#endif /* !LSDNFIN_DEFINED */


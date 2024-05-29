#include "HEADERS.h"
#include "sphigslocal.h"

/** Functions creating geometric data 
**/

double *SPH_defPoint (pt,  x, y, z)
double *pt;  /* SHOULD ALREADY POINT TO AN ARRAY OF 3 DOUBLES */
double x, y, z;
{
   pt[0] = x;
   pt[1] = y;
   pt[2] = z;
   return pt;
}


NDC_rectangle SPH_defNDCrectangle (left_x, bottom_y, right_x, top_y)
double left_x, bottom_y, right_x, top_y;
{
   NDC_rectangle rect;

   rect.bottom_left.x = left_x;
   rect.bottom_left.y = bottom_y;
   rect.top_right.x = right_x;
   rect.top_right.y = top_y;
   return rect;
}




/** INTERNAL UTILITIES
The below functions perform operations on "intelligent rectangles":
   SRGP rectangles that "know" whether they are empty.



			CURRENTLY   ALL     COMMENTED   OUT

void
SPH_clip_intelligent_rectangle (ir, sr)
intelligent_rectangle *ir;
srgp__rectangle sr;
{
   if (ir->nonempty) {
      if (GEOM_computeRectIntersection (ir->rect, sr, &(ir->rect)))
	 ir->nonempty = TRUE;
      else
	 ir->nonempty = FALSE;
   }
}



void
SPH_expand_intelligent_rectangle (ir, sr)
intelligent_rectangle *ir;
srgp__rectangle sr;
{
   if (ir->nonempty)
      GEOM_computeRectUnion (ir->rect, sr, &(ir->rect));
   else
      ir->rect = sr;

   ir->nonempty = TRUE;
}
   **/

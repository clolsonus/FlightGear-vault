#include "HEADERS.h"
#define SPHIGS_BOSS
#include "sphigslocal.h"
#include <math.h>




static int
ResizeCallback (int width, int height)
{
   register vi;

   SPH_ndcSpaceSizeInPixels = (width>height) ? height : width;
   for (vi=0; vi<=MAX_VIEW_INDEX; vi++) {
      SPH__updateViewInfo (vi);
      SPH__refresh_viewport_change (vi, SPH_viewTable[vi].pdc_viewport);
   }
}   



/*!*/
void SPH_begin (int width, int height, int color_planes_desired, 
		int shades_per_flexicolor)
{
   if (SPH__enabled)
      SPH__error (ERR_ALREADY_ENABLED);

   SPH__enabled = TRUE;

   /* What should be the size in pixels of 1 NPC unit? */
   SPH_ndcSpaceSizeInPixels = (width>height) ? height : width;

   SRGP_begin ("SPHIGS", width, height, color_planes_desired, FALSE);

   ALLOC_RECORDS (SPH_viewTable, view_spec, MAX_VIEW_INDEX+1);
   ALLOC_RECORDS (SPH__structureTable, structure, MAX_STRUCTURE_ID+1);
   
   BYTES_PER_BITSTRING = ceil((MAX_STRUCTURE_ID+1.0)/8.0);

   SRGP_allowResize (TRUE);
   SRGP_registerResizeCallback (ResizeCallback);

   SPH__init_structure_table();
   SPH__initDefaultAttributeGroup();
   SPH__init_view_table();
  
   SPH_setImplicitRegenerationMode (ALLOWED);

   SPH__initColorTable (shades_per_flexicolor);
}




/*!*/
void
SPH_end()
{
   SRGP_end();
   SPH__enabled = FALSE;
}




/** ROUTINES ALLOWING APPLICATION TO CHANGE SIZES OF TABLES
These may only be called *before* SPHIGS is enabled.
**/

static void CheckNotEnabledYet (void)
{
   if (SPH__enabled)
      SPH__error (ERR_ALREADY_ENABLED);
}


void SPH_setMaxStructureID (int i)
{
   CheckNotEnabledYet();
   MAX_STRUCTURE_ID = i;
}


void SPH_setMaxViewIndex (int i)
{
   CheckNotEnabledYet();
   MAX_VIEW_INDEX = i;
}

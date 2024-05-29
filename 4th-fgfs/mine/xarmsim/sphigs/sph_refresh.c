#include "HEADERS.h"
#include "sphigslocal.h"


static boolean double_buffer_flag = FALSE;
static int cur_screen_width, cur_screen_height;
static int buffer_canvas;



void SPH_setDoubleBufferingFlag (boolean flag)
{
   if (flag == double_buffer_flag)
      return;
      
   if (flag) {
      /* USER IS TURNING ON DOUBLE BUFFERING */
      /* Allocate an SRGP canvas */
      SRGP_inquireCanvasSize (0, &cur_screen_width, &cur_screen_height);
      buffer_canvas = SRGP_createCanvas (cur_screen_width, cur_screen_height);
      /* Initialize to copy of current screen */
      SRGP_useCanvas (buffer_canvas);
      SRGP_copyPixel (0, SRGP_inquireCanvasExtent(0), SRGP_defPoint(0,0));
   }
   else {
      /* USER IS DISABLING DOUBLE BUFFERING */
      /* Deallocate the canvas */
      SRGP_deleteCanvas (buffer_canvas);
   }
   
   double_buffer_flag = flag;
}

static void PREPARE_FOR_DRAWING (void)
{
   SRGP_useCanvas (double_buffer_flag ? buffer_canvas : 0);
}

#define BUFFER_TO_SCREEN   TERMINATE_DRAWING

static void TERMINATE_DRAWING (void)
{
   if (double_buffer_flag) {
      SRGP_useCanvas (0);
      SRGP_copyPixel (buffer_canvas, 
		      SRGP_inquireCanvasExtent(0), SRGP_defPoint(0,0));
   }
}


/** REFRESH ONE VIEW
Should be called only if a redraw (at least) is needed!

IF wireframe rendering (raw or otherwise):
	full retraversal is performed
ELSE:
   IF view has obsolete object list:
	full retraversal
   ELSE IF view has obsolete PDC vertex list:
	PDC vertices are recalculated from the UVN list
   ELSE 
	Redrawing *only* is performed
	
This function resets the obsolescence flags in the view table!
**/

static void
RefreshOneView (int vi)
{
   register root_header *root;
   boolean do_retraverse = FALSE;

   currentViewIndex = vi;
   currentViewSpec = &(SPH_viewTable[currentViewIndex]);
   currentRendermode = currentViewSpec->rendermode;

   /* Ignore this view if no roots are posted to it, or it's disabled */
   if ((currentViewSpec->lowestOverlapNetwork == NULL) || 
       (currentViewSpec->currently_disabled))
     		 return;
   
   if ((currentViewSpec->obsolete_object_list) || 
       (currentRendermode==WIREFRAME_RAW))
      do_retraverse = TRUE;


   /* Clear the viewport area */
   SRGP_setClipRectangle (currentViewSpec->pdc_viewport);
   SRGP_setFillStyle (SOLID);
   SRGP_setColor (currentViewSpec->background_color);
   SRGP_fillRectangle (currentViewSpec->pdc_viewport);
   
   if (do_retraverse) {
      if (currentRendermode > WIREFRAME_RAW)   
         OBJECT__init (currentViewSpec);
      for
         (root = currentViewSpec->lowestOverlapNetwork;
          root != NULL;
          root = root->nextHigherOverlapRoot)
	    SPH__traverse_network_for_display (currentViewSpec, root);
   }


   /* IF rendermode IS NOT RAW, WE NEED TO PROCESS THE OBJECTS. */
   if (currentRendermode > WIREFRAME_RAW) {
      if (do_retraverse) 
         OBJECT__process (currentViewSpec);
      else if (currentViewSpec->obsolete_pdc_vertices)
         SPH__generate_pdc_vertices (currentViewSpec);
      OBJECT__drawAll (currentViewSpec);
   }

   /* TELL SRGP TO REFRESH SCREEN  (no-op unless X11) */
   SRGP_refresh();
   
   /* RESET THE FLAGS */
   currentViewSpec->obsolete_object_list = 
      currentViewSpec->obsolete_pdc_vertices = FALSE;
}


static void
WHITEWASH_RECT (srgp__rectangle r)
{
   SRGP_setClipRectangle (r);
   SRGP_setFillStyle (SOLID);
   SRGP_setColor (SRGP_WHITE);
   SRGP_fillRectangle (r);
}




/** PERFORM REFRESH
Only refreshes views that actually need to be redrawn or retraversed due to
changes in objects or in the view specification
**/
static void
PERFORM_REFRESH (void)
{
   register vi;
   register view_spec *v;

   for (vi=0; vi <= MAX_VIEW_INDEX; vi++) {
      v = &(SPH_viewTable[vi]);
      if ((v->obsolete_object_list) || (v->obsolete_pdc_vertices))
         RefreshOneView (vi);
   }
}



/*!*/
void
SPH_setImplicitRegenerationMode (int mode)
{
   if (mode == SPH_implicit_regeneration_mode)
      return;
      
   if (mode==ALLOWED)
	 PERFORM_REFRESH();
	 
   SPH_implicit_regeneration_mode = mode;
}


/** REGENERATE SCREEN
Called by the application, typically when it has turned off 
   implicit regeneration.
Thus, we can't assume we're just repairing screen damage!
But it does not retraverse or recompute PDCs
   unless a true data change occurred, so it is efficient.
**/
void
SPH_regenerateScreen (void)
{
   register vi;

   PREPARE_FOR_DRAWING();
   for (vi=0; vi <= MAX_VIEW_INDEX; vi++)
       RefreshOneView (vi);
   TERMINATE_DRAWING();

}


/** REPAINT SCREEN
Called  when we are told by the window manager that an
   update should occur to handle damage.
ASSUMES no data change has taken place since screen last painted.
Very efficient if double buffering happens to be on.
CURRENTLY NOT INSTALLED.  Moreover, it's not needed when backing store is on.
**/
void
SPH__repaintScreen (void)
{
   if (double_buffer_flag)
      BUFFER_TO_SCREEN();
   else
      SPH_regenerateScreen();
}



/*!*/
void
SPH_setRenderingMode (int viewindex, int value)
{
   SPH_viewTable[viewindex].rendermode = value;
   
   SPH_viewTable[viewindex].obsolete_object_list = TRUE;
   if (SPH_implicit_regeneration_mode == ALLOWED) {
      PREPARE_FOR_DRAWING();
      RefreshOneView (viewindex);
      TERMINATE_DRAWING();
   }
}




/*!*/
void
SPH__refresh_structure_close (int structID)
{
   register int v;

   PREPARE_FOR_DRAWING();
   for (v=0; v<=MAX_VIEW_INDEX; v++)
     if (TestBit(SPH_viewTable[v].descendent_list, structID)) {
        SPH_viewTable[v].obsolete_object_list = TRUE;
        if (SPH_implicit_regeneration_mode == ALLOWED)
           RefreshOneView (v);
     }
   TERMINATE_DRAWING();
}
      



/*!*/
void SPH__refresh_post (int view)
{
   PREPARE_FOR_DRAWING();
   SPH_viewTable[view].obsolete_object_list = TRUE;
   if (SPH_implicit_regeneration_mode == ALLOWED)
      RefreshOneView (view);
   TERMINATE_DRAWING();
}




/** Refreshing after an unpost
If the view no longer has any posted members, do a whitewash!
**/
void SPH__refresh_unpost (int view)
{
   PREPARE_FOR_DRAWING();
   if (SPH_viewTable[view].highestOverlapNetwork == NULL)
      WHITEWASH_RECT (SPH_viewTable[view].pdc_viewport);
   else {
      SPH_viewTable[view].obsolete_object_list = TRUE;
      if (SPH_implicit_regeneration_mode == ALLOWED)
         RefreshOneView (view);
   }
   TERMINATE_DRAWING();
}



/** Refreshing after a viewport change
Should we whitewash the area that is occupied by the current
viewport for this view?
 Only if:
      	1) There is at least one structure posted to the view.
   AND  2) The intersection of the current vp and the new vp is NOT
           equal to the current vp.
           
 LATER: we will have to repair damage done to overlapping
             innocent-bystander views.
**/
void SPH__refresh_viewport_change (int viewIndex, srgp__rectangle old_viewport_rect)
{
   srgp__rectangle intersection;
   
   PREPARE_FOR_DRAWING();
   if (GEOM_computeRectIntersection(old_viewport_rect, 
    			   	    SPH_viewTable[viewIndex].pdc_viewport,
    				    &intersection))
       /* GEOM SAYS THEY DO INTERSECT */
       if ( ! memcmp(&intersection,&old_viewport_rect, sizeof(srgp__rectangle))) 
          /* AH!  The intersection is equal to the original viewport. */
          /* Don't whitewash! */
          goto dorefresh_vpc;
   
   /* Perform the whitewash! */
   WHITEWASH_RECT (old_viewport_rect);
       
 dorefresh_vpc:
   SPH_viewTable[viewIndex].obsolete_object_list = TRUE;
   if (SPH_implicit_regeneration_mode == ALLOWED)
      RefreshOneView (viewIndex);
      
   TERMINATE_DRAWING();
}


void SPH_disableView (int v)
{
  if (SPH_viewTable[v].currently_disabled)
     return;
     
  WHITEWASH_RECT (SPH_viewTable[v].pdc_viewport);
  SPH_viewTable[v].currently_disabled = TRUE;
}

void SPH_enableView (int v)
{
  if ( ! SPH_viewTable[v].currently_disabled)
     return;
     
  SPH_viewTable[v].currently_disabled = FALSE;
  SPH__refresh_viewport_change (v, SPH_viewTable[v].pdc_viewport);
}


#include "HEADERS.h"
#include  "sphigslocal.h"


static void ConvertLocatorMeasure 
   (locator_measure *lm, srgp__locator_measure *slm)
{
   register i;

   /* SIMPLY COPY THE PORTIONS THAT NEED NO CONVERSION */
   bcopy (slm->button_chord, lm->button_chord, sizeof(slm->button_chord));
   slm->button_of_last_transition = lm->button_of_last_transition;

   /* TRANSLATE srgp/pdc TO npc */
   lm->position[X] = (double)(slm->position.x) / SPH_ndcSpaceSizeInPixels;
   lm->position[Y] = (double)(slm->position.y) / SPH_ndcSpaceSizeInPixels;
   lm->position[Z] = 0.0;

   /* FIND THE RELEVANT VIEW INDEX:
      I will give higher-indexed views first priority, but ignore any
      views that have nothing posted to them.
      But, 0 will be returned if nothing else fits the bill. */
   for (i=MAX_VIEW_INDEX; i>=0; i--)
      if (GEOM_ptInRect(slm->position, SPH_viewTable[i].pdc_viewport))
	 if (SPH_viewTable[i].highestOverlapNetwork)
	    break;
   lm->view_index = i;
}



void SPH_getLocator (locator_measure* lm)
{
   srgp__locator_measure slm;

   SRGP_getLocator (&slm);
   ConvertLocatorMeasure (lm, &slm);
}



void SPH_sampleLocator (locator_measure* lm)
{
   srgp__locator_measure slm;

   SRGP_sampleLocator (&slm);
   ConvertLocatorMeasure (lm, &slm);
}

void SPH_setLocatorMeasure (point position)
{
   srgp__point spt;
   register i;

   /* TRANSLATE npc TO srgp/pdc */
   spt.x = position[X] * SPH_ndcSpaceSizeInPixels;
   spt.y = position[Y] * SPH_ndcSpaceSizeInPixels;

   SRGP_setLocatorMeasure (spt);
}


void SPH_setKeyboardEchoOrigin (point position)
{
   srgp__point spt;
   register i;

   /* TRANSLATE npc TO srgp/pdc */
   spt.x = position[X] * SPH_ndcSpaceSizeInPixels;
   spt.y = position[Y] * SPH_ndcSpaceSizeInPixels;

   SRGP_setKeyboardEchoOrigin (spt);
}


/** CORRELATION ROUTINES
**/

static srgp__rectangle epsilonSquare;


static int Outcode (srgp__point pt)
{
   int outcode;

   if (pt.x > epsilonSquare.top_right.x)
      outcode = 2;
   else if (pt.x < epsilonSquare.bottom_left.x)
      outcode = 1;
   else
      outcode = 0;

   if (pt.y > epsilonSquare.top_right.y)
      outcode += 8;
   else if (pt.y < epsilonSquare.bottom_left.y)
      outcode += 4;

   return outcode;
}


static boolean IsPowerOfTwo[] = {
   FALSE,		/*  0 */
   TRUE,		/*  1 */
   TRUE,		/*  2 */
   FALSE,		/*  3 */
   TRUE,		/*  4 */
   FALSE,		/*  5 */
   FALSE,		/*  6 */
   FALSE,		/*  7 */
   TRUE,		/*  8 */
   FALSE,		/*  9 */
   FALSE,		/* 10 */
   FALSE,		/* 11 */
   FALSE,		/* 12 */
   FALSE,		/* 13 */
   FALSE,		/* 15 */
   TRUE			/* 16 */
};




static boolean
LineCrossesEpsilonSquare (srgp__point pt1, srgp__point pt2)
{
   enum {UNKNOWN, YES, NO} doesCross = UNKNOWN;
   int outcode1, outcode2;


   do {
      outcode1 = Outcode (pt1);
      outcode2 = Outcode (pt2);

      if ((outcode1 & outcode2) > 0) 
	 doesCross = NO;
      else if ((outcode1==0) || (outcode2==0)) 
	 doesCross = YES;
      else if ((IsPowerOfTwo[outcode1]) && (IsPowerOfTwo[outcode2])) {
	 if ((outcode1 ^ outcode2) == 3)
	    doesCross = YES;
	 else if ((outcode1 ^ outcode2) == 12)
	    doesCross = YES;
      }

      if (doesCross == UNKNOWN) {
	 if ((outcode1 & 8) != 0) {
	    pt1.x += ((pt2.x-pt1.x)*(epsilonSquare.top_right.y-pt1.y))/
	       (pt2.y-pt1.y);
	    pt1.y = epsilonSquare.top_right.y;
	 }
	 else if ((outcode1 & 4) != 0) {
	    pt1.x += ((pt2.x-pt1.x)*(epsilonSquare.bottom_left.y-pt1.y))/
	       (pt2.y-pt1.y);
	    pt1.y = epsilonSquare.bottom_left.y;
	 }
	 else if ((outcode1 & 2) != 0) {
	    pt1.y += ((pt2.y-pt1.y)*(epsilonSquare.top_right.x-pt1.x))/
	       (pt2.x-pt1.x);
	    pt1.x = epsilonSquare.top_right.x;
	 }
	 else if ((outcode1 & 1) != 0) {
	    pt1.y += ((pt2.y-pt1.y)*(epsilonSquare.bottom_left.x-pt1.x))/
	       (pt2.x-pt1.x);
	    pt1.x = epsilonSquare.bottom_left.x;
	 }
      }
   }
   while (doesCross == UNKNOWN);

   return (doesCross == YES);
}
      


static boolean 
PtInPolygon (int vertcount, vertex_index *vertindexlist, srgp__point *verts)
{
   int hitcount = 0;
   int codecur, codeprev, codeLOOK;
   int jcur, jprev;
   double m, b;
   srgp__point ptcur, ptprev;
   register i;


   jcur = 0;
   ptcur = verts[vertindexlist[jcur]];
   codecur = Outcode (ptcur);

   for (i=1; i<=vertcount; i++) {
      jprev = jcur;
      codeprev = codecur;
      ptprev = ptcur;
      
      if (i==vertcount)
	 jcur = 0;
      else
	 jcur = i;
      
      ptcur = verts[vertindexlist[jcur]];
      codecur = Outcode (ptcur);

      /* FIRST, CHECK FOR SPECIAL CASE OF A VERTEX ON THE RAY ITSELF. */
      if ((codecur == 2) || (codeprev == 2)) {     /* if  *on* the ray! */
	 if (ptcur.y < ptprev.y)
	    codeLOOK = codecur;
	 else
	    codeLOOK = codeprev;
	 if ((codeLOOK == 2) && (codecur != codeprev))
	    hitcount++;

	 continue;   /* don't go into the switch statement */
      }
      
      switch (codecur & codeprev) {
       case 1:
       case 4:
       case 5:
       case 6:
       case 8:
       case 9:
       case 10:
	 /* TRIVIAL REJECT */
	 break;
       case 2:
	 /* TRIVIAL ACCEPT */
	 hitcount++;
	 break;
       default:
	 if (((codecur | codeprev) & 3) == 0) {
	    /* THE prev->cur LINE ACTUALLY PASSES THROUGH THE pdcpt !!! */
	    /* IGNORE IT!!! */
	 }
	 else {
#           define pdcpt   (epsilonSquare.bottom_left)
	    m = ((double)ptprev.y-ptcur.y) / ((double)ptprev.x-ptcur.x);
	    b = (double)ptprev.y - (m*ptprev.x);
	    if ( (((double)pdcpt.y - b) / m ) >= (double)pdcpt.x)
	       hitcount++;
#           undef pdcpt
	 }
      }
   }
   
   return ((hitcount % 2) == 1);
}



      


static boolean 
PtOnPolygon (int vertcount, vertex_index *vertindexlist, srgp__point *verts)
{
   int jcur, jprev;
   srgp__point ptcur, ptprev;
   register i;


   jcur = 0;
   ptcur = verts[vertindexlist[jcur]];

   for (i=1; i<=vertcount; i++) {
      jprev = jcur;
      ptprev = ptcur;
      
      if (i==vertcount)
	 jcur = 0;
      else
	 jcur = i;
      
      ptcur = verts[vertindexlist[jcur]];

      if (LineCrossesEpsilonSquare (ptcur, ptprev))
	 return TRUE;
   }

   return FALSE;
}






static boolean hit_was_made;
static unsigned short cur_traversal_index, hit_traversal_index;
static pickInformation *INFO;




static boolean TraverseStruct (int structID)
{
   register element *curel;

   pickPathItem *curpitem = &(INFO->path[INFO->pickLevel++]);

   curpitem->structureID = structID;
   curpitem->elementIndex = 0;
   curpitem->pickID = 0;

   for /* FOR EACH ELEMENT IN THE STRUCTURE, IN ORDER: */
      (curel = SPH__structureTable[structID].first_element;
       curel;
       curel = curel->next) {
	 curpitem->elementIndex++;
	 switch (curel->type) {
	  case ELTYP__PICK_ID:
	    curpitem->pickID = curel->data.value;
	    break;
	  case ELTYP__POLYHEDRON:
	  case ELTYP__FILL_AREA:
	    if (cur_traversal_index++ == hit_traversal_index) {
	       /******* !!!!!!!!!!!! C O R R E L A T I O N !!!!!!!!!!!! ******/
	       SRGP_beep();
	       curpitem->elementType = curel->type;
	       return TRUE;
	    }
	    break;
	  case ELTYP__EXECUTE_STRUCTURE:
	    if (TraverseStruct (curel->data.value)) {
	       curpitem->elementType = curel->type;
	       return TRUE;
	    }
	 }
      }

   /* Well, we didn't make a correlate happen in this structure! */
   INFO->pickLevel--;
   return FALSE;
}





void SPH_pickCorrelate (point npc_position, int viewindex, 
			pickInformation *pickinfo)
{
   srgp__point pdcpoint;
   register obj *curobj;
   root_header *root;
   view_spec *vs = &(SPH_viewTable[viewindex]);
   boolean (*correlator)();


   /* IF THE POINT IS OUTSIDE ALL KNOWN VIEWPORTS */
   if (viewindex < 0) {
      pickinfo->pickLevel = 0;
      return;
   }
      
   
   if (vs->obsolete_object_list || vs->obsolete_pdc_vertices)
      SPH__error (ERR_PICK_CORR_WITH_UNSYNCH_DISPLAY);

   if (vs->rendermode == WIREFRAME_RAW)
      SPH__error (ERR_PICK_CORR_WITH_WIRE_RAW);


   /********* CONVERT NPC PICK POINT TO srgp COORDS. */
   pdcpoint.x = npc_position[X] * SPH_ndcSpaceSizeInPixels;
   pdcpoint.y = npc_position[Y] * SPH_ndcSpaceSizeInPixels;

   /********* SET THE EPSILON SQUARE STATIC
    The way it is set depends upon the type of correlation that will be done:
    for PtInPolygon, it should be a square of 0 area. */
   epsilonSquare.bottom_left = pdcpoint;
   epsilonSquare.top_right = pdcpoint;
   correlator = PtInPolygon;
   if (vs->rendermode == WIREFRAME) {
      epsilonSquare.bottom_left.x--;
      epsilonSquare.bottom_left.y--;
      epsilonSquare.top_right.x++;
      epsilonSquare.top_right.y++;
      correlator = PtOnPolygon;
   }


   /********** FIRST, SCAN BACKWARDS THROUGH THE OBJECT LIST. 
     Optimization bug!  We can't scan backwards, because the object
     list currently is singly linked.  All I can do is scan forwards
     and I have to examine EVERYTHING in the list unconditionally!
     */
   hit_was_made = FALSE;   /* ...until proven guilty */

   for (curobj = vs->objects;
	curobj != NULL;
	curobj = curobj->next) {
      if (correlator
		  (curobj->data.face.numPoints,
		  curobj->data.face.points,
		  vs->pdcVertices)) {
	 /********** !!!!!!!!!!!  H I T !!!!!!!!!!!!! ************/
	 hit_was_made = TRUE;
	 hit_traversal_index = curobj->traversal_index;
      }
   }


   if (FALSE == hit_was_made) {
      pickinfo->pickLevel = 0;
      return;
   }


   /***** NOW, WE KNOW THE UNIQUE TRAVERSAL INDEX OF THE PRIMITIVE SELECTED.
     We must traverse the network speedily (honoring only the
     execution elements) looking for the primitive having that index, keeping
     a stack with the pick information in it.
     */

   cur_traversal_index = 0;
   INFO = pickinfo;

   for
      (root = vs->lowestOverlapNetwork;
       root != NULL;
       root = root->nextHigherOverlapRoot) {
	 INFO->pickLevel  = 0;
	 if (TraverseStruct (root->root_structID))
	    break;
      }
}

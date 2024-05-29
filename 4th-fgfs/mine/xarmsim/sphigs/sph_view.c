#include "HEADERS.h"
#include "sphigslocal.h"
#include <stdio.h>

#define THE_VIEW    SPH_viewTable[viewIndex]

#define FEPS         (1e-12)
#define FZERO(N)      ((N) < FEPS && (N) > -FEPS)
#define FEQUAL(A,B)      (((A) - (B)) < FEPS && ((A) - (B)) > -FEPS)

#define PI            3.1415926535



/*!*/
void 
SPH_evaluateViewOrientationMatrix 
   (point view_ref_point,
    vector view_plane_normal, vector view_up_vector,
    matrix vo_matrix /*varparm*/ )
{
   vector tempvec, up_v, normal_v, proj_up_v, right_v;
   matrix trans, rot;
   double dot, tmp1;


   /* FIND THE UNIT VERSION OF THE up VECTOR */
   MAT3_COPY_VEC(up_v, view_up_vector);
   MAT3_NORMALIZE_VEC(up_v, tmp1);
   if (tmp1 == 0.0)
      SPH__error (ERR_MAT3_PACKAGE, "up vector has zero length");

   /* FIND THE UNIT VERSION OF THE view plane normal VECTOR */
   MAT3_COPY_VEC(normal_v, view_plane_normal);
   MAT3_NORMALIZE_VEC(normal_v, tmp1);
   if (tmp1 == 0.0)
      SPH__error (ERR_MAT3_PACKAGE, "View plane normal has zero length");

   /* Find the projection of the up vector on the film plane */
   dot = MAT3_DOT_PRODUCT(up_v, normal_v);
   if (dot == 1.0 || dot == -1.0)
      SPH__error (ERR_MAT3_PACKAGE, "Up vector and view plane normal are the same");

   MAT3_LINEAR_COMB(proj_up_v, 1.0, up_v, -dot, normal_v);
   MAT3_NORMALIZE_VEC(proj_up_v, tmp1);

   /* Find the vector that goes to the right on the film plane */
   MAT3cross_product(right_v,    normal_v, proj_up_v);


/*!*/
   /********* STEP 1 OF FOLEY CHAPTER VIEW.5.2 */
   /* Translate view reference point to origin */
   MAT3_SCALE_VEC(tempvec, view_ref_point, -1);
   MAT3translate(trans, tempvec);


   /********* STEP 2 OF FOLEY CHAPTER VIEW.5.2 */
   /* Rotate so normal lies on z-axis and proj_up is on y-axis */
   /* Remember: right-hand rule is in effect. */
   MAT3identity(rot);
   rot[0][2] = normal_v[X];
   rot[1][2] = normal_v[Y];
   rot[2][2] = normal_v[Z];

   rot[0][0] = -right_v[X];
   rot[1][0] = -right_v[Y];
   rot[2][0] = -right_v[Z];

   rot[0][1] = proj_up_v[X];
   rot[1][1] = proj_up_v[Y];
   rot[2][1] = proj_up_v[Z];


   /*************** PRODUCE THE FINAL PRODUCT: TRANS x ROT */
   MAT3mult(vo_matrix,   trans, rot);
}




/*!*/


static double most_recent_F, most_recent_B;



void
SPH_evaluateViewMappingMatrix
   (double umin, double umax, double vmin, double vmax,
    int proj_type,  /* ORTHOGRAPHIC or PERSPECTIVE */
    point proj_ref_point,
    double front_plane_dist, double back_plane_dist,
    double viewport_minx, 
    double viewport_maxx, 
    double viewport_miny, 
    double viewport_maxy,
    double viewport_minz, 
    double viewport_maxz,
    matrix vm_matrix /*varparm*/ )
{
   matrix step3_matrix, step4_matrix, step3and4_matrix, step5_matrix,
          step6_matrix, sklar_matrix;
   MAT3hvec vrp;

   MAT3_SET_HVEC (vrp, 0.0, 0.0, 0.0, 1.0);

/*!*/

   /**** First, we test for "garbage in" ****/
   if (proj_ref_point[Z] <= 0.0)
      SPH__error (ERR_BAD_VIEWING_PARAMETER, 
		  "proj_ref_point[Z] is zero or negative\n");
   if (front_plane_dist >= proj_ref_point[Z])
      SPH__error (ERR_BAD_VIEWING_PARAMETER, 
		 "front clip plane lies behind or on the PRP\n");
   if (back_plane_dist >= front_plane_dist)
      SPH__error (ERR_BAD_VIEWING_PARAMETER, 
		 "back clip plane is closer to PRP than the front clip plane");


   /**** Save the F and B because currently there's a bug in ORTHO mode;
         we can't backcalc the F and B from the vm matrix in ORTH mode! ***/
   if (proj_type == ORTHOGRAPHIC) {
      most_recent_F = front_plane_dist;
      most_recent_B = back_plane_dist;
   }

   /********* STEP 3 OF FOLEY CHAPTER VIEW.5.2 */
   /* Only to be done for PERSPECTIVE. */
   /* Translate so the PRP (center of projection) is at the origin. */
   if (proj_type == PERSPECTIVE) {
      vector tempvec;

      MAT3_SCALE_VEC(tempvec, proj_ref_point, -1);
      MAT3translate(step3_matrix, tempvec);
#ifdef DEBUG
      MAT3print_formatted
	 (step3_matrix, stdout, 0, "Foley step 3  ", 0, 0); 
#endif
      
      /* update vrp */
      MAT3_ADD_VEC(vrp,  vrp, tempvec);
   }
   else
      MAT3identity (step3_matrix);



/*!*/   
   /********* STEP 4 OF FOLEY CHAPTER VIEW.5.2 */
   /* Shear so that the center line of the view volume lies on z-axis */
   {
      vector dop_v;   /* direction of projection */

      if (proj_type == PERSPECTIVE) {
	 dop_v[0] = vrp[0] + (umax+umin)/2;
	 dop_v[1] = vrp[1] + (vmax+vmin)/2;
	 dop_v[2] = vrp[2];
      }
      else {
	 dop_v[0] = (umax+umin)/2 - proj_ref_point[0];
	 dop_v[1] = (vmax+vmin)/2 - proj_ref_point[1];
	 dop_v[2] = 0.0 - proj_ref_point[2];
      }

      MAT3shear(step4_matrix, -dop_v[X] / dop_v[Z], -dop_v[Y] / dop_v[Z]);
#ifdef DEBUG
      MAT3print_formatted
	    (step4_matrix, stdout, 0, "Shear  ", 0, 0); 
#endif
   }



   

/*!*/   
   /* LET'S COMPUTE THE INTERMEDIATE RESULT: step3 times step4 */
   MAT3mult (step3and4_matrix, step3_matrix, step4_matrix);
#ifdef DEBUG
   MAT3print_formatted
      (step3and4_matrix, stdout, 0, "Foley step 3 x step 4  ", 0, 0); 
#endif


/*!*/
   /* STEPS 5 and 6 DIFFER BETWEEN THE TWO TYPES OF PROJECTIONS. */

   if (proj_type == ORTHOGRAPHIC) {
      vector tempvec;
      MAT3_SET_VEC (tempvec,  
		    (umax+umin)/(-2.0),
		    (vmax+vmin)/(-2.0),
		    0.0);			/******** F=0.0 */
      MAT3translate(step5_matrix, tempvec);
#ifdef DEBUG
      MAT3print_formatted
	    (step5_matrix, stdout, 0, "ORTHO step Tpar w/ F=0  ", 0, 0); 
#endif

      MAT3_SET_VEC (tempvec,  
		    2.0/(umax-umin),
		    2.0/(vmax-vmin),
		    1.0/2.0);			/******** B=2.0 */
      MAT3scale(step6_matrix, tempvec);
#ifdef DEBUG
      MAT3print_formatted
	    (step6_matrix, stdout, 0, "ORTHO step Spar w/ B=2  ", 0, 0); 
#endif
   }
   else {
/*!*/   
   /********* STEP 5 OF FOLEY CHAPTER VIEW.5.2 */
   /* Get it to canonical perspective volume. */
      MAT3hvec vrp_prime;
      vector scale_vec;
      double zmin;

      /* FIRST, find VRP' (VRP after steps 3 and 4) */
      MAT3mult_hvec (vrp_prime, vrp, step4_matrix, FALSE);
#ifdef DEBUG
      printf ("vrp_prime is found to be: (%f %f %f %f)\n",
	      vrp_prime[0], vrp_prime[1], vrp_prime[2], vrp_prime[3]);
#endif

      /* Compute a scale matrix ala Foley */
#     define F front_plane_dist
#     define B back_plane_dist
      MAT3_SET_VEC(scale_vec,
		   (2.0)*vrp_prime[Z]/((umax-umin)*(vrp_prime[Z]+B)),
		   (2.0)*vrp_prime[Z]/((vmax-vmin)*(vrp_prime[Z]+B)),
		   1.0/(vrp_prime[Z]+B));

      MAT3scale(step5_matrix, scale_vec);
#ifdef DEBUG
      MAT3print_formatted
	    (step5_matrix, stdout, 0, "Foley step 5 w/ B=2  ", 0, 0); 
#endif

/*!*/
   /************ STEP 6 OF FOLEY */
   /* Get it to canonical parallel volume. */
   
   
      zmin = (vrp_prime[Z]+F)/(vrp_prime[Z]+B);
#ifdef DEBUG
      fprintf (stderr, "VIEW MAPPING MATRIX CALC'D FOR    F=%f  B=%f\n", F,B);
      fprintf (stderr, "zmin is calculated as %f\n", zmin);
#endif
      MAT3zero (step6_matrix);
      step6_matrix[0][0] = 1.0;
      step6_matrix[1][1] = 1.0;
      step6_matrix[2][2] = 1.0 / (1.0 - zmin);
      step6_matrix[3][2] = (0.0 - zmin) / (1.0 - zmin);
      step6_matrix[2][3] = 1.0;
#ifdef DEBUG
      MAT3print_formatted
	    (step6_matrix, stdout, 0, "Foley step 6 w/ F=0  ", 0, 0); 
#endif
   }

  
/*!*/   
   /******** DO SOME MERGING */
   MAT3mult(vm_matrix,   step3and4_matrix, step5_matrix);
   MAT3mult(vm_matrix,   vm_matrix, step6_matrix);
#ifdef DEBUG
   MAT3print_formatted
         (vm_matrix, stdout, 0, "All of Foley's steps  ", 0, 0); 
#endif


/*!*/
   /********* SKLAR'S STEP */
   /* To bring the canonical (-1 to +1) to the NPC viewport.
    * This transformation will produce NDC coordinates (if you ignore z).
    */
   {
      matrix scalemat, transmat;
      vector scale_vec, trans_vec;

      MAT3_SET_VEC(scale_vec,
		   (viewport_maxx - viewport_minx)/2,
		   (viewport_maxy - viewport_miny)/2,
		   1.0);
      MAT3scale(scalemat,   scale_vec);
      MAT3_SET_VEC(trans_vec,
		   (viewport_minx+viewport_maxx)/2,
		   (viewport_miny+viewport_maxy)/2,
		   0);
      MAT3translate(transmat,   trans_vec);

      MAT3mult(sklar_matrix,   scalemat, transmat);
   }

   MAT3mult(vm_matrix,   vm_matrix, sklar_matrix);
   
#ifdef DEBUG
   MAT3print_formatted
         (vm_matrix, stdout, 0, "Final matrix  ", 0, 0); 
#endif
}





/** SPH_translateWCtoPDC
Translates a single point.
**/
void
SPH__translateWCtoPDC (int viewIndex, MAT3hvec inpt, MAT3hvec outpt)
{
   MAT3mult_hvec (outpt, inpt, THE_VIEW.cammat, TRUE);
}



/** SPH_translateMCtoPDC
Translates a single point from MC to PDC by 
multiplying by the TOTAL transform matrix.
**/
void
SPH__translateMCtoPDC (matrix xformmat, MAT3hvec inpt, MAT3hvec outpt)
{
   MAT3mult_hvec (outpt, inpt, xformmat, TRUE);
}





/** SPH_update_view_info
Upon entry, the vo_matrix, vm_matrix, and viewport for the given view can
be assumed correct, but the pdc-viewport and the final viewing matrix
are obsolete and must be recomputed.
**/

void SPH__updateViewInfo (int viewIndex)
{
   vector scale_vec;
   matrix scale_mat;


   THE_VIEW.pdc_viewport.bottom_left.x =
       THE_VIEW.viewport.bottom_left.x * SPH_ndcSpaceSizeInPixels;
   THE_VIEW.pdc_viewport.bottom_left.y =
       THE_VIEW.viewport.bottom_left.y * SPH_ndcSpaceSizeInPixels;
   THE_VIEW.pdc_viewport.top_right.x =
      (THE_VIEW.viewport.top_right.x * SPH_ndcSpaceSizeInPixels) - 1;
   THE_VIEW.pdc_viewport.top_right.y =
      (THE_VIEW.viewport.top_right.y * SPH_ndcSpaceSizeInPixels) - 1;

   /* ONE FINAL STEP.  The transformation after the view-orientation
      and the view-mapping produces NDC coords (NPC, to PHIGS-gurus).
      We need to scale that by the pixel-size of our NDC space to
      get PDCs.
      The Z-coord of the PDC coord is being scaled quite substantially
         so when we turn it into an integer via truncation it doesn't
	 lose its meaning.  It will be used for HLHSR.
    */
   MAT3mult (THE_VIEW.cammat, THE_VIEW.vo_matrix, THE_VIEW.vm_matrix);
   MAT3_SET_VEC(scale_vec,  
		SPH_ndcSpaceSizeInPixels*1.0,
		SPH_ndcSpaceSizeInPixels*1.0,
		1000.0);
   MAT3scale (scale_mat, scale_vec);
   MAT3mult (THE_VIEW.cammat,    THE_VIEW.cammat, scale_mat);
}

   


/** SPH_init_view_table
**/
void SPH__init_view_table (void)
{
   register viewIndex=0;
   register view_spec *v;
   point vrp, prp;
   vector vpn, vupv;
   matrix vo_matrix, vm_matrix;

   SPH_evaluateViewOrientationMatrix 
      (SPH_defPoint(vrp,  0.0,0.0,0.0),
       SPH_defPoint(vpn,  0.0,0.0,1.0),
       SPH_defPoint(vupv, 0.0,1.0,0.0),
       vo_matrix);

   SPH_evaluateViewMappingMatrix
      (0.0,1.0,  0.0,1.0,
       ORTHOGRAPHIC,
       SPH_defPoint(prp,  0.5,0.5,1.0),
       0.99, -50000.0,
       0.0,1.0, 0.0,1.0, 0.0, 1.0,
       vm_matrix);

   bzero (SPH_viewTable, sizeof(view_spec)*(MAX_VIEW_INDEX+1));

#if defined(THINK_C) || defined(__MSDOS__)
#define HUGE ((double)9999999.0)
#endif
   v = SPH_viewTable;
   for (viewIndex=0; viewIndex<=MAX_VIEW_INDEX; viewIndex++) {
      SPH_setViewRepresentation
	 (viewIndex,
	  vo_matrix, vm_matrix,
	  0.0,1.0, 0.0,1.0, 0.0,1.0);
      ClearBitstring (&v->descendent_list);
      v->rendermode = WIREFRAME;
      v->background_color = SRGP_WHITE;
      MAT3_SET_VEC (v->uvn_point_light_source, 0.0, HUGE, HUGE);
      v++;
   }
}



/** SPH_setViewBackgroundColor
On a per-view basis, the background color can be set. 
Warning: this does NOT cause a refresh.  The new color will only
   be seen upon the next operation causing a refresh.
**/

void SPH_setViewBackgroundColor (int viewIndex, int color)
{
   if ( ! IS_LEGAL_COLOR_INDEX(color))
      color = 0;
   SPH_viewTable[viewIndex].background_color = color;
}
   



/** SPH_setViewRepresentation
**/
void 
SPH_setViewRepresentation 
   (int viewIndex,
    matrix vo_matrix, matrix vm_matrix,
    double vp_minx, double vp_maxx, 
    double vp_miny, double vp_maxy,
    double vp_minz, double vp_maxz)
{
   srgp__rectangle oldpdcviewport;
   MAT3hvec testvec = {0.0, 0.0, 0.0, 1.0}, resultvec;
   matrix inverse_vm_matrix;


   SPH_check_system_state;
   SPH_check_no_open_structure;
   SPH_check_view_index;

   SPH_check_rectangle (vp_minx,vp_miny,vp_maxx,vp_maxy);

   oldpdcviewport = THE_VIEW.pdc_viewport;
   THE_VIEW.viewport = 
      SPH_defNDCrectangle (vp_minx, vp_miny, vp_maxx, vp_maxy);
       	   
   MAT3copy (THE_VIEW.vo_matrix,   vo_matrix);
   MAT3copy (THE_VIEW.vm_matrix,   vm_matrix);
   
   MAT3invert (inverse_vm_matrix,  vm_matrix);
   
   MAT3mult_hvec (resultvec, testvec, inverse_vm_matrix, TRUE);
   THE_VIEW.frontPlaneDistance = resultvec[Z];
  
#ifdef DEBUG
   fprintf (stderr, "Backcalcing the F from z=0 results in %f\n", resultvec[Z]);
#endif

   testvec[Z] = 1.0;
   MAT3mult_hvec (resultvec, testvec, inverse_vm_matrix, TRUE);
   THE_VIEW.backPlaneDistance = resultvec[Z];
   
#ifdef DEBUG
   fprintf (stderr, "Backcalcing the B from z=1 results in %f\n", resultvec[Z]);
#endif

   if (THE_VIEW.frontPlaneDistance <= THE_VIEW.backPlaneDistance) {
      /**** WHOA!  Bad news!  Can't backcalc (happens in ORTHO matrices) ***/
      /* We'll have to take a not-so-wild guess. */
      THE_VIEW.frontPlaneDistance = most_recent_F;
      THE_VIEW.backPlaneDistance = most_recent_B;
   }

   SPH__updateViewInfo (viewIndex);
   SPH__refresh_viewport_change (viewIndex, oldpdcviewport);

}



 
void
SPH_setViewPointLightSource (int viewIndex, double u, double v, double n)
{
   MAT3_SET_VEC (THE_VIEW.uvn_point_light_source,
		 u,v,n);
   SPH_regenerateScreen();
}

#include "HEADERS.h"
/**********************
                      *
 File                 * intensity.c
                      * calculates intensity for each object
                      *
                      * takes the list of objects and calculates Gouraud
                      *    intensities (flat shading)
                      * 
                      * changes objects intensity
                      * needs to be called after clip (to get the bounds)
		      *
                      *********************************************************/

/**********************
                      *
 Includes	      *
                      *
                      *********************************************************/

#include <stdio.h>
#include "sphigslocal.h"
#include <assert.h>


/**********************
                      *
 Function	      * calc_intensity
                      *
                      * calculates the intensity for each object
		      *
                      *********************************************************/
void SPH__calc_intensity (view_spec *vs)
{
     MAT3hvec negatedVector;
     MAT3hvec centerFace;
     MAT3hvec oddNormal;
     register obj *scanObj;
     double temp;
     
     scanObj = vs->objects;
     
     while (scanObj != NULL) {
	  if (scanObj->type == objFace) {
	       centerFace[X] = ( scanObj->max[X] + scanObj->min[X] ) / 2;
	       centerFace[Y] = ( scanObj->max[Y] + scanObj->min[Y] ) / 2;
	       centerFace[Z] = ( scanObj->max[Z] + scanObj->min[Z] ) / 2;
	       centerFace[3] = 1;
	       MAT3_SUB_VEC (negatedVector, 
			     vs->uvn_point_light_source, centerFace );
	       negatedVector[3] = 1;
	       MAT3_NORMALIZE_VEC( negatedVector, temp );
	       MAT3_COPY_VEC ( oddNormal, scanObj->normal );
	       oddNormal[3] = 1;
	       MAT3_NORMALIZE_VEC( oddNormal, temp );
	       scanObj->intensity = MAT3_DOT_PRODUCT(negatedVector, oddNormal );
	  }
	  scanObj = scanObj->next;
     }
}

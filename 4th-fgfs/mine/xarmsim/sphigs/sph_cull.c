#include "HEADERS.h"
/**********************
                      *
 File                 * cull.c
                      * backface culling unit
                      *
                      * changes p_normals in objects
                      *
                      *********************************************************/

/**********************
                      *
 Includes	      *
                      *
                      *********************************************************/

#include <stdio.h>
#include <assert.h>

#include "sphigslocal.h"

/**********************
                      *
 Function	      * cull
                      *
                      * reduces the number of nodes in the objects list
                      * through backface culling with the perspectivized
                      * normals
                      *
                      *********************************************************/

void SPH__cull (view_spec *vs)
{
     obj *			current;	/* current object */
     obj *			prev;		/* keep the previous for node removal */
     MAT3hvec			vec1;		/* subtracted vector 1 */
     MAT3hvec			vec2;		/* subtracted vector 2 */
     MAT3hvec			canon_normal;	/* canonized normal to the face */
     double			length;		/* used in normalize */
     
     assert( vs != NULL );
     
     if( vs->objects == NULL )
	  return;
     
     prev = NULL;
     current = vs->objects;
     
     prev = NULL;
     while( current != NULL ) {
	  
	  if (current->type == objFace) {
	       
	       /* Calculate normal */
	       MAT3_SUB_VEC (vec1,
			     vs->npcVertices[current->data.face.points[1]],
			     vs->npcVertices[current->data.face.points[0]]);
	       MAT3_SUB_VEC (vec2,
			     vs->npcVertices[current->data.face.points[2]],
			     vs->npcVertices[current->data.face.points[1]]);
	       vec1[3] = 1;
	       vec2[3] = 1;
	       MAT3_NORMALIZE_VEC( vec1, length );
	       MAT3_NORMALIZE_VEC( vec2, length );
	       MAT3cross_product (canon_normal, vec1, vec2);
	       canon_normal[3] = 1;
	       MAT3_NORMALIZE_VEC( canon_normal, length );
	       MAT3_COPY_VEC( current->p_normal, canon_normal );
	       current->p_normal[3] = 1;
	       
	       if( current->p_normal[Z] <= 0 ) {
		    if( prev == NULL )
			 vs->objects = current->next;
		    else
			 prev->next = current->next;
	       } else
		    prev = current;
	       
	       current = current -> next;
	  }
     }
}

#include "HEADERS.h"
/**********************
                      *
 File                 * gen_pdc_verts.c
                      * calculates pdc points from npc vertices
                      *
                      * changes pdc points
                      * assumes map_to_canon was called before
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
 Function	      * generate_pdc_vertices
                      *
                      * calculates the pdc points from the npc vertices
		      *
                      *********************************************************/
void  
SPH__generate_pdc_vertices (view_spec *vs)
{
     MAT3hvec		result_vec;
     MAT3hvec *		scanNPCvertex;
     srgp__point *	scanPDCvertex;
     MAT3vec		tempVertex;
     int		count;

     scanNPCvertex = vs->npcVertices;

     if (vs->pdcVertices)
	  free (vs->pdcVertices);
     
     vs->pdcVertices = 
	(srgp__point*) malloc (vs->vertexCount * sizeof(srgp__point));
     assert( vs->pdcVertices != NULL );
     
     scanPDCvertex = vs->pdcVertices;
     
     for (count = 0; count < vs->vertexCount; count++) {
	  tempVertex[0] = (* scanNPCvertex)[0] * SPH_ndcSpaceSizeInPixels;
	  tempVertex[1] = (* scanNPCvertex)[1] * SPH_ndcSpaceSizeInPixels;
	  scanPDCvertex->x = irint( tempVertex[0] );
	  scanPDCvertex->y = irint( tempVertex[1] );
	  scanPDCvertex++;
	  scanNPCvertex++;
     }
}

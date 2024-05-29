#include "HEADERS.h"
   /**********************
    *
    File                 * map_to_canon.c
    * calculates npc vertices from uvn vertices
    *
    * changes npc points
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
    Function	      * map_to_canon
    *
    * calculates the npc vertices from the uvn vertices
    * negates the z coordinate to keep the same handedness
    *    as the uvn space
    *
    *********************************************************/
   void SPH__map_to_canon (view_spec *vs)
{
   int		count;
   
   /* Allocate space for the npc coords to be generated in */
   
   if (vs->npcVertices)
      free (vs->npcVertices);
   ALLOC_RECORDS (vs->npcVertices, MAT3hvec, vs->vertexArraySize);
   
   /* Map using the view-mapping matrix to get NDC coords */
   
   for (count = 0; count < vs->vertexCount; count++) {
      MAT3mult_hvec
	 (vs->npcVertices[count], vs->uvnVertices[count], 
	  vs->vm_matrix, 1);
      (vs->npcVertices[count])[Z] *= -1;
   }
}

#include "HEADERS.h"
#include <stdio.h>
#include "sphigslocal.h"
#include <string.h>

#ifdef THINK_C
#include <stdlib.h>
#endif

/** POLYHEDRON
The routine in this file creates the actual polyhedron record.
The structure database points to the memory allocated here.
**/




void
SPH__freePolyhedron (POLYHEDRON *poly)
{
   facet *fptr = poly->facet_list;
   int fcount = poly->facet_count;

   free (poly->vertex_list);

   while (fcount--) {
      free (fptr->vertex_indices);
      fptr = fptr++;
   }


   free (poly->facet_list);
   free (poly);
}




/*!*/
void
SPH_polyhedron
   (int numverts, int numfacets, point *verts, vertex_index *facets)
{
   register i;
   register vertex_index *viptr;
   vertex_index *start_viptr;
   facet *fptr;
   POLYHEDRON *newpoly;
   vector vec1, vec2;


   ALLOC_RECORDS (newpoly, POLYHEDRON, 1);
   ALLOC_RECORDS (newpoly->vertex_list, MAT3hvec, numverts);
   ALLOC_RECORDS (newpoly->facet_list, facet, numfacets);

   newpoly->vertex_count = numverts;
   newpoly->facet_count = numfacets;

   /* COPY VERTICES, TRANSFORMING TO HVERTS. */
   for (i=0; i<numverts; i++)
      MAT3_SET_HVEC (newpoly->vertex_list[i] ,
		     verts[i][0], verts[i][1], verts[i][2], 1.0);

   /* SEPARATE LONG LIST OF VERTEX INDICES INTO INDIVIDUAL FACET DEF'S */
   fptr = newpoly->facet_list;
   viptr = facets;
   while ((numfacets--) > 0) {
      /* Calculate number of vertex indices in the next list */
      start_viptr = viptr;
      while (*(++viptr) != (-1));   /* scan forward to find next sentinel */
      fptr->vertex_count = viptr - start_viptr;

      /* Allocate space for them, and copy into the new memory */
      ALLOC_RECORDS (fptr->vertex_indices, vertex_index, fptr->vertex_count);
      bcopy (start_viptr, fptr->vertex_indices, 
	         sizeof(vertex_index) * fptr->vertex_count);

      /* Calculate normal */
      MAT3_SUB_VEC (vec1, newpoly->vertex_list[fptr->vertex_indices[1]],
		           newpoly->vertex_list[fptr->vertex_indices[0]]);
      MAT3_SUB_VEC (vec2, newpoly->vertex_list[fptr->vertex_indices[2]],
		           newpoly->vertex_list[fptr->vertex_indices[1]]);
      MAT3cross_product (fptr->normal, vec1, vec2);
      fptr->normal[3] = 1.0;
			
      viptr++;   /* to move past sentinel to start of next list */
      fptr++;    /* to move to next facet struct in the array of facets */
   }
   
   SPH__add_polyhedron_element (newpoly);
}

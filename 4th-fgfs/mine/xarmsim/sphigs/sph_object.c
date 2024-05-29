#include "HEADERS.h"
#include "sphigslocal.h"
#include <stdlib.h>


static MAT3hvec raw_origin = {0.0,0.0,0.0,1.0};
static MAT3hvec xformed_origin;




extern srgp__point srgp_pdc_points[], srgp_polygon_points[];





/** OBJECT _ init
Makes sure there is space allocated for "fltptVertices";
   if it needs to allocate some, sets "vertexArraySize".
Sets "vertexCount" to 0.

Initializes list of objects to NULL.  Initializes the chunk that will be
used to falloc objects.

TO BE CALLED just before starting to traverse the network posted to
  a single view.
**/

void OBJECT__init (view_spec *vs)
{
   if (vs->uvnVertices == NULL) {
      ALLOC (vs->uvnVertices, MAT3hvec, ASSUMED_NUM_OF_OBJECTS, 0);
      vs->vertexArraySize = ASSUMED_NUM_OF_OBJECTS;
   }
   vs->vertexCount = 0;

   vs->objects = NULL;
   vs->objectTail = NULL;

   vs->curTraversalIndex = 0;
   
   if (vs->objectChunk == NULL)
      vs->objectChunk = FALLOCnew_chunk();
   else
      FALLOCclear_chunk (vs->objectChunk);

}



static void
AddVertices (view_spec *vs, matrix xform, MAT3hvec *points, int count)
{
   register i;

   /* Check to see if enough vertices remain in global array */
   if ((vs->vertexCount + count) > vs->vertexArraySize) {
      vs->vertexArraySize += ASSUMED_NUM_OF_OBJECTS;
      vs->uvnVertices = 
	 (MAT3hvec*) 
	    realloc (vs->uvnVertices, 
		     (MALLOCARGTYPE)(vs->vertexArraySize*sizeof(MAT3hvec)));
      if (vs->uvnVertices == NULL)
	 SPH__error (ERR_MALLOC);
   }

   /* Copy all the vertices to end of the uvnVertices array, performing
      transformation while doing so. */
   for (i=0; i<count; i++)
      MAT3mult_hvec (vs->uvnVertices[vs->vertexCount++], points[i], xform, 1);
}



static void
AddObjectToEndOfList (view_spec *vs, obj *babyobj)
{
   if (vs->objectTail) {
      vs->objectTail->next = babyobj;
      vs->objectTail = babyobj;
   }
   else 
      vs->objects = vs->objectTail = babyobj;
   vs->objectTail->next = NULL;
}



/** OBJECT__addFillArea
**/

void
OBJECT__addFillArea
   (view_spec *vs, MAT3hvec *points, int count,
    matrix xform, attribute_group *attrs)
{
   obj *babyobj;
   int bias;
   register j;
   vector vec1, vec2;

   bias = vs->vertexCount;

   AddVertices (vs, xform, points, count);

   /* Create a single facet objects. */
   babyobj = (obj*)
      FALLOCalloc (vs->objectChunk, sizeof(obj), FALLOC__DONT_ZERO);
   babyobj->type = objFace;
   babyobj->attributes = *attrs;
   babyobj->data.face.numPoints = count;	
   babyobj->data.face.points = 
      (vertex_index*)
	 FALLOCalloc (vs->objectChunk,
		      sizeof(vertex_index) * babyobj->data.face.numPoints,
		      FALLOC__DONT_ZERO);
   for (j=0; j < babyobj->data.face.numPoints; j++)
      babyobj->data.face.points[j] = j + bias;

   /* Compute normal */
   MAT3_SUB_VEC (vec1, 
		 vs->uvnVertices[bias+1], vs->uvnVertices[bias]);
   MAT3_SUB_VEC (vec2, 
		 vs->uvnVertices[bias+2], vs->uvnVertices[bias+1]);
   MAT3cross_product (babyobj->normal, vec1, vec2);
   babyobj->normal[3] = 1.0;

   babyobj->traversal_index = vs->curTraversalIndex++;

   AddObjectToEndOfList (vs, babyobj);
}




/** OBJECT_addPoly
The received xform is a composite of:
   the composite modeling transform, and
   the view orientation transform.
Divides a polygon into its component faces. 
**/

void OBJECT__addPoly 
   (view_spec *vs, POLYHEDRON *poly, matrix xform, attribute_group *attrs)
{
   obj *babyobj;
   MAT3hvec xformed_normal;
   register i;
   int bias;

   bias = vs->vertexCount;

   AddVertices (vs, xform, poly->vertex_list, poly->vertex_count);

   /* For each face, create a new object. */
   for (i=0; i<poly->facet_count; i++) {
      register j;

      babyobj = (obj*)
	 FALLOCalloc (vs->objectChunk, sizeof(obj), FALLOC__DONT_ZERO);
      babyobj->type = objFace;
      babyobj->attributes = *attrs;
      babyobj->data.face.numPoints = poly->facet_list[i].vertex_count;
      babyobj->data.face.points = 
	 (vertex_index*)
	    FALLOCalloc (vs->objectChunk,
			 sizeof(vertex_index) * babyobj->data.face.numPoints,
			 FALLOC__DONT_ZERO);
      /* Compute normal */
      MAT3mult_vec (babyobj->normal, poly->facet_list[i].normal, 
		    currentNormalMCtoUVNxform);

      for (j=0; j < babyobj->data.face.numPoints; j++)
	 babyobj->data.face.points[j] =
	    poly->facet_list[i].vertex_indices[j] + bias;

      babyobj->traversal_index = vs->curTraversalIndex;

      AddObjectToEndOfList (vs, babyobj);
   }

   vs->curTraversalIndex++;
}



/** OBJECT process
Called after the objects have been collected.
The coordinates are still in uvn.
This function launches the processes of culling, clipping, etc.
Which processes are actually done depends upon the current
   rendering mode.

This will NEVER be called when rendermode is WIREFRAME_RAW (fastest).
**/
void OBJECT__process (view_spec *vs)
{
/*   printf ("On entry: objects are:\n");
   print_objects_in_view (vs); */

   switch (currentRendermode) {
    case WIREFRAME:
      SPH__map_to_canon (vs);
      SPH__cull (vs);
      SPH__clip (vs);
      SPH__generate_pdc_vertices (vs);
      break;
    case FLAT:
    case LIT_FLAT:
      SPH__map_to_canon (vs);
      SPH__cull (vs);
      SPH__clip (vs);
      SPH__calc_intensity (vs);
      SPH__zsort (vs);
      SPH__generate_pdc_vertices (vs);
      break;
   }
   
/*   printf ("On exit: objects are:\n");
   print_objects_in_view (vs); */
}





/** OBJECT draw All
Called after clipping, culling, sorting etc has all been done on an
   entire set of objects posted to a single view.
The objects by now are all in integer pdc coords!
*/

void OBJECT__drawAll (view_spec *vs)
{
   register obj *curobj;
   register int v;
   vertex_index *viptr;
   facet *facetptr;
   int shade_number, face_color;


   for (curobj=vs->objects; curobj; curobj = curobj->next) {
      switch (curobj->type) {

       case objFace:
	 /* CONVERT ALL VERTICES INTO SRGP VERTEX FORMAT. */
	 for (v = 0; v < curobj->data.face.numPoints; v++)
	    srgp_pdc_points[v] = vs->pdcVertices[curobj->data.face.points[v]];

	 /* IF NOT WIREFRAME: */
	 if (currentRendermode > WIREFRAME) {
	    face_color = curobj->attributes.interior_color;
	    if (currentRendermode == LIT_FLAT) {
	       if IS_A_FLEXICOLORINDEX(face_color) {
	         shade_number = 
		    fabs(curobj->intensity)*NUM_OF_SHADES_PER_FLEXICOLOR;
	         if (shade_number == NUM_OF_SHADES_PER_FLEXICOLOR)
		    shade_number--;
	         SRGP_setColor ( 
		   BASE_OF_SHADE_LUT_ENTRIES
		     + ((face_color-2)*NUM_OF_SHADES_PER_FLEXICOLOR)
			+ shade_number);
		   SRGP_setFillStyle (SOLID);
		}
		else {
		   SRGP_setColor (face_color);
		   SRGP_setBackgroundColor (1);
		   shade_number = fabs(curobj->intensity)*64;
		   if (shade_number == 64)  shade_number--;
		   SRGP_setFillStyle (BITMAP_PATTERN_OPAQUE);
		   SRGP_setFillBitmapPattern (40 + shade_number);
	 	}
	    }
	    /* Draw the interior. */
	    SRGP_fillPolygon (curobj->data.face.numPoints, srgp_pdc_points);
	 }

	 /* Draw the edge. */
	 if (curobj->attributes.edge_flag == EDGE_VISIBLE) {
	    SRGP_setColor (curobj->attributes.edge_color);
	    SRGP_setLineWidth (curobj->attributes.edge_width);
	    SRGP_polygon (curobj->data.face.numPoints, srgp_pdc_points);
	 }

	 break;
      }
   }
}


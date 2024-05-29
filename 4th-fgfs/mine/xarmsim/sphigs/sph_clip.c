#include "HEADERS.h"
   /**********************
    *
    File                 * clip.c
    * object clipping unit
    *
    * takes all the objects in the list and clips them against
    * the front and back planes
    *
    * calculates bounds in obj
    * may add points to global list
    *
    *********************************************************/
   
   /**********************
    *
    Includes             *
    *
    *********************************************************/
   
#include <stdio.h>
#include <assert.h>
#include <math.h>
   
#include "sphigslocal.h"
   
   
   /**********************
    *
    Prototypes           *
    *
    *********************************************************/
   
void bound(view_spec *vs, obj *current );

vertex_index add_global_point(view_spec *vs, MAT3hvec p );

#define FRONT 1
#define BACK  2

static void clip_generic (view_spec *vs,  obj *current, double wall, int WHICH);
static void interpolate_point( MAT3hvec p1, MAT3hvec p2, double z, MAT3hvec p3 );


#define clip_front(V,C,W)   clip_generic((V),(C),(W), FRONT)
#define clip_back(V,C,W)   clip_generic((V),(C),(W), BACK)

/**********************
 *
 Function             * clip
 *
 * filters the list of objects by clipping the objects
 * against the front and back planes
 *
 *********************************************************/

void SPH__clip (view_spec *vs)
{
   obj *              current;
   obj *              prev;
   MAT3hvec           min;
   MAT3hvec           max;
   MAT3hvec           previous;
   double             front;
   double             back;
   MAT3hvec *         uvnVertices;
   
   assert( vs != NULL );
   front = vs->frontPlaneDistance;
   back = vs->backPlaneDistance;
   
   if( vs->objects == NULL )
      return;
   
   assert( vs->vm_matrix != NULL );
   assert( vs->uvnVertices != NULL );
   uvnVertices = vs->uvnVertices;
   assert( back < front );
   
   prev = NULL;
   current = vs->objects;
   while( current != NULL ) {
      
      assert( current->next != current );
      
      bound(vs, current );
      assert( current->min[X] <= current->max[X] );
      assert( current->min[Y] <= current->max[Y] );
      assert( current->min[Z] <= current->max[Z] );
      
      MAT3_COPY_VEC( min, current->min );
      MAT3_COPY_VEC( max, current->max );   /* saves the dereference each time we need it */
      
      /* remove obj if z bounds are behind "back" or in front of "front" */
      
      /* this will take care of points and text */
      
      if( max[Z] < back || min[Z] > front ) {
	 if( prev == NULL )
	    vs->objects = current->next;
	 else
	    prev->next = current->next;
	 current = current->next;
	 continue;
      }
      
      /* only clip object if it's intersecting the front or back plane */
      
      if( max[Z] > front || min[Z] < back ) {
	 
	 /* handle line intersection with front and back planes */
	 
	 if( current->type == objLine ) {
	    
	    /* special case lines parallel to X-Y */
	    
	    if( min[X] == max[X] || min[Y] == max[Y] ) {
	       min[Z] = MAX( min[Z], back );
	       max[Z] = MIN( max[Z], front );
	       MAT3_COPY_VEC( current->min, min );
	       MAT3_COPY_VEC( current->max, max );
	       
	    } else {
	       if( max[Z] > back && back > min[Z] ) {
		  fprintf (stderr, "CALL TO interpolate_point IS WEIRD (clip.c,1)\n");
		  sleep(15);
		  exit(1);
		  /* interpolate_point( max[Z], min[Z], back, min[Z] ); */
	       }
	       if( max[Z] > front && front > min[Z] ) {
		  fprintf (stderr, "CALL TO interpolate_point IS WEIRD (clip.c,2)\n");
		  sleep(15);
		  exit(1);
		  /* interpolate_point( min[Z], max[Z], back, max[Z] ); */
	       }
	    }
	    
	 } else if( current->type == objFace ) {
	    
	    /* handle plane intersection with front and back planes */
	    
	    assert( current->data.face.numPoints >= 3 );
	    
	    if( max[Z] > front )
	       clip_front( vs, current, front );
	    
	    if( min[Z] < back )
	       clip_back( vs, current, back );
	    
	 }
      }
      
      prev = current;
      current = current->next;
   }
}

/**********************
 *
 Function             * bound
 *
 * calculates the uvn bounding cube for the object
 *
 *********************************************************/

void bound(view_spec *vs, obj *current )

{
   register int       i;
   MAT3hvec           p;
   MAT3hvec           max;
   MAT3hvec           min;
   
   assert( vs != NULL );
   assert( vs->uvnVertices != NULL );
   assert( current != NULL );
   
   switch( current->type ) {
    case objFace:
      
      max[X] = max[Y] = max[Z] = -HUGE_VAL;
      min[X] = min[Y] = min[Z] = HUGE_VAL;
      
      for( i = 0; i < current->data.face.numPoints; i++ ) {
	 MAT3_COPY_VEC (p, 
			(vs->uvnVertices)
			   [current->data.face.points[i]] );
	 if( p[X] > max[X] )
	    max[X] = p[X];
	 if( p[Y] > max[Y] )
	    max[Y] = p[Y];
	 if( p[Z] > max[Z] )
	    max[Z] = p[Z];
	 
	 if( p[X] < min[X] )
	    min[X] = p[X];
	 if( p[Y] < min[Y] )
	    min[Y] = p[Y];
	 if( p[Z] < min[Z] ) 
	    min[Z] = p[Z];
      }
      MAT3_COPY_VEC( current->min, min );
      MAT3_COPY_VEC( current->max, max );
      break;
      
    case objLine:
      for (i=X; i<=Z; i++) {
         min[i] = 
	    MIN (current->data.line.end1[i], current->data.line.end2[i]);
	 max[i] = 
	    MAX ( current->data.line.end1[i], current->data.line.end2[i]);
      }
      break;
      
    case objPoint:
      MAT3_COPY_VEC( current->max, current->min );
      break;
      
    case objText:
      current->max[Z] = current->min[Z];
      break;
      
   }
}



static void clip_generic (view_spec *vs,  obj *current, double wall, int WHICH)
{
   int                last_zone;
   int                current_zone;
   int                index1, index2;
   MAT3hvec           new_point;
   MAT3hvec           last_point;
   MAT3hvec           current_point;
   int                new_global_index;
   vertex_index *     new_face_points;
   vertex_index *     points;
   MAT3hvec *         uvnVertices;
   int                i;
   boolean            final_point, flag;
   
   assert( vs != NULL );
   assert( vs->uvnVertices != NULL );
   assert( current != NULL );
   
   uvnVertices = vs->uvnVertices;
   
   /* go through all the points */
   
   index2 = 0;
   index1 = 0;
   
   new_face_points = (vertex_index *) 
      FALLOCalloc(vs->objectChunk, 
		  sizeof(vertex_index) * 
		  (current->data.face.numPoints+2), FALLOC__DONT_ZERO);
   assert( new_face_points != NULL );
   
   points = & current->data.face.points[0];
   assert( points != NULL );
   
   MAT3_COPY_VEC( last_point, uvnVertices[ points[ index1 ] ] );

   flag = FALSE;
   if (WHICH == BACK) {   
      if (last_point[Z] < wall) {last_zone = -1; flag=TRUE;}
   }
   else {
      if (last_point[Z] > wall) {last_zone = 1; flag=TRUE;}
   }
   if ( ! flag) {
      last_zone = 0;
      new_face_points[ index2 ] = points[ index1 ];
      index2 ++;
   }
   
   final_point = FALSE;
   for( index1 = 1; index1 <= current->data.face.numPoints && ! final_point; index1 ++ ) {
      
      if( index1 == current->data.face.numPoints ) {
	 final_point = TRUE;
	 MAT3_COPY_VEC( current_point, uvnVertices[ points[ 0 ] ] );
      } else
	 MAT3_COPY_VEC( current_point, uvnVertices[ points[ index1 ] ] );
      
      flag = FALSE;
      if (WHICH == BACK) {   
         if (current_point[Z] < wall) {current_zone = -1; flag=TRUE;}
      }
      else {
         if (current_point[Z] > wall) {current_zone = 1; flag=TRUE;}
      }
      if ( ! flag)
	 current_zone = 0;
      

      if( current_zone != last_zone ) {
	 interpolate_point( last_point, current_point, wall, new_point );
	 new_global_index = add_global_point( vs, new_point );
	 new_face_points[ index2 ] = new_global_index;
	 index2 ++;
      }
      
      if( current_zone == 0 && ! final_point ) {
	 new_face_points[ index2 ] = points[ index1 ];
	 index2 ++;
      }
      
      MAT3_COPY_VEC( last_point, current_point );
      last_zone = current_zone;
   }
   
   current->data.face.points = new_face_points;
   current->data.face.numPoints = index2;
   
}




/**********************
 *
 Function             * interpolate_point
 *
 * given two endpoints, returns the point at z
 *
 *********************************************************/

static void interpolate_point( MAT3hvec p1, MAT3hvec p2, double z, MAT3hvec p3 )
{
   double             yz_slope;
   double             xz_slope;
   
   yz_slope = ( p2[Y] - p1[Y] ) / ( p2[Z] - p1[Z] );
   xz_slope = ( p2[X] - p1[X] ) / ( p2[Z] - p1[Z] );
   
   p3[Z] = z;
   p3[X] = p1[X] + xz_slope * ( z - p1[Z] );
   p3[Y] = p1[Y] + yz_slope * ( z - p1[Z] );
}


/**********************
 *
 Function             * add_global_point
 *
 * adds a point to the global list of points
 * returns the index of the point in the array
 * also adds the perspectivized point into its array
 *
 *********************************************************/

vertex_index add_global_point(view_spec *vs, MAT3hvec p )

{
   assert( vs != NULL );
   assert( vs->uvnVertices != NULL );
   assert( vs->npcVertices != NULL );
   assert( vs->vertexArraySize != 0 );
   assert( vs->vertexCount <= vs->vertexArraySize );
   
   if( vs->vertexCount == vs->vertexArraySize ) {
      vs->vertexArraySize /= 2;
      vs->vertexArraySize *= 3;
      vs->uvnVertices = 
	 (MAT3hvec*) 
	    realloc (vs->uvnVertices, 
		     (MALLOCARGTYPE)(vs->vertexArraySize*sizeof(MAT3hvec)));
      if (vs->uvnVertices == NULL) 
	 SPH__error (ERR_MALLOC);
      vs->npcVertices = 
	 (MAT3hvec *) 
	    realloc (vs->npcVertices, 
		     (MALLOCARGTYPE)(vs->vertexArraySize*sizeof(MAT3hvec)));
      if (vs->npcVertices == NULL) 
	 SPH__error (ERR_MALLOC);
   }
   
   MAT3_COPY_VEC( (vs->uvnVertices)[ vs->vertexCount ], p );
   ((vs->uvnVertices)[ vs->vertexCount ])[3] = 1;
   
   MAT3mult_hvec
      (vs->npcVertices[ vs->vertexCount ], 
       vs->uvnVertices[ vs->vertexCount ],
       vs->vm_matrix, 1 );
   (vs->npcVertices[ vs->vertexCount ])[Z] *= -1;
   
   vs->vertexCount ++;
   
   return( vs->vertexCount - 1 );
}

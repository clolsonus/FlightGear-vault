#include "HEADERS.h"
/**********************
                      *
 File                 * zsort.c
                      * painter's algorithm
                      *
                      * takes the list of objects and zsorts them
                      * corrects any ambiguities
                      *
                      * assumes normalized object p_normals
                      * computes plane equations
                      *
                      *********************************************************/

/**** CURRENTLY: splitting of polygons is commented out!  See "HORRORS". */

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
 Prototypes	      *
                      *
                      *********************************************************/

static void sort( );
static void correct( );
static int test_point( );
static int test_relation( );
static void compute_plane_equation( );
static int test_point( );
static boolean xy_extent_intersect( );
static boolean test_lines( );
static boolean get_index_line( );
static boolean test_proj_intersect( );
static void perspect_bound( );
static void interpolate_3d_point( );
static void split( );


void bound(view_spec *vs, obj *current );
vertex_index add_global_point(view_spec *vs, MAT3hvec p );



#ifdef NEVER_WILL_BE_DEFINED

/**********************
                      *
 Function	      * make_split_point_list
                      *
                      *********************************************************/

static int
make_split_point_list( antizone, obj1, obj2, new_points, vs )
     int	       	antizone;
     obj *		obj1;
     obj *		obj2;
     vertex_index *	new_points;
     view_spec *	vs;
{
     int		last_zone;
     int		current_zone;
     int		index1, index2;
     MAT3hvec		new_point;
     MAT3hvec		last_point;
     MAT3hvec		current_point;
     int		new_global_index;
     vertex_index *    	points;
     MAT3hvec *		uvnVertices;
     boolean		final_point;
     
     assert( obj1 != NULL );
     assert( obj2 != NULL );
     assert( new_points != NULL );
     assert( vs != NULL );
     assert( vs->uvnVertices != NULL );
     
     uvnVertices = vs->uvnVertices;
     
     /* go through all the points */
     
     index2 = 0;
     index1 = 0;
     
     points = & obj2->data.face.points[0];
     assert( points != NULL );
     
     MAT3_COPY_VEC( last_point, uvnVertices[ points[ index1 ] ] );
     last_zone = test_point( last_point, obj1->normal, obj1->directed_distance );
     if( last_zone != antizone ) {				/* getting front face first */
	  new_points[ index2 ] = points[ index1 ];
	  index2 ++;
     }
     
     final_point = FALSE;
     for( index1 = 1; index1 <= obj2->data.face.numPoints && ! final_point; index1 ++ ) {
	  
	  if( index1 == obj2->data.face.numPoints ) {
	       final_point = TRUE;
	       MAT3_COPY_VEC( current_point, uvnVertices[ points[ 0 ] ] );
	  } else
	       MAT3_COPY_VEC( current_point, uvnVertices[ points[ index1 ] ] );
	  
	  current_zone = test_point( current_point, obj1->normal, obj1->directed_distance );
	  
	  /* interpolate only when going from zone 1 -> -1 or 1 -> 1 */
	  
	  if( current_zone != last_zone && current_zone + last_zone == 0 ) {
	       
	       interpolate_3d_point( last_point, current_point, obj1, new_point );
	       
	       new_global_index = add_global_point( vs, new_point );
	       new_points[ index2 ] = new_global_index;
	       index2 ++;
	       
	  }
	  
	  if( current_zone != antizone && ! final_point ) {
	       new_points[ index2 ] = points[ index1 ];
	       index2 ++;
	  }
	  
	  MAT3_COPY_VEC( last_point, current_point );
	  last_zone = current_zone;
     }
     
     return( index2 );
}


#endif



/**********************
                      *
 Function	      * zsort
                      *
                      * sorts and corrects the list of objects
		      *
                      *********************************************************/

void
SPH__zsort( vs )
     view_spec *	vs;
{
     obj *		list;
     obj *		sorted_list;		/* list sorted from back to front */
     obj *		current;
     obj *		prev;
     obj *		next_object;
     
     assert( vs != NULL );
     
     if( vs->objects != NULL )
	  sort( vs );
     
     if( vs->objects != NULL )
	  correct( vs );
}

/**********************
                      *
 Function	      * sort
                      *
                      * uses a simple insertion sort to sort the bounding cubes
                      *   of the objects
		      *
                      *********************************************************/

static void
sort( vs )
     view_spec *	vs;
{
     obj *		list;
     obj *		sorted_list;		/* list sorted from back to front */
     obj *		current;
     obj *		prev;
     obj *		next_object;
     
     assert( vs != NULL );
     list = vs->objects;
     
     sorted_list = NULL;
     while( list != NULL ) {			/* take the top object of the list */
	  
	  compute_plane_equation( vs, list );
	  list->already_moved = FALSE;
	  
	  prev = NULL;
	  current = sorted_list;
	  
	  /* search in the new list for the place to put it */
	  
	  while( current != NULL && list->min[Z] > current->min[Z] ) {
	       prev = current;
	       assert( current != current->next );
	       current = current->next;
	  }
	  
	  if( prev == NULL )
	       sorted_list = list;
	  else
	       prev->next = list;
	  
	  next_object = list->next;
	  assert( next_object != list );
	  list->next = current;
	  
	  list = next_object;
	  
     }
     
     vs->objects = sorted_list;
}

/**********************
                      *
 Function	      * correct
                      *
                      * the heart of the algorithm
                      * checks objects in intersecting z bounds against
                      *    each other with four tests
                      *
                      *********************************************************/

static void
correct( vs )
     view_spec *	vs;
{
     obj *		list;
     obj *		current_prev;
     obj *		current;
     obj *		current_next;
     obj *		scan_prev;
     obj *		scan;
     obj *		scan_next;
     int		relation1, relation2;
     obj		swap;
     
     assert( vs != NULL );
     
     list = vs->objects;
     
     current = list;
     current_prev = NULL;
     while( current != NULL ) {
here:	  
	  scan = current->next;
	  scan_prev = current;
	  while( scan != NULL && scan->min[Z] <= current->max[Z] ) {
	       
/*	       fprintf( stderr, "checking %d %d %d   %d %d %d\n", scan->data.face.points[0], scan->data.face.points[1], scan->data.face.points[2], current->data.face.points[0], current->data.face.points[1], current->data.face.points[2] ); */
	       
	       if( ! xy_extent_intersect( current, scan ) ) {
		    scan_prev = scan;
		    scan = scan->next;
		    continue;
	       }
	       relation1 = test_relation( current, scan, vs );
	       if( relation1 == -1 ) {
		    scan_prev = scan;
		    scan = scan->next;
		    continue;
	       }
	       relation2 = test_relation( scan, current, vs );
	       if( relation2 == 1 ) {
		    scan_prev = scan;
		    scan = scan->next;
		    continue;
	       }
	       
	       if( relation1 == relation2 == 0 ) {
		    /* HORRORS! a pair of intersecting polygons! */
		    /* split( current, scan, scan_prev, vs ); */
		    /*********scan_prev = scan;	        SKLAR added as a guess*/
		    /*********scan = scan->next;	SKLAR added as a guess*/
		    /* continue; */  /*SKLAR commented out as a guess*/
	       }
	       
	       if( ! test_proj_intersect( scan, current, vs ) ) {
		    scan_prev = scan;
		    scan = scan->next;
		    continue;
	       }
	       
	       if( current->already_moved ) {
		    scan_prev = scan;
		    scan = scan->next;
		    continue;
	       }
	       
	       current_next = current->next;
	       scan_next = scan->next;
	       
/*	       fprintf (stderr, "swapping..\n");
		    current->attributes.interior_color = 7;
		    scan->attributes.interior_color = 7; */
	       
/*	       fprintf (stderr, 
	                "before swap c %x s %x cp %x sp %x cn %x sn %x\n", 
			(long) current, (long) scan,
			(long) current_prev, (long) scan_prev, 
			(long) current_next, (long) scan_next ); */
	       
	       swap = *scan;				/* the easy swap */
	       *scan = *current;
	       *current = swap;
	       
	       current->next = current_next;
	       scan->next = scan_next;
	       
	       scan->already_moved = TRUE;


/*	       if( scan == current->next ) {		* we are reordering two consecutive nodes *
		    
		    fprintf( stderr, "ah its seq!\n" );
		    if( current_prev != NULL )
			 current_prev->next = scan;
		    else
			 list = scan;
		    scan->next = current;
		    current->next = scan_next;
		    
		    current = scan;
		    
	       } else {					* two arbitrary nodes *
		    
	       * fix the previous node *
		    
		    if( current_prev != NULL )
			 current_prev->next = scan;
		    else
			 list = scan;
		    scan->next = current_next;
		    
		    scan_prev->next = current;
		    current->next = scan_next;
		    
		    current = scan;
	       }
*/	       

/*	       fprintf (stderr, 
	                "after swap c %x s %x cp %x sp %x cn %x sn %x\n", 
			(long) current, (long) scan,
			(long) current_prev, (long) scan_prev, 
			(long) current_next, (long) scan_next ); */
	       
	       /* restart this level of scanning */
	       
	       goto here;
	  }
	  
	  current_prev = current;
	  current = current->next;
     }
     
     vs->objects = list;
}

/**********************
                      *
 Function	      * compute_plane_equation
                      *
		      * computes the plane equation for the object
		      *
		      * calculates the "normal" for lines
		      * normalizes all normals
		      *
                      *********************************************************/

static void
compute_plane_equation( vs, current )
     view_spec *	vs;
     obj *	      	current;
{
     MAT3hvec		p;
     MAT3hvec		normal;
     double		length;
     
     assert( current != NULL );
     assert( vs != NULL );
     assert( vs->npcVertices != NULL );
     
     perspect_bound( vs, current );
     
     switch( current->type ) {
	case objFace:
	  MAT3_COPY_VEC( p, vs->npcVertices[ current->data.face.points[0] ] );
	  MAT3_COPY_VEC( normal, current->p_normal );
	  current->directed_distance = - ( normal[X] * p[X] + normal[Y] * p[Y] + normal[Z] * p[Z] );
	  break;
	  
	case objText:
	  current->p_normal[3] = 1;
	  MAT3_NORMALIZE_VEC( current->p_normal, length );
	  current->directed_distance = - ( current->min[Z] );
	  break;
	  
	case objLine:
	  MAT3_COPY_VEC( p, current->data.line.end1 );
	  normal[X] = - ( current->data.line.end1[Z] - current->data.line.end2[Z] );
	  normal[Y] = 0;
	  normal[Z] = - ( current->data.line.end1[X] - current->data.line.end2[X] );
	  normal[3] = 1;
	  MAT3_NORMALIZE_VEC( normal, length );
	  current->directed_distance = - ( normal[X] * current->data.line.end1[X] +
					  normal[Z] * current->data.line.end1[Z] );
	  MAT3_COPY_VEC( current->normal, normal );
	  break;
	  
	case objPoint:
	  break;
     }
}


/**********************
                      *
 Function	      * perspect_bound
                      *
                      * calculates the bounding cube for the object
                      *
                      *********************************************************/

static void
perspect_bound( vs, current )
     view_spec *        vs;
     obj *		current;
{
     int		index;
     MAT3hvec		p;
     MAT3hvec		max;
     MAT3hvec		min;
     
     assert( vs != NULL );
     assert( vs->npcVertices != NULL );
     assert( current != NULL );
     
     assert( current->type == objFace );
     
     switch( current->type ) {
	case objFace:
	  
	  max[X] = max[Y] = -HUGE_VAL;
	  min[X] = min[Y] = HUGE_VAL;
	  
	  for( index = 0; index < current->data.face.numPoints; index++ ) {
	       MAT3_COPY_VEC( p, (vs->npcVertices)[ current->data.face.points[ index ] ] );
	       if( p[X] > max[X] )
		    max[X] = p[X];
	       if( p[Y] > max[Y] )
		    max[Y] = p[Y];
	       
	       if( p[X] < min[X] )
		    min[X] = p[X];
	       if( p[Y] < min[Y] )
		    min[Y] = p[Y];
	  }
	  
	  MAT3_COPY_VEC( current->p_min, min );
	  MAT3_COPY_VEC( current->p_max, max );
	  
	  break;
     }
  }

/**********************
                      *
 Function	      * xy_extent_intersect
                      *
		      * returns TRUE if the x-y bounds intersect
		      *
                      *********************************************************/

static boolean
xy_extent_intersect( obj1, obj2 )
     obj *		obj1;
     obj *		obj2;
{

     MAT3hvec		obj2_p_min, obj2_p_max;
     boolean		x_intersect;
     boolean		y_intersect;
     
     MAT3_COPY_VEC( (double *) obj2_p_min, (double *) obj2->p_min );
     MAT3_COPY_VEC( (double *) obj2_p_max, (double *) obj2->p_max );
     
     x_intersect = ( obj1->p_min[X] >= obj2_p_min[X] && obj1->p_min[X] < obj2_p_max[X] ) |
	  ( obj1->p_max[X] > obj2_p_min[X] && obj1->p_max[X] <= obj2_p_max[X] );
     y_intersect = ( obj1->p_min[Y] >= obj2_p_min[Y] && obj1->p_min[Y] < obj2_p_max[Y] ) |
	  ( obj1->p_max[Y] > obj2_p_min[Y] && obj1->p_max[Y] <= obj2_p_max[Y] );
     return( x_intersect & y_intersect );
/*
     MAT3hvec		obj2_min, obj2_max;
     boolean		x_intersect;
     boolean		y_intersect;
     
     MAT3_COPY_VEC( (double *) obj2_min, (double *) obj2->min );
     MAT3_COPY_VEC( (double *) obj2_max, (double *) obj2->max );
     
     x_intersect = ( obj1->min[X] >= obj2_min[X] && obj1->min[X] < obj2_max[X] ) |
	  ( obj1->max[X] > obj2_min[X] && obj1->max[X] <= obj2_max[X] );
     y_intersect = ( obj1->min[Y] >= obj2_min[Y] && obj1->min[Y] < obj2_max[Y] ) |
	  ( obj1->max[Y] > obj2_min[Y] && obj1->max[Y] <= obj2_max[Y] );
     return( x_intersect & y_intersect );
*/
}

/**********************
                      *
 Function	      * test_relation
                      *
		      * returns -1 if obj1 is outside obj2
		      * returns  0 if obj1 is intersecting obj2
		      * returns  1 if obj1 is inside obj2
		      *	
		      *	tests each point in obj1 against the plane equation of
		      *	   obj2
		      *	
                      *********************************************************/

static int
test_relation( obj1, obj2, vs )
     obj *		obj1;
     obj *		obj2;
     view_spec *	vs;
{
     MAT3hvec	     	obj1_point;
     MAT3hvec		obj2_normal;
     int		index;
     int		relation;			/* relation of first point of obj1 to obj2; the relation to match */
     int		new_relation;			/* the relation of each other point of obj1 */
     double		plane_point;			/* each point of the plane obj1 */
     
     assert( vs != NULL );
     assert( vs->npcVertices != NULL );
     assert( obj1 != NULL );
     assert( obj2 != NULL );
     
     switch( obj1->type ) {
	  
	  /******* face *******/
	  
	case objFace:
	  
	  index = 0;
	  
	  MAT3_COPY_VEC( obj2_normal, obj2->p_normal );
	  
	  MAT3_COPY_VEC( obj1_point, (vs->npcVertices)[ obj1->data.face.points[ index ] ] );
	  relation = test_point( obj1_point, obj2_normal, obj2->directed_distance );
	  
	  for( index = 1; index < obj1->data.face.numPoints; index++ ) {
	       
	       MAT3_COPY_VEC( obj1_point, (vs->npcVertices)[ obj1->data.face.points[ index ] ] );
	       new_relation = test_point( obj1_point, obj2_normal, obj2->directed_distance );
	       
	       /* check for intersecting between faces */
	       
	       if( relation == 0 && new_relation != 0 )
		    relation = new_relation;
	       
	       if( new_relation != relation && new_relation + relation == 0 )
		    return( 0 );
	  }
	  return( relation );
	  break;
	  
	  /******* text *******/
	  
	case objText:
	  MAT3_COPY_VEC( obj2_normal, obj2->p_normal );
	  
	  obj1_point[Z] = obj1->min[Z];
	  
	  obj1_point[X] = obj1->min[X];
	  obj1_point[Y] = obj1->min[Y];
	  relation = test_point( obj1_point, obj2_normal, obj2->directed_distance );
	  
	  obj1_point[X] = obj1->min[X];
	  obj1_point[Y] = obj1->max[Y];
	  new_relation = test_point( obj1_point, obj2_normal, obj2->directed_distance );
	  if( relation != new_relation )
	       return( 0 );
	  
	  obj1_point[X] = obj1->max[X];
	  obj1_point[Y] = obj1->min[Y];
	  new_relation = test_point( obj1_point, obj2_normal, obj2->directed_distance );
	  if( relation != new_relation )
	       return( 0 );
	  
	  obj1_point[X] = obj1->max[X];
	  obj1_point[Y] = obj1->max[Y];
	  new_relation = test_point( obj1_point, obj2_normal, obj2->directed_distance );
	  if( relation != new_relation )
	       return( 0 );
	  
	  return( relation );
	  break;
	  
	  /******* line *******/
	  
	case objLine:
	  MAT3_COPY_VEC( obj2_normal, obj2->normal );
	  
	  MAT3_COPY_VEC( obj1_point, obj1->data.line.end1 );
	  relation = test_point( obj1_point, obj2->normal, obj2->directed_distance );
	  
	  MAT3_COPY_VEC( obj1_point, obj1->data.line.end2 );
	  new_relation = test_point( obj1_point, obj2->normal, obj2->directed_distance );
	  if( relation != new_relation )
	       return( 0 );
	  
	  return( relation );
	  break;
	  
	case objPoint:
	  MAT3_COPY_VEC( obj2_normal, obj2->normal );
	  
	  relation = test_point( obj1->min, obj2->normal, obj2->directed_distance );
	  return( relation );
     }
}

/**********************
                      *
 Function	      * test_point
                      *
		      * returns -1 if p is behind plane with normal
		      * returns  0 if p is on plane with normal
		      * returns  1 if p is in front of plane with normal
		      *	
                      *********************************************************/

static int
test_point( p, normal, distance )
     MAT3hvec			p;
     MAT3hvec			normal;
     double			distance;
{
     double			plane_point;
     
     plane_point = normal[X] * p[X] + normal[Y] * p[Y] + normal[Z] * p[Z] + distance;
     if( normal[Z] > 0 ) {
	  if( plane_point < -1e-4 )
	       return( -1 );
	  else if( plane_point > 1e-4 )
	       return( 1 );
	  else
	       return( 0 );
     } else {
	  if( plane_point < -1e-4 )
	       return( 1 );
	  else if( plane_point > 1e-4 )
	       return( -1 );
	  else
	       return( 0 );
     }
}

/**********************
                      *
 Function	      * test_proj_intersect
                      *
		      * returns TRUE if the projections intersect
		      *	a brutal n^2 approach
		      *
                      *********************************************************/

static boolean
test_proj_intersect( obj1, obj2, vs )
     obj *		obj1;
     obj *		obj2;
     view_spec *	vs;
{
     int		end1;
     int		end2;
     int		endA;
     int		endB;
     int		i, j;
     int		old_cw;
     int		last;
     int		cw;
     boolean		same;
     
     assert( vs != NULL );
     assert( obj1 != NULL );
     assert( obj2 != NULL );
     
/*     last = -1;
     same = TRUE;
     for( i = 0; i < obj1->data.face.numPoints; i++ ) {
	  get_index_line( obj2, i, &end1, &end2 );
	  cw = ccw( obj1->data.face.points[ i ], end1, end2, vs );
	  if( last == -1 ) {
	       old_cw = cw;
	       last = 0;
	  } else {
	       same = same & ( cw == old_cw );
	  }
     }
     if( same )
	  return( TRUE );
     
     last = -1;
     same = TRUE;
     for( i = 0; i < obj2->data.face.numPoints; i++ ) {
	  get_index_line( obj1, i, &end1, &end2 );
	  cw = ccw( obj2->data.face.points[ i ], end1, end2, vs );
	  if( last == -1 ) {
	       old_cw = cw;
	       last = 0;
	  } else {
	       same = same & ( cw == old_cw );
	  }
     }
     if( same )
	  return( TRUE );
     */
     
     assert( vs != NULL );
     
     i = 0;
     while( get_index_line( obj1, i, &end1, &end2 ) ) {
	  j = 0;
	  while( get_index_line( obj2, j, &endA, &endB ) ) {
	       if( test_lines( end1, end2, endA, endB, vs ) ) {
		    return( TRUE );
	       }
	       j ++;
	  }
	  i ++;
     }
     return( FALSE );
}

/**********************
                      *
 Function	      * ccw
                      *
                      *********************************************************/

static int
ccw( end0, end1, end2, vs )
     int		end0;
     int		end1;
     int		end2;
     view_spec *	vs;
{
     MAT3hvec *		npcVertices;
     double		dx1, dx2, dy1, dy2;
     
     assert( vs != NULL );
     
     assert( vs->npcVertices != NULL );
     npcVertices = vs->npcVertices;
     
     dx1 = (npcVertices[end1])[X] - (npcVertices[end0])[X];
     dy1 = (npcVertices[end1])[Y] - (npcVertices[end0])[Y];
     dx2 = (npcVertices[end2])[X] - (npcVertices[end0])[X];
     dy2 = (npcVertices[end2])[Y] - (npcVertices[end0])[Y];
     
     if( dx1 * dy2 > dy1 * dx2 )
	  return( 1 );
     else if( dx1 * dy2 < dy1 * dx2 )
	  return( -1 );
     else
	  return( 0 );
}

/**********************
                      *
 Function	      * test_lines
                      *
		      * returns TRUE if the lines intersect
		      *
                      *********************************************************/

static boolean
test_lines( end1, end2, endA, endB, vs )
     int		end1;
     int		end2;
     int		endA;
     int		endB;
     view_spec  *vs;
{
     assert( vs != NULL );
     
     if( ( end1 == endA && end2 == endB ) || ( end1 == endB && end2 == endA ) ) {
	  return( FALSE );
     }
     
     return( ( ccw( end1, end2, endA, vs ) * ccw( end1, end2, endB, vs ) < 0 ) &&
	    ( ccw( endA, endB, end1, vs ) * ccw( endA, endB, end2, vs ) < 0 ) );
}

/**********************
                      *
 Function	      * get_index_line
                      *
		      * returns FALSE if the index is out of bounds
		      *
                      *********************************************************/

static boolean
get_index_line( obj1, index, end1, end2 )
     obj *		obj1;
     int		index;
     int * 		end1;
     int *		end2;
{
     vertex_index *	points;
     
     assert( end1 != NULL );
     assert( end2 != NULL );
     assert( obj1 != NULL );
     
     assert( obj1->type == objFace );		/* for now */
     
     switch( obj1->type ) {
	case objFace:
	  points = obj1->data.face.points;
	  if( index >= obj1->data.face.numPoints )
	       return( FALSE );
	  *end1 = points[ index ];
	  if( index == obj1->data.face.numPoints - 1 )
	       *end2 = points[ 0 ];
	  else
	       *end2 = points[ index + 1 ];
	  break;
	case objText:
	  break;
	case objLine:
	  break;
	case objPoint:
	  break;
     }
     return( TRUE );
}



#ifdef NEVER_WILL_BE_DEFINED

/**********************
                      *
 Function	      * split
                      *
		      * splits obj2 along its intersection with obj1
		      *
                      *********************************************************/

static void
split( obj1, obj2, obj2prev, vs )
     obj *		obj1;
     obj *		obj2;
     obj *	        obj2prev;
     view_spec *	vs;
{
     vertex_index *	new_list1;
     vertex_index *	new_list2;
     int		size_list1;
     int		size_list2;
     obj *		new_object;
     
     assert( vs != NULL );
     assert( vs->uvnVertices != NULL );
     assert( obj1 != NULL );
     assert( obj2 != NULL );
     assert( obj2prev != NULL );
     
     new_list1 = (vertex_index *) 
	FALLOCalloc(vs->objectChunk, sizeof( vertex_index ) * ( obj2->data.face.numPoints + 2 ), FALLOC__DONT_ZERO );
     assert( new_list1 != NULL );
     new_list2 = (vertex_index *) 
	FALLOCalloc(vs->objectChunk, sizeof( vertex_index ) * ( obj2->data.face.numPoints + 2 ), FALLOC__DONT_ZERO );
     assert( new_list2 != NULL );
     
     size_list1 = make_split_point_list( -1, obj1, obj2, new_list1, vs );
     size_list2 = make_split_point_list( 1, obj1, obj2, new_list2, vs );
     
     new_object = (obj *)
	  FALLOCalloc( vs->objectChunk,	sizeof( obj ), FALLOC__DONT_ZERO );
     assert( new_object != NULL );
     *new_object = *obj2;
     
     obj2->data.face.points = new_list1;
     new_object->data.face.points = new_list2;
     obj2->data.face.numPoints = size_list1;
     new_object->data.face.numPoints = size_list2;
     
     bound( vs, obj2 );
     bound( vs, new_object );
     
     if( obj2->min[Z] < new_object->min[Z] )
	  obj2->next = new_object;
     else {
	  obj2prev->next = new_object;
	  new_object->next = obj2;
     }
}


#endif




/**********************
                      *
 Function	      * interpolate_3d_points
                      *
                      *********************************************************/

static void
interpolate_3d_point( start, end, obj1, new )
     obj *		obj1;
     MAT3hvec		start, end, new;
{
     double		p;
     MAT3hvec		slope;
     
     slope[X] = end[X] - start[X];
     slope[Y] = end[Y] - start[Y];
     slope[Z] = end[Z] - start[Z];
     
     p = - ( obj1->normal[X] * start[X] + obj1->normal[Y] * start[Y] + obj1->normal[Z] * start[Z] + obj1->directed_distance ) /
	  ( obj1->normal[X] * slope[X] + obj1->normal[Y] * slope[Y] + obj1->normal[Z] * slope[Z] );
     
     new[X] = p * slope[X] + start[X];
     new[Y] = p * slope[Y] + start[Y];
     new[Z] = p * slope[Z] + start[Z];
}

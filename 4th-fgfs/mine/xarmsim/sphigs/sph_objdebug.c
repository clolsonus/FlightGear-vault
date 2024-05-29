#include "HEADERS.h"
/**********************
                      *
 File                 * objdebug.c
                      *
                      *********************************************************/

/**********************
                      *
 Includes	      *
                      *
                      *********************************************************/

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "sphigslocal.h"


/**********************
                      *
 Prototypes	      *
                      *
                      *********************************************************/

void print_objects_in_view( );
void print_objects( );
void print_object( );

void
print_objects_in_view (vs)
view_spec *vs;
{
     assert( vs != NULL );
     print_objects( vs, vs->objects );
}

int
count_objects( current )
obj *current;
{
     register i = 0;
     
     while( current != NULL ) {
	  i++;
	  current = current->next;
     }
     
     return( i );
}

void
print_objects( vs, current )
view_spec *vs;
obj *current;
{
     assert( vs != NULL );
     fprintf( stdout, "START OF LIST\n" );
     while (current) {
	  print_object( vs, current, 0 );
	  current = current -> next;
     }
     fprintf( stdout, "END OF LIST\n" );
}

void
print_object( vs, current, where )
view_spec *vs;
obj *current;
int where;
{
     register i;
     FILE * stream;
     
     assert( vs != NULL );
     assert( current != NULL );
     
     if( where == 1 )
	  stream = stderr;
     else
	  stream = stdout;
     
     fprintf( stream, "[ ]\tobject address = 0x%x\n", (long) current );
     fprintf( stream, "\tobject type = " );
     switch( current->type ) {
	case objFace:
	  printf( "face\n" );
	  break;
	case objLine:
	  printf( "face\n" );
	  break;
	case objText:
	  printf( "text\n" );
	  break;
	case objPoint:
	  printf( "point\n" );
	  break;
     }
     
     fprintf( stream, "\tnormal = [ %lf %lf %lf %lf ]\n", current->normal[0],
	     current->normal[1], current->normal[2], current->normal[3] );
     
     fprintf( stream, "\tp normal = [ %lf %lf %lf %lf ]\n", current->p_normal[0],
	     current->p_normal[1], current->p_normal[2], current->p_normal[3] );
     
     fprintf( stream, "\tmin = [ %lf %lf %lf %lf ]\n", current->min[0],
	     current->min[1], current->min[2], current->min[3] );
     
     fprintf( stream, "\tmax = [ %lf %lf %lf %lf ]\n", current->max[0],
	     current->max[1], current->max[2], current->max[3] );
     
     fprintf( stream, "\tintensity = %lf\n", current->intensity );
     fprintf( stream, "\tdirected distance = %lf\n", current->directed_distance );
     
     switch( current->type ) {
	case objFace:
	  fprintf( stream, "\tinterior color = %d\n", current->attributes.interior_color );
	  fprintf( stream, "\tnumPoints = %d\n", current->data.face.numPoints );
	  fprintf( stream, "\tpoints = 0x%x\n", (long) current->data.face.points );
	  assert( current->data.face.points != NULL );
	  fprintf( stream, "\tarray = " );
	  for( i = 0; i < current->data.face.numPoints; i++ )
	       fprintf( stream, "%d ", (int) current->data.face.points[ i ] );
	  fprintf( stream, "\n" );
	  for( i = 0; i < current->data.face.numPoints; i++ ) {
	       if( vs->npcVertices != NULL ) {
		    fprintf( stream, "\t\t%d (%lf %lf %lf) (%lf %lf %lf)\n", current->data.face.points[ i ],
			    vs->uvnVertices[current->data.face.points[i] ][0],
			    vs->uvnVertices[current->data.face.points[i] ][1],
			    vs->uvnVertices[current->data.face.points[i] ][2],
			    vs->npcVertices[current->data.face.points[i] ][0],
			    vs->npcVertices[current->data.face.points[i] ][1],
			    vs->npcVertices[current->data.face.points[i] ][2] );
	       } else {
		    fprintf( stream, "\t\t%d (%lf %lf %lf)\n", current->data.face.points[ i ],
			    vs->uvnVertices[current->data.face.points[i] ][0],
			    vs->uvnVertices[current->data.face.points[i] ][1],
			    vs->uvnVertices[current->data.face.points[i] ][2] );
	       }
	  }
	  
     
	  fprintf( stream, "\tp_min = [ %lf %lf %lf %lf ]\n", current->p_min[0],
		  current->p_min[1], current->p_min[2], current->p_min[3] );
     
	  fprintf( stream, "\tp_max = [ %lf %lf %lf %lf ]\n", current->p_max[0],
		  current->p_max[1], current->p_max[2], current->p_max[3] );
	  
	  break;
	  
	case objLine:
	  fprintf( stream, "\tend1 = [ %lf %lf %lf %lf ]\n", current->data.line.end1[0],
		  current->data.line.end1[1], current->data.line.end1[2], current->data.line.end1[3] );
	  
	  fprintf( stream, "\tend2 = [ %lf %lf %lf %lf ]\n", current->data.line.end2[0],
		  current->data.line.end2[1], current->data.line.end2[2], current->data.line.end2[3] );
	  break;
	  
	case objText:
	  fprintf( stream, "\ttext = %s\n", current->data.text.text );
	  break;
	  
	case objPoint:
	  break;
     }
     
}

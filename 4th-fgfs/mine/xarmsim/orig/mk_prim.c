/* mk_prim.c
 *
 * Functions to make primitive shapes
 */

#include <gl/gl.h>    /* TRUE/FALSE definitions */
#include <math.h>

#include "armsim.h"
#include "mk_prim.h"


/************************************************************************
 **
 ** FUNCTION NAME    : box                                              
 ** PURPOSE          : This function creates a box.  The top of the box 
 **                    may be a different size than the bottom of the box
 **                    although the top and bottom are parallel.  This allows
 **                    us to create boxes which are skewed with respect to
 **                    x and y axis.  Creates a solid box or a wire frame
 **                    box depending on the value of the constant solid.
 ** RETURNS          : int which is iris index to box object.           
 ** INPUTS           : x1 -- least bottom x coordinate.
 **                    x2 -- greatest bottom x coordinate.
 **                    x3 -- least top x coordinate.                    
 **                    x4 -- greatest top x coordinate.                 
 **                    y1 -- least bottom y coordinate.         
 **                    y2 -- greatest bottom y coordinate.      
 **                    y3 -- least top y coordinate.                    
 **                    y4 -- greatest top y coordinate.                 
 **                    z1 -- least z coordinate.                        
 **                    z2 -- greates z coordinate.                      
 **                    c -- color box should be. - disabled; 
 gray surfaces, black wire
 ** LOCAL VARIABLES  : o1 -- object index to be returned.               
 v: vertices of box
 wpath, spath: order of vertices used for wire and solid drawing
 a,j: loop counters
 ** GLOBAL VARIABLES : solid                                            
 **                                                                     
 ************************************************************************/


long int box (float x1, float x2, float x3, float x4, float y1, float y2, 
	      float y3, float y4, float z1, float z2, int c)
{
    float v[8][3];
    long int o1;

    int wpath[16] = {	
	0,1,2,3,
	0,4,5,6,
	7,4,5,1,
	2,6,7,3
    };

    int spath[24] = {0,1,2,3,1,5,6,2,2,6,7,3,3,7,4,0,0,4,5,1,5,4,7,6};
    int i,j;

    for ( i = 0; i < 2; i++ ) {
	v[i][0]=x1;
	v[i+2][0]=x2;
	v[i+4][0]=x3;
	v[i+6][0]=x4;
	v[i][2]=z1;
	v[i+2][2]=z1;
	v[i+4][2]=z2;
	v[i+6][2]=z2;
	v[i+1][1]=y2;
	v[i+5][1]=y4;
	v[3*i][1]=y1;
	v[3*i+4][1]=y3;
    }

    makeobj( o1 = genobj() );

    /* color(GRAY); */
    color(c);
				
    if (SOLID) {
	for( i = 0; i <= 5; ++i ) {
	    bgnpolygon();
	    for(j=0;j<=3;++j)v3f(v[spath[4*i+j]]);	
	    endpolygon();
	}
	color(BLACK);
    }
			
    bgnline();
    for ( i = 0; i < 16; i++ ) {
	v3f(v[wpath[i]]);
    }
    endline();
    closeobj();
    return (o1);
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : cil
                                              
 ** PURPOSE          : This function creates a cylinder.  The top of the 
 **                    cylinder may be a different size than the bottom of
 **                    the cylinder although the top and bottom are parallel.
 **                    This allows us to create cylinders which are skewed
 **                    with respect to x and y axis.  Creates a solid
 **                    cylinder or a wire frame cylinder depending on the
 **                    value of the constant solid.  The center of the top
 **                    of the cylinder may be different from the bottom.
 **
 **                    The cylinder is approximated as two parallel, 2D, 
 **                    twelve sided regula polygons connectd at their vertices
 **
 ** RETURNS          : int which is iris index to cylinder object.      
 ** INPUTS           : r1 -- radius of the bottom, centered at (0, 0, 0).
 **                    r2 -- radius of the top, centered at (x2, y2, l).
 **                    x2 -- x offset for the top.                      
 **                    y2 -- y offset for the top.                      
 **                    l -- length of the cylinder.                     
 **                    c -- color cylinder should be.                        
 ** LOCAL VARIABLES  : o1 -- object index to be returned. 
 **				v -- vertices of wire frame
 **				vs -- vertices of solid surfaces
 **				rs1,2 -- solid radii
 ** GLOBAL VARIABLES : solid                                            
 **                                                                     
 ************************************************************************/

long int cil(float r1, float r2, float x2, float y2, float l, int c) {
    long int o1;
    int i,j;
    float v[2][12][3], vs[2][12][3] /*, org[2][3]*/, rs1, rs2;
	
    rs1 = (r1 * 0.95);	/* smaller radii avoid sufaces overlapping wireframe */
    rs2 = (r2 * 0.95);
	
    /*set up vertices */
    for ( i = 0; i <= 11; ++i ) {
	v[0][i][0] = (r1*cos(i*PI/6.));
	v[0][i][1] = (r1*sin(i*PI/6.));
	v[0][i][2] = 0;
	v[1][i][0] = (r2*cos(i*PI/6.) + x2);
	v[1][i][1] = (r2*sin(i*PI/6.) + y2);
	v[1][i][2] = l;
	vs[0][i][0] = (rs1*cos(i*PI/6.));
	vs[0][i][1] = (rs1*sin(i*PI/6.));
	vs[0][i][2] = 0;
	vs[1][i][0] = (rs2*cos(i*PI/6.) + x2);
	vs[1][i][1] = (rs2*sin(i*PI/6.) + y2);
	vs[1][i][2] = l;
    }

    /* draw object */
    makeobj( o1 = genobj() );

    /* color(GRAY); */
    color(c);

    if (SOLID) {
	/* draw quadrilateral sides of 'cylinder' */ 
	bgnqstrip();
	for ( i = 11; i >= 0; --i ) {
	    v3f(vs[0][i]);
	    v3f(vs[1][i]);
	}
	v3f(vs[0][11]);
	v3f(vs[1][11]);
	endqstrip();
		
	/* draw twelve sided ends of 'cylinder' */
	bgnpolygon();
	for ( i = 0; i <= 11; ++i ) {
	    v3f(vs[1][i]);
	}
	endpolygon();
	bgnpolygon();
	for ( i = 11; i >= 0; --i) {
	    v3f(vs[0][i]);
	}
	endpolygon();
		
	color(BLACK);
    }
	
    /*draw wire frame*/
    for (i=0;i<=10;++i) {
	bgnline();
	v3f(v[0][i]);
	v3f(v[0][i+1]);
	v3f(v[1][i+1]);	
	v3f(v[1][i]);
	endline();
    }
    bgnline();
    v3f(v[0][11]);
    v3f(v[0][0]);
    v3f(v[1][0]);
    v3f(v[1][11]);
    endline();	

    closeobj();
    return (o1);
}



/**************************************************************************
 * This source code contains trade secrets of FMC and is not to be
 * duplicated, reproduced or otherwise replicated.  Unpublished
 * copyright 1990.  All rights reserved by FMC under Federal Copyright
 * Law.
 **************************************************************************/

/**************************************************************************
 * modified 4/92 by a.kreider to run on univ of mn human motion lab
 * iris i have kept things such as arm geometry and joint angle zero
 * positions as they were in the original code
 *
 * modified 12/94 by Curtis Olson (clolson@me.umn.edu, curt@sledge.mn.org)
 **************************************************************************/

#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>
#include <stdio.h> 
#include <termio.h> 

#include "armsim.h"
#include "mk_prim.h"
#include "model.h"


/************************************************************************
 **                                                                     
 ** w1 -- window to draw robot in.                                      
 ** obstacles - obstacles in robot's workspace.
 ** p1 -- robot arm from base to 1st roll joint.                        
 ** p2 -- robot arm from 1st roll joint to 1st pitch joint.             
 ** p3 -- robot arm from 1st pitch joint to 2nd roll joint.             
 ** p4 -- robot arm from 2nd roll joint to 2nd pitch joint.             
 ** p5 -- robot arm from 2nd pitch joint to 3rd roll joint.             
 ** p6 -- robot arm from 3rd roll joint to 3rd pitch joint.             
 ** p7 -- robot arm from 3rd pitch joint to 4th roll joint.             
 ** h1 -- end effector from torque force senser to fingers.             
 ** f1 -- larger lower part of the finger.                               
 ** f2 -- midsection part of the finger.                                
 ** f3 -- upper portion of the finger.                                  
 ** base -- structure upon which the arms sit.                          
 **                                                                     
 ************************************************************************/

long int w1, obstacles, p1, p2, p3, p4, p5, p6, p7, h1, f1, f2, f3, base;

float data[20000][32];    

/*                                     |   Arm   | Gripper
                                  -----|---------|----------
9000 data points is enough for    Arm1 |  0 -  6 |  7 - 15
5 minutes at 30 frames/sec.       -----|---------|----------
                                  Arm2 | 16 - 22 | 23 - 31       */

/* the above defines the correspondence between the array 'data' and
   the joint angle data.  joint angle data is provided to the program
   in the form of matrix x output.  the matrix x output should be four
   matrices, one for each arm and end effector.  arm matrices are n
   rows by 7 columns where each column is a joint parameter and each
   row is a time step.  gripper matrices are n rows by 9 columns.  the
   three line header appended by matrix x to the beginning of each
   matrix must be left intact or the program will not perform
   correctly */


/************************************************************************
 **ok
 ** FUNCTION NAME    : makeobstacles
 ** PURPOSE          : This function creates all obstacles in the robot's
 **                    workspace.
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.
 ** LOCAL VARIABLES  : p -- object to be returned.
 **                    sp1 -- obstacle1
 ** FUNCTIONS CALLED : box
 **
 ************************************************************************/

long int makeobstacles(char *ob_filename) {
    long int p;			/* pointer to object returned */
    long int spbox[50];		/* Array of pointers 
				   to sub-objects to be returned */
    /* The sub-object are obstacles, and the limit of 50 is arbitrary */
    float boxpoints[50][10]; /* 10 points necessary for box routine   */
    float x,y,z;
    float xbmin, xbmax, ybmin, ybmax, xtmin, xtmax, ytmin, ytmax, zmin, zmax;
    int i, j, color, count, boxcount, cylindercount, type, xint, yint, zint;
    FILE *obfile;

    if ( (obfile = fopen(ob_filename,"r")) == NULL ) {
	printf("Getdata cannot open data file: %s\n", ob_filename);
	exit(-2);
    }

    /* Get the number of obstacles in the file */
    fscanf(obfile, "%d", &count);

    /* Now get the data points from the file */
    /* (for now we assume all boxes - can add cylinders later */

    boxcount = 0;
    cylindercount = 0;

    for (i = 0; i < count; i++) {
	fscanf(obfile,"%d",&type);
		
	if (type == 1) {	/* type 1 indicates box*/
	    xbmin = 1000;
	    xbmax = -1000;
	    ybmin = 1000;
	    ybmax = -1000;
	    xtmin = 1000;
	    xtmax = -1000;
	    ytmin = 1000;
	    ytmax = -1000;
	    zmin = 2000;
	    zmax = -2000;
 
	    /* ok - we have a box(rectangle) */
	    /* 8 vertices (x,y,z) describe it */
	    for(j = 0; j < 8; j++) {
		/* Read vertices of box */
		fscanf(obfile,"%d,%d,%d",&xint,&yint,&zint);

		/*
		 * Convert from 128x128x128 coordinate
		 * system to (+/-530, +/-530, 0-1060) coords
		 */
		x = ((float)xint * 8.28125) - 530.0;
		y = ((float)yint * 8.28125) - 530.0;
		z = ((float)zint * 8.28125);

		/* get bottom and top min,max x,y and min/max z */
		if ( j <= 3 ) {/* j < 3 indicates bottom */
		    if (x < xbmin) {
			xbmin = x;
		    } else if (x > xbmax) {
			xbmax = x;
		    }

		    if (y < ybmin) {
			ybmin = y;
		    } else if (y > ybmax) {
			ybmax = y;
		    }
		} else {
		    if (x < xtmin) {
			xtmin = x;
		    } else if (x > xtmax) {
			xtmax = x;
		    }

		    if (y < ytmin) {
			ytmin = y;
		    } else if (y > ytmax) {
			ytmax = y;
		    }
		} /* end max/min top/bottom x/y */

		if (z < zmin) {
		    zmin = z;
		} else if (z > zmax) {
		    zmax = z;
		}

	    } /* end for j ... */
			
	    /* Save info necessary for box routine  */
	    boxpoints[boxcount][0] = xbmin;
	    boxpoints[boxcount][1] = xbmax;
	    boxpoints[boxcount][2] = xtmin;
	    boxpoints[boxcount][3] = xtmax;
	    boxpoints[boxcount][4] = ybmin;
	    boxpoints[boxcount][5] = ybmax;
	    boxpoints[boxcount][6] = ytmin;
	    boxpoints[boxcount][7] = ytmax;
	    boxpoints[boxcount][8] = zmin;
	    boxpoints[boxcount][9] = zmax;
	    
	    /********************************************
	      printf("Points for box: %d\n",boxcount);
	      for (j = 0; j < 10; j++) {
		  printf("%f ",boxpoints[boxcount][j]);
	      }
	      printf("\n");
	      *********************************************/
	

	    /* increment box count */
	    boxcount++; 
	    /* end if type == 1 */
	} else {
	    printf("Error: Obstacle is type cylinder!\n");
	    printf("Not implemented yet!!!");
	    exit(-1);

	} /* obstacle type check */

    } /* end reading in the data (for i = ....) */
    fclose(obfile);

    /* now make the subobjects - cylinders not implemented yet */
    color = BLUE;		/* assume base is object zero, make it blue */
    for (i=0; i < boxcount; i++) {
	spbox[i] = box(boxpoints[i][0], boxpoints[i][1], 
		       boxpoints[i][2], boxpoints[i][3], 
		       boxpoints[i][4], boxpoints[i][5], 
		       boxpoints[i][6], boxpoints[i][7], 
		       boxpoints[i][8], boxpoints[i][9],
		       color);
	/* make rest of obstacle objects red */
	color = RED;
    }

    /*
     * finally, make one object with all the obstacles in it, and
     * return it 
     */
    makeobj( p = genobj() );
    for ( i = 0; i < boxcount; i++ ) {
	callobj(spbox[i]);
    }
    closeobj();
    return(p);
	
} /* end makeobstacles */


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makebase                                         
 ** PURPOSE          : This function creates an object which is the base
 **                    which the robot arms are attached to.            
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.                                            
 ** LOCAL VARIABLES  : p -- object to be returned.                      
 **                    sp1 -- lower portion of the base.                
 **                    sp2 -- midsection of the base.                   
 **                    sp3 -- upper section of the base.                
 ** FUNCTIONS CALLED : box                                              
 **                                                                    
 ************************************************************************/

long int makebase(void) {
    long int p, sp1, sp2, sp3, sp4; 

    sp1 = box (0., 280., 0., 280.,
	       60., 180., 0., 180., 0., 120., BLUE);
    sp2 = box (0., 280., 0., 280.,
	       0., 180., 0., 180., 120., 150., BLUE); 
    sp3 = box (0., 280., 0., 280.,
	       0., 180., 0., 90., 150., 240., BLUE); 
    makeobj( p = genobj() );
    callobj(sp1); 
    callobj(sp2); 
    callobj(sp3);
    closeobj(); 
    return(p); 
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : drawobstacles                                         
 ** PURPOSE          : This function draws the obstacles in the robots
 **		       environment in the appropriate locations.             
 ** RETURNS          : none                                            
 ** INPUTS           : none                                             
 ** GLOBAL VARIABLES : obstacles                                             
 **                                                                     
 ************************************************************************/

void drawobstacles(void) {
    pushmatrix(); 
    /* translate (0.0, 0.0, 0.0); */
    callobj(obstacles); 
    popmatrix(); 
    return;
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : drawbase                                         
 ** PURPOSE          : This function draws the base of the robot in the 
 **                    appropriate location.                            
 ** RETURNS          : none                                            
 ** INPUTS           : none                                             
 ** GLOBAL VARIABLES : base                                             
 **                                                                     
 ************************************************************************/

void drawbase(void) {
    pushmatrix (); 
    /* translate (260., 355., 10.); */
    callobj (base); 
    popmatrix (); 
    return;
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : drawhand                                         
 ** PURPOSE          : This function draws out an end effector.         
 ** RETURNS          : none.                                            
 ** INPUTS           : a1 -- angle of rotation for 1st finger.          
 **                    a2 -- 1st finger, middle joint angle.            
 **                    a3 -- 1st finger, top joint angle.               
 **                    a4 -- angle of rotation for 2nd finger.          
 **                    a5 -- 2nd finger, middle joint angle.            
 **                    a6 -- 2nd finger, top joint angle.               
 **                    a7 -- angle of rotation for 2nd finger.          
 **                    a8 -- 3rd finger, middle joint angle.            
 **                    a9 -- 3rd finger, top joint angle.               
 ** GLOBAL VARIABLES : h1, f1, f2, f3                                   
 **                                                                     
 ************************************************************************/

void drawhand (float a1, float a2, float a3, float a4, float a5, float a6, 
	       float a7, float a8, float a9)
{
    callobj (h1);
    translate (-12.5, 0., 39.75);
    pushmatrix ();  
    translate (0., 17.5, 0.); 
    rotate (-900, 'z');
    rot (a1, 'z');  
    callobj (f1);
    translate (0., 21.875, 49.5); 
    rot (a2, 'x');   
    callobj (f2); 
    translate (0., 0., 22.5);
    rot (a3, 'x');  
    callobj (f3);  
    popmatrix ();  
    pushmatrix ();  
    rotate (-900, 'z');
    callobj (f1); 
    translate (0., 21.875, 49.5);
    rot (a4, 'z');
    rot (a5, 'x');   
    callobj (f2);  
    translate (0., 0., 22.5);
    rot (a6, 'x');  
    callobj (f3);  
    popmatrix ();  
    pushmatrix ();  
    translate (0., -17.5, 0.); 
    rotate (-900, 'z');
    rot (a7, 'z');  
    callobj (f1); 
    translate (0., 21.875, 49.5); 
    rot (a8, 'x');   
    callobj (f2);  
    translate (0., 0., 22.5);
    rot (a9, 'x');  
    callobj (f3);  
    popmatrix ();  
    return;
} 


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : drawarm                                          
 ** PURPOSE          : This function draws an arm at the appropriate location
 **                    and transforms the shape of the arm base upon the
 **                    input angles of the 7 degrees of freedom and 9 angles
 **                    describing the end effector.                     
 ** RETURNS          : none                                             
 ** INPUTS           : x -- x coordinate of the arm being drawn.        
 **                    y -- y coordinate of the arm being drawn.        
 **                    z -- z coordinate of the arm being drawn.        
 **                    a1 -- angle of first roll joint.                 
 **                    a2 -- angle of first pitch joint.                
 **                    a3 -- angle of second roll joint.                
 **                    a4 -- angle of second pitch joint.               
 **                    a5 -- angle of third roll joint.                 
 **                    a6 -- angle of third pitch joint.                
 **                    a7 -- angle of fourth roll joint.                
 **                    h1 -- angle of rotation for 1st finger.          
 **                    h2 -- 1st finger, middle joint angle.            
 **                    h3 -- 1st finger, top joint angle.               
 **                    h4 -- angle of rotation for 2nd finger.          
 **                    h5 -- 2nd finger, middle joint angle.            
 **                    h6 -- 2nd finger, top joint angle.               
 **                    h7 -- angle of rotation for 2nd finger.          
 **                    h8 -- 3rd finger, middle joint angle.            
 **                    h9 -- 3rd finger, top joint angle.               
 ** GLOBAL VARIABLES : p1, p2, p3, p4, p5, p6, p7
 ** FUNCTIONS CALLED : drawhand                                         
 **                                                                     
 ************************************************************************/

void drawarm (float x, float y, float z, float a1, float a2, float a3, 
	      float a4, float a5, float a6, float a7, float h1, float h2, 
	      float h3, float h4, float h5, float h6, float h7, float h8, 
	      float h9)
{
    pushmatrix (); 
    translate (x, y, z);
    rotate (-900, 'z');		/* rotate - angle in 10*degrees */
    callobj (p1); 
    translate (0., 0., 109.); 
    rot (-a1, 'z');  		/* rot - angle in degrees */
    callobj (p2);
    translate (31.5, 0., 39.);
    rot (a2, 'y'); 
    callobj (p3);
    translate (-31.5, 0., 39.); 
    rot (-a3, 'z'); 
    callobj (p4);
    translate (26.5, 0., 91.);
    rot (a4, 'y');  
    callobj (p5);  
    translate (-26.5, 0., 34.);
    rot (-a5, 'z');  
    callobj (p6); 
    translate (0., 0., 96.);
    rot (a6, 'y');  
    callobj (p7); 
    translate (0., 0., 31.);
    rot (-a7, 'z');  
    drawhand (h1, h2, h3, h4, h5, h6, h7, h8, h9);  
    popmatrix (); 
    return;
}  


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : drawarm1                                         
 ** PURPOSE          : This function draws out the robots right arm on the
 **                    base.                                            
 ** RETURNS          : none                                             
 ** INPUTS           : a1 -- angle of first roll joint.                 
 **                    a2 -- angle of first pitch joint.                
 **                    a3 -- angle of second roll joint.                
 **                    a4 -- angle of second pitch joint.               
 **                    a5 -- angle of third roll joint.                 
 **                    a6 -- angle of third pitch joint.                
 **                    a7 -- angle of fourth roll joint.                
 **                    h1 -- angle of rotation for 1st finger.          
 **                    h2 -- 1st finger, middle joint angle.            
 **                    h3 -- 1st finger, top joint angle.               
 **                    h4 -- angle of rotation for 2nd finger.          
 **                    h5 -- 2nd finger, middle joint angle.            
 **                    h6 -- 2nd finger, top joint angle.               
 **                    h7 -- angle of rotation for 2nd finger.          
 **                    h8 -- 3rd finger, middle joint angle.            
 **                    h9 -- 3rd finger, top joint angle.               
 ** FUNCTIONS CALLED : drawarm                                          
 **                                                                     
 ************************************************************************/

void drawarm1 (float a1, float a2, float a3, float a4, float a5, 
	       float a6, float a7, float h1, float h2, float h3, 
	       float h4, float h5, float h6, float h7, float h8, 
	       float h9)
{

    /* FIRST THREE VALUES ARE THE BOTTOM CENTER OF THE BASE */
    drawarm (0., 0., 397.5, a1, a2, a3, a4, a5, a6, a7, 
	     h1, h2, h3, h4, h5, h6, h7, h8, h9); 
    return;
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : drawarm2                                         
 ** PURPOSE          : This function draws out the robots left arm on the
 **                    base.                                            
 ** RETURNS          : none                                             
 ** INPUTS           : a1 -- angle of first roll joint.                 
 **                    a2 -- angle of first pitch joint.                
 **                    a3 -- angle of second roll joint.                
 **                    a4 -- angle of second pitch joint.               
 **                    a5 -- angle of third roll joint.                 
 **                    a6 -- angle of third pitch joint.                
 **                    a7 -- angle of fourth roll joint.                
 **                    h1 -- angle of rotation for 1st finger.          
 **                    h2 -- 1st finger, middle joint angle.            
 **                    h3 -- 1st finger, top joint angle.               
 **                    h4 -- angle of rotation for 2nd finger.          
 **                    h5 -- 2nd finger, middle joint angle.            
 **                    h6 -- 2nd finger, top joint angle.               
 **                    h7 -- angle of rotation for 2nd finger.          
 **                    h8 -- 3rd finger, middle joint angle.            
 **                    h9 -- 3rd finger, top joint angle.               
 ** FUNCTIONS CALLED : drawarm                                          
 **                                                                     
 ************************************************************************/

void drawarm2 (float a1, float a2, float a3, float a4, float a5, 
	       float a6, float a7, float h1, float h2, float h3, 
	       float h4, float h5, float h6, float h7, float h8, 
	       float h9)
{
    drawarm (490., 400., 250., a1, a2, a3, a4, a5, a6, a7, 
	     h1, h2, h3, h4, h5, h6, h7, h8, h9); 
    return;
}


/************************************************************************
 **                                                                     
 ** FUNCTION NAME    : getdata                                          
 ** PURPOSE          : this function reads in the data specified in the  
 **                    program call and builds the main data array.  
 ** RETURNS          : Nothing. 
 ** INPUTS           : count -- the number of data points to be simulated.
 **                    arm1 -- indicates origin of arm1 data.
 **                    hand1 -- indicates origin of hand1 data.  
 **                    arm2 -- indicates origin of arm2 data.
 **                    hand2 -- indicates origin of hand2 data.  
 ** LOCAL VARIABLES  : i, j, k -- loop counters 
 **                    in -- input file pointer.
 **                    temp -- data from file yet to have unit conversion.  
 ** GLOBAL VARIABLES : data -- main data array.  
 **                                                                     
 ************************************************************************/

void getdata (int *count, char *arm1dat) {
    int i, j, k; 
    FILE *planfile;
    float temp;


    if ( (planfile = fopen(arm1dat,"r")) == NULL ) {
	printf("Getdata cannot open data file: %s\n", arm1dat);
	exit(-2);
    }

    /*
     * Get the number of entries in the plan stack (number of entries
     * in stack - 1) 
     */
    fscanf(planfile,"%d",count);
	
    /* Zero out the appropriate entries in the data table */
    for (i=0; i <= *count; i++) {
	for (j=0; j<=31; j++) data[i][j]=0;
    }

    /* Get the start configuration and discard (repeated later) */
    for(j = 0; j < DOF; j++) {
	fscanf(planfile,"%f",&temp);
	data[0][j] = (temp * 180.0) / PI;
    }


    /* Get the joint deltas out of the file and discard */
    for(i = 0; i < DOF; i++) {
	fscanf(planfile,"%f",&temp);
    }

    /* Get the link lengths out of the plan file and discard */
    for(i = 0; i < DOF; i++) {
	fscanf(planfile,"%f",&temp);
    }

    /*
     * get the path data (start configuration is repeated in these)
     * and convert it from degrees to radians 
     */
    for (i = 0; i < *count; i++) {
	for ( j = 0; j < DOF; j++) {
	    fscanf (planfile, "%f", &temp);
	    data[i][j] = (temp * 180.0) / PI;
	}
    }
    
    fclose (planfile); 

    /* make duplicate first arm data for second arm */
    for (i = 0; i < *count; i++) {
	for (j = 0; j < DOF; j++) {
	    data[i][j+16] = data[i][j]; 
	}
    } 

    /*
     * leave the hands in the home position for both arms (i.e. all
     * entries in the matrix should be zero) 
     */

    return;
}

/************************************************************************** 
 *commented out code used for random motion generation to test animation. *
 {	
     int i;

     *count = 100;

     for(i = 0; i <= *count; ++i)
       {
	   data[i][0] = 0.;  	 **closest to base** 
	     data[i][1] = 0.;  
	   data[i][2] = 0;  
	   data[i][3] = 0.;  
	   data[i][4] = 0.;  
	   data[i][5] = 45.;  
	   data[i][6] = 0.; 	 ***furthest from base**** 

	     data[i][7] = 90.;  	 *outer finger pedestal rotate *
	       data[i][8] = -45.;  	 *outer finger first digit flex *
		 data[i][9] = 45.;  	 *outer finger end digit flex *
		   data[i][10] = 30.;  	 *mid fngr rotate (pedestal fixed) *
		     data[i][11] = -90.;  	 *mid fngr first digit flex *
		       data[i][12] = 60.;  	 *mid fngr end digit flex *
			 data[i][13] = -120.;  	 *other outer finger as first *
			   data[i][14] = 90.;  
	   data[i][15] = -90.;
  
	   data[i][16] = 0.;  	 *arm2 as arm1* 
	     data[i][17] = 0.;  
	   data[i][18] = 0.;  
	   data[i][19] = 0.;  
	   data[i][20] = 30.;  
	   data[i][21] = 60.;  
	   data[i][22] = 45.;
 
	   data[i][23] = 90.;  	 *hand2 as hand1 *
	     data[i][24] = -90.;  
	   data[i][25] = 90.;  
	   data[i][26] = 0.;  
	   data[i][27] = -90.;  
	   data[i][28] = 90.;  
	   data[i][29] = -90.;  
	   data[i][30] = -90.;  
	   data[i][31] = 90.;  
       } 

     return;
 } 
 ********************************************************************/


/**************************************************************
  mdlvw sets up a polar modelview matrix
  rot - angle in xy plane from robot to viewpoint, measured from
  front of robot in degrees*10
  elev - declination from +z axis (above robot) in degrees*10 
  ***************************************************************/

void mdlvw(int rot, int elev) {
    popmatrix();	   /* get rid of old matrix */
    pushmatrix();	   /* save an extra identity matrix for the next time */
    /* orthographic projection */
    ortho (-530., 530., -530., 530., -1060.,1060.);  
    /* perspective projection */
    /* pespective(900, 1.0, 10.0,1060.); */
    polarview(500.,rot,elev,0);
    /* translate(-400.,-400.,-530.); */
    translate(0.0,0.0,-530.);
    if (SOLID) {
	zbuffer (TRUE);
	zclear (); 
    }
    return;
}	


/************************************************************************
 ** FUNCTION NAME	: smoothdata
 ** PURPOSE		: function smooths the data points at each point
 **			: in the precomputed path by averaging them over
 **			  a user-specified width
 ** RETURNS		: Returns - the smoothed points
 ** INPUTS		: The max radius of the points to be averaged, and the
 **			  count of points, the index of the point to be
 **			  smoothed in the data array
 ** GLOBAL VARS		: data
 **
 ************************************************************************/

void smoothdata(int index, int count, int radius, double *smoothpoints) {
    int i, j, span, acceptable;
    double tmp;
 
    /*
     * Check to make sure we are at a point in the path with enough
     * points to average 
     */

    acceptable = 0;		/* set acceptable flag to false */
    while ((radius > 0) && (!acceptable)) {
	if ( (index < radius) || ((index + radius) >= count) ) {
	    radius = radius/2;
	} else {
	    acceptable = 1;	/* found an acceptable radius */
	}
    } /* end while */

    /*
     * if an acceptable radius has been found, then go ahead and
     * smooth the points with it. Otherwise, just copy the points at
     * the current data index to the smooth points 
     */
    if (acceptable) {
	/* find the number of points we are averaging together */
	span = (double)(radius * 2 + 1);

	/*
	 * for each point, we compute an average over the span of
	 * points centered at index 
	 */
	for (i = 0; i < 16; i++) {
	    tmp = 0;
	    for ( j = (index - radius); j <= (index + radius); j++) {
		tmp += data[j][i];
	    }

	    smoothpoints[i] = tmp/span;
	}
    } else {
	for (i = 0; i < 16; i++) {
	    smoothpoints[i] = data[index][i];
	}
    }

    return;

} /* end smoothpoints */


/************************************************************************
 **                                                                     
 ** FUNCTION NAME    : demo                                           
 ** PURPOSE          : This function actually does simulation of arm motion
 **                    using data from the main data array.  
 ** RETURNS          : Nothing.  
 ** INPUTS           : speed -- initial sampling rate for displaying simulation. 
 **                    count -- number of data points in main data array.  
 ** LOCAL VARIABLES  : i -- loop counter.  
 ** GLOBAL VARIABLES : data -- main data array.  
 ** FUNCTIONS CALLED : drawbase, drawarm1, drawarm2  
 **                                                                     
 ************************************************************************/

void demo (int speed, int count, int radius) {
    int i, elev, rot, a1, a2, rev, spd, stop;
    short in;
    char x;
    double	smoothpoints[16];

    /************
      qdevice(KEYBD);	* set up keyboard as input device *
      *************/

    qdevice(F1KEY);		/* Function key F1 - on/off arm1 */
    qdevice(F2KEY);		/* Function key F2  - on/off arm2 */
    qdevice(ESCKEY);		/* Escape key to quit */
    qdevice(LEFTARROWKEY);	/* Left arrow to speed up */
    qdevice(RIGHTARROWKEY);	/* Right arrow to slow down */
    qdevice(RKEY);		/* Rewind to start of simulation */
    qdevice(FKEY);		/* Fast forward to end of simulation */
    qdevice(TKEY);		/* Toggle direction of Simulation */

    /*********************/
    qreset();

    i = 0;			/* initialize time slice pointer */
    a1=a2=TRUE;
    stop=rev=FALSE;
    elev=900;
    rot=0;
    spd=speed;
	

    while (!stop) {
	while (qtest())	{ /* read and act on accumulated user commands */
	    switch (qread(&in)) {
	      case F1KEY:	/* arm1 on/off toggle */
		if (in == 1) {
		    a1=!a1;
		    break;
		}

	      case F2KEY:	/* arm2 on/off toggle */
		if (in == 1) {
		    a2=!a2;
		    break;
		}

	      case LEFTARROWKEY: /* speed up */
		if (in == 1) {
		    ++spd;
		    break;
		}

	      case RIGHTARROWKEY:	/* slow down */
		if (in == 1) {
		    if (spd>0) --spd;
		    break;
		}

	      case RKEY:	/* rewind to first time slice */
		if (in == 1) {
		    i=0;
		    break;
		}

	      case FKEY:	/* fast forward to last time slice */
		if (in == 1) {
		    i=count;
		    break;
		}

	      case TKEY: /* Toggle direction of simulation*/
		if (in == 1) {
		    rev=!rev;
		    break;
		}

	      case ESCKEY: /* end simulation */
		if (in == 1) {
		    stop=TRUE;
		    break;
		}

	    } /* end of switch */


	} /* end of while(qtest) loop */
	
	color(WHITE);
	clear();
	mdlvw(rot,elev);
	
	drawobstacles();

	smoothdata(i, count, radius, smoothpoints);

	if (a1) {
	    drawarm1 (smoothpoints[0], smoothpoints[1], smoothpoints[2],
		      smoothpoints[3], smoothpoints[4], smoothpoints[5],
		      smoothpoints[6], smoothpoints[7], smoothpoints[8],
		      smoothpoints[9], smoothpoints[10], smoothpoints[11],
		      smoothpoints[12], smoothpoints[13], smoothpoints[14],
		      smoothpoints[15]);
	}


	/******************************************************************
	  if (a2) { 
	      drawarm2 (data[i][16], data[i][17], data[i][18], data[i][19], 
	      data[i][20], data[i][21], data[i][22], data[i][23], 
	      data[i][24], data[i][25], data[i][26], data[i][27], 
	      data[i][28], data[i][29], data[i][30], data[i][31]);
	  }
	  ******************************************************************/
	  

	  /* get next data point */	  
	  if (rev) {
	      if (i-spd >= 0) {
		  i-=spd;
	      }
	  } else if (i+spd < count) {
	      i += spd;
	  }

	  swapbuffers();

    } /* end of while (!stop) loop */

    return;
}


/************************************************************************
 **ok                                                                 
 ** FUNCTION NAME    : hello                                            
 ** PURPOSE          : This function is a initialization function.  It does
 **                    several things including opening graphics window,
 **                    setting up various transformations, enables z-buffering
 **                    if appropriate, and generates robot parts to be drawn.
 ** RETURNS          : none                                             
 ** INPUTS           : none                                             
 ** GLOBAL VARIABLES : solid, p1, p2, p3, p4, p5, p6, p7, h1, f1, f2, f3, 
 **                    base, w1                                            
 ** FUNCTIONS CALLED : makep1, makep2, makep3, makep4, makep5, makep6,  
 **                    makep7, makeh1, makef1, makef2, makef3, makebase 
 **                                                                     
 ************************************************************************/       
void hello (char *obfilename) {
    prefsize(500,500);

    /****************************
      keepaspect (1, 1);  
      ****************************/

    w1 = winopen ("Parallel Motion Planning Demonstration");
    /* drawmode(NORMALDRAW);  */

    /************************************
      ortho (-530., 530., -530., 530., 0.,1060.);  
      *************************************/
    /***********************************
      mapcolor(GRAY,150,150,150);
      RGBmode();
      if (SOLID)
      {
      zbuffer (TRUE);
      zclear (); 
      backface(TRUE);
      }
      ************************************/

    doublebuffer (); 
    gconfig ();
    frontbuffer(TRUE);
    backbuffer(TRUE);
    color(WHITE);
    clear();
    frontbuffer(FALSE);
    /* mmode(MVIEWING); */

    pushmatrix();	

    p1 = makep1 ();  
    p2 = makep2 ();  
    p3 = makep3 ();  
    p4 = makep4 ();  
    p5 = makep5 ();  
    p6 = makep6 ();  
    p7 = makep7 ();  
    h1 = makeh1 (); 
    f1 = makef1 (); 
    f2 = makef2 (); 
    f3 = makef3 (); 

    /*******************
      base = makebase ();  
      *******************/

    obstacles = makeobstacles(obfilename);
    return;
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : strgtoint                                         
 ** PURPOSE          : This function changes a string of digits to an int.  
 ** RETURNS          : int value                                             
 ** INPUTS           : s -- string to be converted. 
 ** LOCAL VARIABLES  : i -- loop counter to index through the digits.
 **                  : n -- keeps track of the order of mag of digits.  
 ** GLOBAL VARIABLES : solid                                            
 **                                                                     
 ************************************************************************/

int strgtoint (char *s) {
    int i, n;
	
    n = 0; 
    for (i = 0; s[i] >= '0' && s[i] <= '9'; ++i) {
	n = 10 * n + s[i] - '0';
    }
    return(n);
} 


/************************************************************************
 **                                                                     
 ** MAIN ROUTINE
 **                                                                     
 ************************************************************************/

main (int argc, char **argv) {
    int speed,count,smooth_radius; 

    /***************
      foreground();
      ****************/
    if (argc != 4) {
	printf("ERROR: usage: %s <plan file> <obstacle file> <smoothing radius>\n", argv[0]);
	exit(-1);
    }
    getdata (&count, argv[1]); 
    printf("count equals %d\n",count);
    printf("Got the data%d\n",count);
    hello (argv[2]);
    printf("back from hello\n");
    speed = 1;  
    smooth_radius = atoi(argv[3]);
    demo (speed,count,smooth_radius); 
    gexit();
    return 0; 
}



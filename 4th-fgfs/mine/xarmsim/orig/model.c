/* model.c
 */


#include <gl/gl.h>    /* COLOR definitions */
#include <gl/device.h>

#include "armsim.h"
#include "mk_prim.h"
#include "model.h"


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makep1                                           
 ** PURPOSE          : This function creates an object which is the robot
 **                    arm from the base to the first roll joint.       
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.                                            
 ** LOCAL VARIABLES  : p -- object to be returned.                      
 **                    sp1 -- subpart 1, cylinder from base to 1st roll joint
 **                           interface.                                
 **                    sp2 -- subpart 2, 1st roll joint interface closest
 **                           to the base.                              
 ** FUNCTIONS CALLED : cil                                              
 **                                                                     
 ************************************************************************/

long int makep1 (void) {
    long int p, sp1, sp2;

    sp1 = cil (31.5, 31.5, 0., 0., 101.5, BLUE);
    sp2 = cil (31.5, 31.5, 0., 0., 7.5, CYAN);  
    makeobj(p = genobj() );
    pushmatrix ();  
    callobj (sp1);
    translate (0., 0., 101.5); 
    callobj (sp2); 
    popmatrix ();  
    closeobj (); 
    return (p);
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makep2                                           
 ** PURPOSE          : This function makes an object which is the robot arm
 **                    from the first roll joint to the first pitch joint.
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.                                            
 ** LOCAL VARIABLES  : p -- object to be returned.                      
 **                    sp1 -- top of 1st roll joint interface.          
 **                    sp2 -- arm from 1st roll joint to 1st pitch joint.
 **                    sp3 -- harmonic drive of 1st pitch joint.        
 ** FUNCTIONS CALLED : cil                                              
 **                                                                    
 ************************************************************************/

long int makep2 (void) {
    long int p, sp1, sp2, sp3;

    sp1 = cil (31.5, 31.5, 0., 0., 7.5, CYAN);     
    sp2 = cil (31.5, 26.5, 31.5, 0., 31.5, BLUE);
    sp3 = cil (31.5, 31.5, 0., 0., 80., GREEN);  
    makeobj( p = genobj() );
    pushmatrix (); 
    callobj (sp1);
    translate (0., 0., 7.5);  
    callobj (sp2);
    translate (31.5, 40., 31.5);
    rotate (900, 'x');  
    callobj (sp3); 
    popmatrix ();
    closeobj ();
    return (p);
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makep3                                           
 ** PURPOSE          : This function creates an object which is the robot
 **                    arm from the 1st pitch joint to the 2nd roll joint.
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.                                            
 ** LOCAL VARIABLES  : p -- object to be returned.                      
 **                    sp1 -- arm from 1st pitch joint to 2nd roll joint.
 **                    sp2 -- lower half of 2nd roll joint interface.   
 ** FUNCTIONS CALLED : cil                                              
 **                                                                   
 ************************************************************************/

long int makep3(void) {
    long int p, sp1, sp2;

    sp1 = cil (21.5, 26.5, -31.5, 0., 31.5, BLUE);
    sp2 = cil (26.5, 26.5, 0., 0., 7.5, CYAN);  
    makeobj( p = genobj() );  
    pushmatrix();
    callobj(sp1);
    translate (-31.5, 0., 31.5); 
    callobj(sp2);  
    popmatrix(); 
    closeobj(); 
    return(p);
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makep4                                           
 ** PURPOSE          : This function produces that part of the robot arm
 **                    which lies between the 2nd roll joint and the 2nd
 **                    pitch joint.                                     
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.                                            
 ** LOCAL VARIABLES  : p -- object to be returned.                      
 **                    sp1 -- 2nd roll joint upper interface.           
 **                    sp2 -- straight portion of arm between the 2nd roll
 **                           joint and the 2nd pitch joint.            
 **                    sp3 -- squewed portion of arm between the 2nd roll
 **                           joint and the 2nd pitch joint.            
 **                    sp4 -- harmonic drive of 2nd pitch joint.        
 ** FUNCTIONS CALLED : cil                                              
 **                                                                    
 ************************************************************************/

long int makep4 (void) {
    long int p, sp1, sp2, sp3, sp4;

    sp1 = cil (26.5, 26.5, 0., 0., 7.5, CYAN);
    sp2 = cil (26.5, 26.5, 0., 0., 57., BLUE);  
    sp3 = cil (26.5, 21.5, 26.5, 0., 26.5, BLUE);
    sp4 = cil (26.5, 26.5, 0., 0., 70., GREEN);
    makeobj( p = genobj() ); 
    pushmatrix(); 
    callobj(sp1);
    translate(0., 0., 7.5);
    callobj(sp2);
    translate(0., 0., 57.); 
    callobj(sp3);  
    translate(26.5, 35., 26.5); 
    rotate(900, 'x'); 
    callobj(sp4); 
    popmatrix(); 
    closeobj(); 
    return(p);
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makep5                                           
 ** PURPOSE          : This function produces the portion of the robot arm
 **                    which lies between the 2nd pitch joint and the 3rd
 **                    roll joint.                                      
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.                                            
 ** LOCAL VARIABLES  : p -- object to be returned.                      
 **                    sp1 -- squewed arm segment from 2nd pitch joint to
 **                           3rd roll joint interface.                 
 **                    sp2 -- 3rd lower roll joint interface.           
 ** FUNCTIONS CALLED : cil                                              
 **                                                                    
 ************************************************************************/

long int makep5(void) {
    long int p, sp1, sp2;

    sp1 = cil (16.5, 21.5, -26.5, 0., 26.5, BLUE);
    sp2 = cil (21.5, 21.5, 0., 0., 7.5, CYAN);  
    makeobj( p = genobj() ); 
    pushmatrix();
    callobj(sp1);
    translate(-26.5, 0., 26.5); 
    callobj(sp2);  
    popmatrix(); 
    closeobj();
    return(p);
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makep6                                           
 ** PURPOSE          : This function produces that portion of the robot 
 **                    arm which lies between the 3rd roll joint and the
 **                    3rd pitch joint.                                 
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.                                            
 ** LOCAL VARIABLES  : p -- object to be returned.                      
 **                    sp1 -- 3rd roll joint upper interface.           
 **                    sp2 -- straight portion of arm between the 3rd roll
 **                           joint and the 3rd pitch joint.            
 **                    sp3 -- squewed portion of arm between the 3rd roll
 **                           joint and the 3rd pitch joint.            
 **                    sp4 -- harmonic drive of 3rd pitch joint.        
 ** FUNCTIONS CALLED : cil                                              
 **                                                                    
 ************************************************************************/

long int makep6(void) {
    long int p, sp1, sp2, sp3, sp4;

    sp1 = cil (21.5, 21.5, 0., 0., 7.5, CYAN);  
    sp2 = cil (21.5, 21.5, 0., 0., 41.5, BLUE); 
    sp3 = cil (21.5, 10., 0., 0., 47., BLUE); 
    sp4 = cil (21.5, 21.5, 0., 0., 60., GREEN); 
    makeobj( p = genobj () ); 
    pushmatrix(); 
    callobj(sp1); 
    translate(0., 0., 7.5); 
    callobj(sp2);
    translate(0., 0., 41.5); 
    callobj(sp3);  
    translate(0., 30., 47.); 
    rotate(900, 'x'); 
    callobj(sp4); 
    popmatrix(); 
    closeobj(); 
    return(p);
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makep7                                           
 ** PURPOSE          : This function produces that portion of the robot arm
 **                    which lies between the 3rd pitch joint and the 4th
 **                    roll joint.                                      
 ** RETURNS          : int which is iris index to the object.
 ** INPUTS           : none.                                            
 ** LOCAL VARIABLES  : p -- object to be returned.                      
 **                    sp1 -- arm segment between 3rd pitch joint and 4th
 **                           roll join.                                
 **                    sp2 -- 4th roll joint lower interface.           
 ** FUNCTIONS CALLED : cil                                              
 **                                                                    
 ************************************************************************/

long int makep7(void) {
    long int p, sp1, sp2;

    sp1 = cil (17.5, 21.5, 0., 0., 25., BLUE); 
    sp2 = cil (21.5, 21.5, 0., 0., 5., CYAN); 
    makeobj( p = genobj () ); 
    pushmatrix();
    callobj(sp1); 
    translate(0., 0., 25.); 
    callobj(sp2); 
    popmatrix(); 
    closeobj();  
    return(p);
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makef1                                           
 ** PURPOSE          : This function makes the large lower part of the  
 **                    finger.                                          
 ** RETURNS          : the object created, lower part of finger.        
 ** INPUTS           : none                                             
 ** LOCAL VARIABLES  : p -- part to be returned, lower part of finger.  
 **                    sp1 -- square part of the lower finger.          
 **                    sp2 -- round part of finger, point of rotation.  
 ** FUNCTIONS CALLED : box, cil                                         
 **                                                                     
 ************************************************************************/

long int makef1 (void) {
    long int p, sp1, sp2;

    sp1 = box (-8.75, 8.75, -8.75, 8.75, 0., 27.5, 0., 27.5, 0., 51.25, WHITE); 
    sp2 = cil (8.75, 8.75, 0., 0., 49.5, WHITE);  
    makeobj( p = genobj() );
    callobj(sp1); 
    callobj(sp2); 
    closeobj(); 
    return(p);  
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makef2                                           
 ** PURPOSE          : This function makes the midsection part of the 
 **                    finger.                                          
 ** RETURNS          : the object created, midsection of finger.        
 ** INPUTS           : none                                             
 ** LOCAL VARIABLES  : p -- part to be returned, midsection of finger.  
 ** FUNCTIONS CALLED : box                                              
 **                                                                     
 ************************************************************************/

long int makef2(void) {
    long int p;

    p = box (-3.75, 3.75, -3.75, 3.75,  
	     -5.625, 5.625, -5.625, 5.625, 0., 22.5, WHITE); 
    return (p);  
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makef3                                           
 ** PURPOSE          : This function makes the upper part of the 
 **                    finger.                                          
 ** RETURNS          : the object created, upper part of finger.        
 ** INPUTS           : none                                             
 ** LOCAL VARIABLES  : p -- part to be returned, upper part of finger.  
 ** FUNCTIONS CALLED : box                                              
 **                                                                     
 ************************************************************************/

long int makef3(void) {
    long int p;

    p = box (-3.75, 3.75, -3.75, 3.75, 
	     -5.625, 5.625, -5.625, -.625, 0., 27.5, WHITE);
    return (p);  
}


/************************************************************************
 **ok                                                                   
 ** FUNCTION NAME    : makeh1                                           
 ** PURPOSE          : This function creates the end effector section from
 **                    the torque force senser to the fingers.          
 ** RETURNS          : object, represents wrist portion of end effector.
 ** INPUTS           : none                                             
 ** LOCAL VARIABLES  : p -- object to be returned, portion of wrist     
 **                    sp1 -- force torque senser.                      
 **                    sp2 -- square wrist portion of the end effector. 
 ** FUNCTIONS CALLED : cil, box                                         
 **                                                                     
 ************************************************************************/

long int makeh1 (void) {
    long int p, sp1, sp2, finger;

    sp1 = cil (22.5, 22.5, 0., 0., 30., YELLOW);
    sp2 = box (-29.75, 15.25, -29.75, 15.25,
	       -26.25, 26.25, -26.25, 26.25, 31., 39.75, MAGENTA);  
    makeobj( p = genobj() );
    callobj (sp1); 		 
    callobj (sp2);
    closeobj (); 
    return (p); 
}



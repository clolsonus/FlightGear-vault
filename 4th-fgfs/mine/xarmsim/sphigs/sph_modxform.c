#include "HEADERS.h"
#include "sphigslocal.h"
      
/* A pointer to double is actually a pointer to
 *  to the first element of a 4by4 matrix.
 */

void SPH_scale (scaleX, scaleY, scaleZ,  result)
double scaleX, scaleY, scaleZ;
matrix result;
{
   MAT3hvec scalevec;
   
   scalevec[0] = scaleX;
   scalevec[1] = scaleY;
   scalevec[2] = scaleZ;
   MAT3scale (result, scalevec);
}


void SPH_translate (translateX, translateY, translateZ,  result)
double translateX, translateY, translateZ;
matrix result;
{
   MAT3hvec translatevec;
   
   translatevec[0] = translateX;
   translatevec[1] = translateY;
   translatevec[2] = translateZ;
   MAT3translate (result, translatevec);
}

static MAT3hvec rotaxisvec = {0.0, 0.0, 0.0};
static double degtoradfactor = 0.01745329252;
   
void SPH_rotateX (double angle,  matrix result)
{
   rotaxisvec[0] = 1;
   MAT3rotate (result, rotaxisvec, angle*degtoradfactor);
   rotaxisvec[0] = 0;
}

void SPH_rotateY (double angle,  matrix result)
{
   static MAT3hvec rotaxisvec = {0.0, 0.0, 0.0};
   
   rotaxisvec[1] = 1;
   MAT3rotate (result, rotaxisvec, angle*degtoradfactor);
   rotaxisvec[1] = 0;
}

void SPH_rotateZ (double angle,  matrix result)
{
   static MAT3hvec rotaxisvec = {0.0, 0.0, 0.0};
   
   rotaxisvec[2] = 1;
   MAT3rotate (result, rotaxisvec, angle*degtoradfactor);
   rotaxisvec[2] = 0;
}


void SPH_compose (matrix mat1, matrix mat2,  matrix result)
{
   MAT3mult (result, mat1, mat2);
}

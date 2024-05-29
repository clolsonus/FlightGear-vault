/* fggeom.c -- flight gear trig and vector math routines
 *
 * Copyright (C) 1996  Eric J. Korpela -- korpela@ssl.berkeley.edu
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * $Id: fggeom.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $ 
 */

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include "fgtypes.h"
#include "fgvars.h"
#include "fgdefs.h"
#include "fggeom.h"
#include "fgdriver.h"
#include "fgmain.h"

#ifdef USE_DLL101
#include "dll.h"
#endif

const FG_I2VECT x_axis_2d={FG_ONE,0};
const FG_I2VECT y_axis_2d={0,FG_ONE};
const FG_I2X2   i2x2_identity={{{1,0},
                                {0,1}}};
const FG_I2X2   i2x2_zero={{{0,0},
                            {0,0}}};


const FG_I3VECT x_axis_3d={FG_ONE,0,0};
const FG_I3VECT y_axis_3d={0,FG_ONE,0};
const FG_I3VECT z_axis_3d={0,0,FG_ONE};
const FG_I3X3   i3x3_identity={{{1,0,0},
                                {0,1,0},
                                {0,0,1}}};
const FG_I3X3   i3x3_zero={{{0,0,0},
                            {0,0,0},
                            {0,0,0}}};

const FG_I4VECT x_axis_4d={FG_ONE,0,0,0};
const FG_I4VECT y_axis_4d={0,FG_ONE,0,0};
const FG_I4VECT z_axis_4d={0,0,FG_ONE,0};
const FG_I4VECT t_axis_4d={0,0,0,FG_ONE};
const FG_I4X4   i4x4_identity={{{1,0,0,0},
                                {0,1,0,0},
                                {0,0,1,0},
                                {0,0,0,1}}};
const FG_I4X4   i4x4_zero={{{0,0,0,0},
                            {0,0,0,0},
                            {0,0,0,0},
                            {0,0,0,0}}};

inline FG_INT32 isin(FG_INT32 x)
{
/* returns a fixed point number between -1 and 1 (-FG_ONE and FG_ONE)       */
/* turn this into something faster later                                    */
return (FG_INT32)(sin(((FG_FLOAT32)x)/FG_RADIAN)*(FG_ONE));
}

inline FG_INT32 iasin(FG_INT32 x)
{
/* returns a fixed point number between -PI/2 and PI/2                      */
/* turn this into something faster later                                    */
return (FG_INT32)(asin(((FG_FLOAT32)x)/FG_ONE)*FG_RADIAN);
}

inline FG_INT32 icos(FG_INT32 x)
{
/* returns a fixed point number between -1 and 1 (-FG_ONE and FG_ONE)       */
/* turn this into something faster later                                    */
return (FG_INT32)(cos(((FG_FLOAT32)x)/FG_RADIAN)*(FG_ONE));
}

inline FG_INT32 isqrt(FG_INT64 x)
{
/* returns the square root of a fixed point number (1.0=FG_ONE)             */
/* turn this into something faster later                                    */
return (FG_INT32)(sqrt(((FG_FLOAT32)x)/FG_ONE)*FG_ONE);
}

inline FG_INT32 iatan2(FG_INT32 y, FG_INT32 x)
{
/* returns the arctangent (in fg angular coordinates) of y/x without 
   sign confusion                                                           */
return (FG_INT32)(atan2((FG_FLOAT32)y,(FG_FLOAT32)x)*FG_RADIAN);
}


inline FG_INT32 iangdist(FG_INT32 lat1, FG_INT32 lon1, FG_INT32 lat2,
								  FG_INT32 lon2)
{
  FG_INT64 dist=((FG_INT64)isin(lon1)*isin(lon2)/(FG_ONE)+
            (FG_INT64)icos(lon1)*icos(lon2)/(FG_ONE)*icos(lat1-lat2))/(FG_ONE);
  FG_INT32 d=isqrt((FG_ONE)-dist*dist/(FG_ONE));
  return (iatan2(d,dist));
}

FG_INT32 vangdist(FG_I2VECT *a,FG_I2VECT *b)
{
  return (iangdist(a->y,a->x,b->y,b->x));
}

inline FG_INT32 iarcdist(FG_INT32 alt1, FG_INT32 lat1, FG_INT32 lon1,
                  FG_INT32 alt2, FG_INT32 lat2, FG_INT32 lon2)
{
  FG_INT32 angdist=iangdist(lat1,lon1,lat2,lon2);
  fprintf(stdout,"angdist: %d\n",angdist/1000/FG_METER);
  return((FG_INT32)sqrt((FG_FLOAT32)(alt2-alt1)*(alt2-alt1)+(FG_FLOAT32)angdist*angdist));
}

FG_INT32 varcdist(FG_I3VECT *a,FG_I3VECT *b)
{
  return (iarcdist(a->z,a->y,a->x,b->z,b->y,b->x));
}

inline FG_INT32 ilindist(FG_INT32 alt1, FG_INT32 lat1, FG_INT32 lon1,
						FG_INT32 alt2, FG_INT32 lat2, FG_INT32 lon2)
{
/* need too much precision for integer trig functions here  */ 
  FG_INT32 r=alt1+FG_R_EARTH;
  FG_INT32 x=r*sin((FG_FLOAT32)lon1/FG_RADIAN)*cos((FG_FLOAT32)lat1/FG_RADIAN);
  FG_INT32 y=r*cos((FG_FLOAT32)lon1/FG_RADIAN)*cos((FG_FLOAT32)lat1/FG_RADIAN);
  FG_INT32 z=r*sin((FG_FLOAT32)lat1/FG_RADIAN);
  r=alt2+FG_R_EARTH;
  x-=r*sin((FG_FLOAT32)lon2/FG_RADIAN)*cos((FG_FLOAT32)lat2/FG_RADIAN);
  y-=r*cos((FG_FLOAT32)lon2/FG_RADIAN)*cos((FG_FLOAT32)lat2/FG_RADIAN);
  z-=r*sin((FG_FLOAT32)lat2/FG_RADIAN);
  return ((FG_INT32)sqrt((FG_INT64)x*x+(FG_INT64)y*y+(FG_INT64)z*z));
}

FG_INT32 vlindist(FG_I3VECT *a,FG_I3VECT *b)
{
  return (ilindist(a->z,a->y,a->x,b->z,b->y,b->x));
}


inline FG_INT64 i2dot(FG_I2VECT *a, FG_I2VECT *b)
{
  return ((FG_INT64)(a->x)*(b->x) +
          (FG_INT64)(a->y)*(b->y));
}

inline void i2vect_scale(FG_I2VECT *a, FG_INT32 scale, FG_I2VECT *result)
/* scales a vector by a fixed point scale 1.0=FG_ONE */
{
  result->x=a->x*scale/FG_ONE;
  result->y=a->y*scale/FG_ONE;
}

inline FG_INT64 i3dot(FG_I3VECT *a, FG_I3VECT *b)
{
  return ((FG_INT64)(a->x)*(b->x) + 
          (FG_INT64)(a->y)*(b->y) + 
          (FG_INT64)(a->z)*(b->z));
}

inline void i3vect_scale(FG_I3VECT *a, FG_INT32 scale, FG_I3VECT *result)
/* scales a vector by a fixed point scale 1.0=FG_ONE */
{
  result->x=a->x*scale/FG_ONE;
  result->y=a->y*scale/FG_ONE;
  result->z=a->z*scale/FG_ONE;
}

inline FG_INT64 i4dot(FG_I4VECT *a, FG_I4VECT *b)
{
  return ((FG_INT64)(a->x)*(b->x) + 
          (FG_INT64)(a->y)*(b->y) + 
          (FG_INT64)(a->z)*(b->z) +
          (FG_INT64)(a->t)*(b->t));
}

inline void i4vect_scale(FG_I4VECT *a, FG_INT32 scale, FG_I4VECT *result)
/* scales a vector by a fixed point scale 1.0=FG_ONE */
{
  result->x=a->x*scale/FG_ONE;
  result->y=a->y*scale/FG_ONE;
  result->z=a->z*scale/FG_ONE;
  result->t=a->t*scale/FG_ONE;
}

inline FG_INT32 i2mag(FG_I2VECT *a)
{
  return isqrt(i2dot(a,a)/FG_ONE);
}

inline FG_INT32 i3mag(FG_I3VECT *a)
{
  return isqrt(i3dot(a,a)/FG_ONE);
}

inline FG_INT32 i4mag(FG_I4VECT *a)
{
  return isqrt(i4dot(a,a)/FG_ONE);
}

inline void i2norm(FG_I2VECT *a)
{
  register FG_INT32 mag=i2mag(a);
  if (mag != FG_ONE) {
   a->x=(FG_INT64)(a->x)*FG_ONE/mag;
   a->y=(FG_INT64)(a->y)*FG_ONE/mag;
  }
}

inline void i3norm(FG_I3VECT *a)
{
  register FG_INT32 mag=i3mag(a);
  if (mag != FG_ONE) {
   a->x=(FG_INT64)(a->x)*FG_ONE/mag;
   a->y=(FG_INT64)(a->y)*FG_ONE/mag;
   a->z=(FG_INT64)(a->z)*FG_ONE/mag;
  }
}

inline void i4norm(FG_I4VECT *a)
{
  register FG_INT32 mag=i4mag(a);
  if (mag != FG_ONE) {
   a->x=(FG_INT64)(a->x)*FG_ONE/mag;
   a->y=(FG_INT64)(a->y)*FG_ONE/mag;
   a->z=(FG_INT64)(a->z)*FG_ONE/mag;
   a->t=(FG_INT64)(a->z)*FG_ONE/mag;
  }
}

inline void i3cross(FG_I3VECT *a, FG_I3VECT *b, FG_I3VECT *result)
/* result=a x b */
{
  FG_INT32 x,y,z;
  register FG_I3VECT *c=a,*d=b;
  
  x=((FG_INT64)(c->y)*d->z - (FG_INT64)(d->y)*c->z)/FG_ONE;
  y=((FG_INT64)(d->x)*c->z - (FG_INT64)(c->x)*d->z)/FG_ONE;
  z=((FG_INT64)(c->x)*d->y - (FG_INT64)(d->x)*c->y)/FG_ONE;

  result->x=x;
  result->y=y;
  result->z=z;
}

inline void i2x2copy(FG_I2X2 *source, FG_I2X2 *dest) 
{
  memmove(dest,source,sizeof(FG_I2X2));
}

inline void i2x2mul(FG_I2X2 *a, FG_I2X2 *b, FG_I2X2 *result)
/* multiply 2 2x2 matrices together, store in result */
{
   FG_I2X2 tmp;
   register FG_INT32 i,j;
   register FG_I2X2 *c=a,*d=b;
   for (i=0;i<2;i++) {
     for (j=0;j<2;j++) {
       tmp.mat[i][j]=c->mat[i][0]*d->mat[0][j]+
                     c->mat[i][1]*d->mat[1][j];
     }
   }
   i2x2copy(&tmp,result);
}

inline void i2x2scale(FG_I2X2 *a, FG_INT32 scale, FG_I2X2 *result)
/* multiply a 2X2 matric by a fixed point scalar 1.0=FG_ONE */
{
   register FG_INT32 i,j;
   for (i=0;i<2;i++) {
     for (j=0;j<2;j++) {
       result->mat[i][j]=a->mat[i][j]*scale/FG_ONE;
     }
   }
}

inline void i3x3copy(FG_I3X3 *source, FG_I3X3 *dest)
{
  memmove(dest,source,sizeof(FG_I3X3));
}


inline void i3x3mul(FG_I3X3 *a, FG_I3X3 *b, FG_I3X3 *result)
/* multiply 2 3x3 matrices together, store in result */
{
   FG_I3X3 tmp;
   register FG_INT32 i,j;
   register FG_I3X3 *c=a,*d=b;
   for (i=0;i<3;i++) {
     for (j=0;j<3;j++) {
       tmp.mat[i][j]=c->mat[i][0]*d->mat[0][j]+
                     c->mat[i][1]*d->mat[1][j]+
                     c->mat[i][2]*d->mat[2][j];
     }
   }
   i3x3copy(&tmp,result);
}

inline void i3x3scale(FG_I3X3 *a, FG_INT32 scale, FG_I3X3 *result)
/* multiply a 3X3 matric by a fixed point scalar 1.0=FG_ONE */
{
   register FG_INT32 i,j;
   for (i=0;i<3;i++) {
     for (j=0;j<3;j++) {
       result->mat[i][j]=a->mat[i][j]*scale/FG_ONE;
     }
   }
}

inline void i4x4copy(FG_I4X4 *source, FG_I4X4 *dest)
{
  memmove(dest,source,sizeof(FG_I4X4));
}

inline void i4x4mul(FG_I4X4 *a, FG_I4X4 *b, FG_I4X4 *result)
/* multiply 2 4x4 matrices together, store in result */
{
   FG_I4X4 tmp;
   register FG_INT32 i,j;
   register FG_I4X4 *c=a,*d=b;
   for (i=0;i<4;i++) {
     for (j=0;j<4;j++) {
       tmp.mat[i][j]=c->mat[i][0]*d->mat[0][j]+
                     c->mat[i][1]*d->mat[1][j]+
                     c->mat[i][2]*d->mat[2][j]+
                     c->mat[i][3]*d->mat[3][j];
     }
   }
   i4x4copy(&tmp,result);
}

inline void i4x4scale(FG_I4X4 *a, FG_INT32 scale, FG_I4X4 *result)
/* multiply a 4X4 matric by a fixed point scalar 1.0=FG_ONE */
{
   register FG_INT32 i,j;
   for (i=0;i<4;i++) {
     for (j=0;j<4;j++) {
       result->mat[i][j]=a->mat[i][j]*scale/FG_ONE;
     }
   }
}

inline void i2rot(FG_I2VECT *vect, FG_I2VECT *result, FG_INT32 theta)
/* rotates vector "vect" around the origin by theta units and stores
 * the result in vector "result"
 */
{
  FG_INT32 st=isin(theta);
  FG_INT32 ct=icos(theta);
  FG_INT32 vx=vect->x,vy=vect->y;

  result->x= vx*ct/FG_ONE - vy*st/FG_ONE;
  result->y= vx*st/FG_ONE + vy*ct/FG_ONE;
}



  
inline void i3rot(FG_I3VECT *vect, FG_I3VECT *axis, FG_I3VECT *result, 
                  FG_INT32 theta)
/* rotates vector "vect" around unit vector "axis" by theta units and stores 
 * the result in vector "result"
 */
{
  FG_INT32 st=isin(theta);
  FG_INT32 ct=icos(theta);
  FG_INT32 ct1=(FG_ONE)-ct;
  FG_INT32 vx=vect->x,vy=vect->y,vz=vect->z;
  FG_INT32 exp1,exp2,exp3,exp4,exp5,exp6;
  FG_INT32 xs1,xs2,xs3;
  FG_INT32 x=axis->x,y=axis->y,z=axis->z;

  exp1=x*y/FG_ONE*ct1/FG_ONE;
  exp2=z*st/FG_ONE;
  exp3=x*z/FG_ONE*ct1/FG_ONE;
  exp4=y*st/FG_ONE;
  exp5=y*z/FG_ONE*ct1/FG_ONE;
  exp6=x*st/FG_ONE;
  xs1=x*x/FG_ONE;
  xs2=y*y/FG_ONE;
  xs3=z*z/FG_ONE;
 
  result->x=((FG_ONE-xs1)*ct/FG_ONE +xs1)*vx/FG_ONE +
             (exp1-exp2)*vy/FG_ONE +
             (exp3+exp4)*vz/FG_ONE;
  result->y=(exp1+exp2)*vx/FG_ONE +
             ((FG_ONE-xs2)*ct/FG_ONE +xs2)*vy/FG_ONE +
             (exp5-exp6)*vz/FG_ONE;
  result->z=(exp3-exp4)*vx/FG_ONE +
             (exp5+exp6)*vy/FG_ONE +
             ((FG_ONE-xs3)*ct/FG_ONE +xs3)*vz/FG_ONE;
}

void fggeom_export() 
{
#ifdef USE_DLL101
static char *symbols[]={"_x_axis_2d", "_y_axis_2d", "_i2x2_identity", 
                      "_i2x2_zero", "_x_axis_3d", "_y_axis_3d", "_z_axis_3d", 
                      "_i3x3_identity", "_i3x3_zero", "_x_axis_4d", 
                      "_y_axis_4d", "_z_axis_4d", "_t_axis_4d", 
                      "_i4x4_identity", "_i4x4_zero", "_isin",
                      "_icos", "_isqrt", "_iatan2", "_iangdist", "_i2dot",
                      "_i2vect_scale", "_i3dot", "_i3vect_scale", "_i4dot",
                      "_i4vect_scale", "_i2mag", "_i3mag","_i4mag", "_i2norm", 
                      "_i3norm", "_i4norm", "_i3cross", "_i2x2copy", 
                      "_i2x2mul", "_i2x2scale", "_i3x3copy", "_i3x3mul", 
                      "_i3x3scale", "_i4x4copy", "_i4x4mul", "_i4x4scale", 
                      "_i2rot", "_i3rot", NULL };

void *pointers[]={ &x_axis_2d, &y_axis_2d, &i2x2_identity, &i2x2_zero,
                   &x_axis_3d, &y_axis_3d, &z_axis_3d, &i3x3_identity,
                   &i3x3_zero, &x_axis_4d, &y_axis_4d, &z_axis_4d,
                   &t_axis_4d, &i4x4_identity, &i4x4_zero, isin, icos,
                   isqrt, iatan2, iangdist, i2dot, i2vect_scale, i3dot,
                   i3vect_scale, i4dot, i4vect_scale, i2mag, i3mag, i4mag,
                   i2norm, i3norm, i4norm, i3cross, i2x2copy, i2x2mul, 
                   i2x2scale, i3x3copy, i3x3mul, i3x3scale, i4x4copy, i4x4mul, 
                   i4x4scale, i2rot, i3rot, NULL };

  FG_INT32 i=0;

  while (symbols[i]) {
    DLL_AddSymbol(symbols[i],pointers[i]);
    i++;
  }
#endif
  return;
}



/* $Log: fggeom.c,v $
 * Revision 2.2  1996/11/24  23:47:15  korpela
 * Added WIN16 support.  Changed public_vars to a struct.
 * Many minor changes.
 *
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.7  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.6  1996/05/17  17:14:28  korpela
 * Added many matrix functions and macros.
 * Added fggeom_export() for DJGPP DLL support.
 *
 * Revision 1.5  1996/05/06  20:11:53  korpela
 * added i2rot() and i3rot().
 *
 * Revision 1.4  1996/05/05  22:01:26  korpela
 * minor bug fixes
 * */

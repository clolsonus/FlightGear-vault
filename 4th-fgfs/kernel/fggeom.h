/* fggeom.h -- flight gear trig and vector functions 
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
 * $Id: fggeom.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#ifndef _FGGEOM_H
#define _FGGEOM_H

extern const FG_I2VECT x_axis_2d,y_axis_2d;
extern const FG_I3VECT x_axis_3d,y_axis_3d,z_axis_3d;
extern const FG_I4VECT x_axis_4d,y_axis_4d,z_axis_4d,t_axis_4d;

extern const FG_I2X2 i2x2_identity,i2x2_zero;
extern const FG_I3X3 i3x3_identity,i3x3_zero;
extern const FG_I4X4 i4x4_identity,i4x4_zero;

#define eq2d(a,b) (((a).x == (b).x) && ((a).y == (b).y))
#define eq3d(a,b) (((a).x == (b).x) && ((a).y == (b).y) && ((a).z == (b).z))
#define eq4d(a,b) (((a).x == (b).x) && ((a).y == (b).y) && \
                   ((a).z == (b).z) && ((a).t == (b).t))

#define add2d(a,b,result)        \
{                                \
  (result).x = (a).x + (b).x;    \
  (result).y = (a).y + (b).y;    \
}
#define sub2d(a,b,result)        \
{                                \
  (result).x = (a).x - (b).x;    \
  (result).y = (a).y - (b).y;    \
}

#define add3d(a,b,result)        \
{                                \
  (result).x = (a).x + (b).x;    \
  (result).y = (a).y + (b).y;    \
  (result).z = (a).z + (b).z;    \
}
#define sub3d(a,b,result)        \
{                                \
  (result).x = (a).x - (b).x;    \
  (result).y = (a).y - (b).y;    \
  (result).z = (a).z - (b).z;    \
}

#define add4d(a,b,result)        \
{                                \
  (result).x = (a).x + (b).x;    \
  (result).y = (a).y + (b).y;    \
  (result).z = (a).z + (b).z;    \
  (result).t = (a).t + (b).t;    \
}
#define sub4d(a,b,result)        \
{                                \
  (result).x = (a).x - (b).x;    \
  (result).y = (a).y - (b).y;    \
  (result).z = (a).z - (b).z;    \
  (result).t = (a).t - (b).t;    \
}

#define mulc2d(a,cnst,result)    \
{                                \
  (result).x = (a).x * (cnst);   \
  (result).y = (a).y * (cnst);   \
}
#define mulc2x2(a,cnst,result)         \
{                                      \
  mulc2d((a).vect.x,(cnst),(result));  \
  mulc2d((a).vect.y,(cnst),(result));  \
}

#define mulc3d(a,cnst,result)    \
{                                \
  (result).x = (a).x * (cnst);   \
  (result).y = (a).y * (cnst);   \
  (result).z = (a).z * (cnst);   \
}
#define mulc3x3(a,cnst,result)         \
{                                      \
  mulc3d((a).vect.x,(cnst),(result));  \
  mulc3d((a).vect.y,(cnst),(result));  \
  mulc3d((a).vect.z,(cnst),(result));  \
}

#define mulc4d(a,cnst,result)    \
{                                \
  (result).x = (a).x * (cnst);   \
  (result).y = (a).y * (cnst);   \
  (result).z = (a).z * (cnst);   \
  (result).t = (a).t * (cnst);   \
}
#define mulc4x4(a,cnst,result)         \
{                                      \
  mulc4d((a).vect.x,(cnst),(result));  \
  mulc4d((a).vect.y,(cnst),(result));  \
  mulc4d((a).vect.z,(cnst),(result));  \
  mulc4d((a).vect.t,(cnst),(result));  \
}

#define add2x2(a,b,result)                        \
{                                                 \
  add2d((a).vect.x,(b).vect.x,(result).vect.x);   \
  add2d((a).vect.y,(b).vect.y,(result).vect.y);   \
}
#define sub2x2(a,b,result)                        \
{                                                 \
  sub2d((a).vect.x,(b).vect.x,(result).vect.x);   \
  sub2d((a).vect.y,(b).vect.y,(result).vect.y);   \
}

#define add3x3(a,b,result)                        \
{                                                 \
  add3d((a).vect.x,(b).vect.x,(result).vect.x);   \
  add3d((a).vect.y,(b).vect.y,(result).vect.y);   \
  add3d((a).vect.z,(b).vect.z,(result).vect.z);   \
}
#define sub3x3(a,b,result)                        \
{                                                 \
  sub3d((a).vect.x,(b).vect.x,(result).vect.x);   \
  sub3d((a).vect.y,(b).vect.y,(result).vect.y);   \
  sub3d((a).vect.z,(b).vect.z,(result).vect.z);   \
}

#define add4x4(a,b,result)                        \
{                                                 \
  add4d((a).vect.x,(b).vect.x,(result).vect.x);   \
  add4d((a).vect.y,(b).vect.y,(result).vect.y);   \
  add4d((a).vect.z,(b).vect.z,(result).vect.z);   \
  add4d((a).vect.t,(b).vect.t,(result).vect.t);   \
}
#define sub4x4(a,b,result)                        \
{                                                 \
  sub4d((a).vect.x,(b).vect.x,(result).vect.x);   \
  sub4d((a).vect.y,(b).vect.y,(result).vect.y);   \
  sub4d((a).vect.z,(b).vect.z,(result).vect.z);   \
  sub4d((a).vect.t,(b).vect.t,(result).vect.t);   \
}

#define det2x2(a) ((a).mat[0][0]*(a).mat[1][1] - (a).mat[0][1]*(a).mat[1][0])

inline FG_INT32 isin(FG_INT32 x);
/* returns a fixed point number between -1 and 1 (-FG_ONE and FG_ONE)       */

inline FG_INT32 iasin(FG_INT32 x);
/* returns a fixed point number between FG_PI/2 and -FG_PI/2                */

inline FG_INT32 icos(FG_INT32 x);
/* returns a fixed point number between -1 and 1 (-FG_ONE and FG_ONE)       */

inline FG_INT32 isqrt(FG_INT64 x);
/* returns the square root of a fixed point number (1.0=FG_ONE)             */

inline FG_INT32 iatan2(FG_INT32 y, FG_INT32 x);
/* returns the arctangent (in fg angular coordinates) of y/x without 
   sign confusion                                                           */

FG_INT32 iangdist(FG_INT32 lat1, FG_INT32 lon1, FG_INT32 lat2, FG_INT32 lon2);
/* returns the angular distance between two lat,lon points                  */

inline FG_INT64 i2dot(FG_I2VECT *a, FG_I2VECT *b);
/* returns the dot product of two integer 2-vectors (no normalization)      */
/* divide this by FG_ONE if vectors are fixed point                         */

inline void i2vect_scale(FG_I2VECT *a, FG_INT32 scale, FG_I2VECT *result);
/* scales a vector by a fixed point scale (1.0=FG_ONE)                      */

inline FG_INT64 i3dot(FG_I3VECT *a, FG_I3VECT *b);
/* returns the dot product of two integer 3-vectors (no normalization)      */
/* divide this by FG_ONE if vectors are fixed point                         */

inline void i3vect_scale(FG_I3VECT *a, FG_INT32 scale, FG_I3VECT *result);
/* scales a vector by a fixed point scale (1.0=FG_ONE)                      */

inline FG_INT64 i4dot(FG_I4VECT *a, FG_I4VECT *b);
/* returns the dot product of two integer 4-vectors (no normalization)      */
/* divide this by FG_ONE if vectors are fixed point                         */

inline void i4vect_scale(FG_I4VECT *a, FG_INT32 scale, FG_I4VECT *result);
/* scales a vector by a fixed point scale (1.0=FG_ONE)                      */

inline FG_INT32 i2mag(FG_I2VECT *a);
/* returns the magnitude of a 2-vector                                      */

inline FG_INT32 i3mag(FG_I3VECT *a);
/* returns the magnitude of a 3-vector                                      */

inline FG_INT32 i4mag(FG_I4VECT *a);
/* returns the magnitude of a 4-vector                                      */

inline void i2norm(FG_I2VECT *a);
/* normalizes a 2-vector                                                    */

inline void i3norm(FG_I3VECT *a);
/* normalizes a 3-vector                                                    */

inline void i4norm(FG_I4VECT *a);
/* normalizes a 4-vector                                                    */

inline void i3cross(FG_I3VECT *a, FG_I3VECT *b, FG_I3VECT *result);
/* 3-vector cross product (result=a x b)                                    */
/* will work for (a=result) or (b=result)                                   */

inline void i2x2copy(FG_I2X2 *source, FG_I2X2 *dest);
/* copys a integer 2x2 matrix                                               */

inline void i2x2mul(FG_I2X2 *a, FG_I2X2 *b, FG_I2X2 *result);
/* multiply 2 2x2 matrices together, store in result                        */
/* will work for (a=result) or (b=result)                                   */

inline void i2x2scale(FG_I2X2 *a, FG_INT32 scale, FG_I2X2 *result);
/* multiply a 2X2 matrix by a fixed point scalar (1.0=FG_ONE)               */
/* will work for (a=result) or (b=result)                                   */

inline void i3x3copy(FG_I3X3 *source, FG_I3X3 *dest);
/* copys a integer 3x3 matrix                                               */

inline void i3x3mul(FG_I3X3 *a, FG_I3X3 *b, FG_I3X3 *result);
/* multiply 2 3x3 matrices together, store in result                        */
/* will work for (a=result) or (b=result)                                   */

inline void i3x3scale(FG_I3X3 *a, FG_INT32 scale, FG_I3X3 *result);
/* multiply a 3X3 matrix by a fixed point scalar (1.0=FG_ONE)               */
/* will work for (a=result)                                                 */

inline void i4x4copy(FG_I4X4 *source, FG_I4X4 *dest);
/* copys a integer 3x3 matrix                                               */

inline void i4x4mul(FG_I4X4 *a, FG_I4X4 *b, FG_I4X4 *result);
/* multiply 2 4x4 matrices together, store in result                        */
/* will work for (a=result) or (b=result)                                   */

inline void i4x4scale(FG_I4X4 *a, FG_INT32 scale, FG_I4X4 *result);
/* multiply a 4X4 matrix by a fixed point scalar (1.0=FG_ONE)               */
/* will work for (a=result)                                                 */

inline void i2rot(FG_I2VECT *vect, FG_I2VECT *result, FG_INT32 theta);
/* rotates vector "vect" around the origin by theta units and stores
 * the result in vector "result"
 */
/* will work for (vect=result)                                              */
  
inline void i3rot(FG_I3VECT *vect, FG_I3VECT *axis, FG_I3VECT *result, 
                  FG_INT32 theta);
/* rotates vector "vect" around unit vector "axis" by theta units and stores 
 * the result in vector "result"
 */
/* will work for (vect=result) or (axis=result)                             */

void fggeom_export(void);
/* exports all these functions                                              */
#endif

/* $Log: fggeom.h,v $
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
 * Revision 1.6  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.5  1996/05/17  17:14:28  korpela
 * Added many matrix functions and macros.
 * Added fggeom_export() for DJGPP DLL support.
 *
 * Revision 1.4  1996/05/06  20:11:53  korpela
 * added i2rot() and i3rot().
 *
 * Revision 1.3  1996/05/05  22:01:26  korpela
 * minor changes
 * */

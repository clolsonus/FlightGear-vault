/* fgtypes.h -- flight gear type definitions 
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
 * $Id: fgtypes.h,v 2.1 1996/10/31 18:20:21 korpela Exp korpela $
 */

#ifndef _FGTYPES_H

#define _FGTYPES_H

#if defined(__sparc__) || defined(__mc68000__)
#define BIGENDIAN
#endif

typedef unsigned int FG_UINT;
typedef void FG_VOID;
typedef int FG_INT;
typedef unsigned char FG_CHAR;

#if defined(__GNUC__) && !defined(__alpha__)

typedef double FG_FLOAT64;
typedef float FG_FLOAT32;

typedef long long FG_INT64;
typedef int FG_INT32;
typedef short FG_INT16;
typedef char FG_INT8;

typedef unsigned long long FG_UINT64;
typedef unsigned int FG_UINT32;
typedef unsigned short FG_UINT16;
typedef unsigned char FG_UINT8;

#define FG_INT32_MAX LONG_MAX
#define FG_INT32_MIN LONG_MIN

#elif !defined(__GNUC__)

typedef double FG_FLOAT64;
typedef float FG_FLOAT32;

typedef long FG_INT32;
typedef short FG_INT16;
typedef char FG_INT8;
typedef union {
        FG_UINT32 l[2];
        FG_UINT16 s[4];
        FG_UINT8  c[8];
} FG_INT64;

typedef unsigned int FG_UINT32;
typedef unsigned short FG_UINT16;
typedef unsigned char FG_UINT8;
typedef union {
        FG_UINT32 l[2];
        FG_UINT16 s[4];
        FG_UINT8  c[8];
} FG_UINT64;

#define FG_INT32_MAX LONG_MAX
#define FG_INT32_MIN LONG_MIN

#endif

typedef int FG_SEM;

/* A bitmap type that won't conflict with every other bitmap type */

typedef struct _FG_BITMAP {
   FG_UINT struct_size;            /* set to sizeof(FG_BITMAP) for          */
                                   /* version identification                */
   FG_INT w,h;                     /* width and height in pixels            */
   FG_INT clip, cl, cr, ct, cb;    /* clipping flag and rectangle           */
                                   /* these should default to:              */
                                   /*  {TRUE,0,w-1,0,h-1}                   */
   FG_INT type;                    /* Type of bitmap                        */
                                   /*  FG_NORMALBITMAP                      */
                                   /*  FG_SUBBITMAP                         */

                                   /* BITMAPS ARE TOP DOWN !!!!!            */
   FG_UINT8  **lines;              /* The actual bitmap...                  */
                                   /* Pixels  are addressed as:             */
                                   /*  bmp.Lines[y][x]                      */
   FG_SEM  *sem;                   /* Access MUTEX semaphore                */
} FG_BITMAP;

#define FG_NORMALBITMAP 0
#define FG_SUBBITMAP 1

typedef struct _FG_HLINE {
   FG_INT start;
   FG_INT end;
} FG_HLINE;

typedef struct _FG_RASTER_SET {    /* a set of adjacent horizontal lines    */
   FG_INT num_lines;               /* number of lines in the set            */
   FG_INT color;                   /* color of lines                        */
   FG_HLINE *hlines;               /* a list of horizontal lines            */
} FG_RASTER_SET;

typedef struct _FG_RECT {
   FG_INT left,top,right,bottom;
} FG_RECT;

typedef struct _FG_I2VECT {
  FG_INT32 x,y;
} FG_I2VECT;

typedef union _FG_I2X2 {
  FG_INT32 mat[2][2];
  struct {
    FG_I2VECT x,y;
  } vect;
} FG_I2X2;

typedef struct _FG_I3VECT {
  FG_INT32 x,y,z;
} FG_I3VECT;

typedef union _FG_I3X3 {
  FG_INT32 mat[3][3];
  struct {
    FG_I3VECT x,y,z;
  } vect;
} FG_I3X3;

typedef struct _FG_I4VECT {
  FG_INT32 x,y,z,t;
} FG_I4VECT;

typedef union _FG_I4X4 {
  FG_INT32 mat[4][4];
  struct {
    FG_I4VECT x,y,z,t;
  } vect;
} FG_I4X4;

typedef struct _FG_POLYGON {
   FG_INT num_vertices;
   FG_I2VECT *vertices; 
} FG_POLYGON;


#if defined(BIGENDIAN)

#define FG_LOW32(x)  *(((FG_UINT32 *)(&x))+1)
#define FG_LOW16(x)  *(((FG_UINT16 *)(&x))+1)
#define FG_LOW8(x)   *(((FG_UINT8  *)(&x))+1)

#define FG_HIGH32(x) *(FG_UINT32 *)(&x)
#define FG_HIGH16(x) *(FG_UINT16 *)(&x)
#define FG_HIGH8(x)  *(FG_UINT8 *)(&x)

#else

#define FG_HIGH32(x)  *(((FG_UINT32 *)(&x))+1)
#define FG_HIGH16(x)  *(((FG_UINT16 *)(&x))+1)
#define FG_HIGH8(x)   *(((FG_UINT8  *)(&x))+1)

#define FG_LOW32(x) *(FG_UINT32 *)(&x)
#define FG_LOW16(x) *(FG_UINT16 *)(&x)
#define FG_LOW8(x)  *(FG_UINT8 *)(&x)

#endif

#endif

/* $Log: fgtypes.h,v $
 * Revision 2.1  1996/10/31  18:20:21  korpela
 * Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.4  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.3  1996/05/05  22:01:26  korpela
 * minor changes.
 * */

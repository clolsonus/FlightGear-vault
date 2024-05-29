/*                                      
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
 * $Id: isqrt.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "fgtypes.h"
#include "fgdefs.h"

inline FG_INT32 fp_isqrt(FG_INT64 x)
{
  if (x>0)
    return((FG_INT32)(sqrt((double)x/FG_ONE)*FG_ONE));
  else
    return(0);
}


inline FG_INT32 i_isqrt(FG_INT64 x)
{
  FG_INT32 i;
  FG_INT64 y=0;
  FG_INT32 base = INT_MAX/2+1;

  for (i = 1; i <= 32; i++)
  {
    if (((y + base) * (y + base)) <= x  )
      y += base;
    base >>= 1; 
  } 
  y=(y*FG_SQRT_ONE)/FG_ONE;
  return((FG_INT32)FG_MIN(y,INT_MAX));
} 


inline FG_INT32 n_isqrt(FG_INT64 x)
{
  FG_INT64 y=1,y_old=INT_MAX;   
  int i=0;
  
  if (x>0) {
    do {
      i++;
      y_old=y;
      y=(y+x/y)/2;
    } while ((y_old != y) && (i<37));
  } else y=0;  
  return ((FG_INT32)FG_MIN((y*FG_SQRT_ONE)/FG_ONE,INT_MAX));
}      


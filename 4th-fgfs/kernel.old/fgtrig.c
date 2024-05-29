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
 * $Id: fgtrig.c,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $
 */
/* A poor attempt at faster integer trig functions */

/* $Id: fgtrig.c,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $ */

static FG_INT32 fg_factorial[10]={1,1,2,6,24,120,720,5040,40320,362880};
static FG_INT32 fg_mone_to_n[10]={1,-1,1,-1,1,-1,1,-1,1,-1};

inline FG_INT32 fg_apow(FG_INT32 x, FG_INT32 p) 
{
  FG_INT32 i
  FG_INT32 xn,prod=SHRT_MAX;

  x=(x/(FG_RADIAN/SHRT_MAX));

  for(i=0;i<p;i++) {
   prod=prod*x/SHRT_MAX;
  }
  return (prod);
}
  

inline FG_INT32 isin(FG_INT32 x)
{
  FG_INT32 term,sum=0;
  FG_INT32 i2p1,sign,i=0;
 
  if (x<0) {
    sign=-1;
    x=-1*x;
  }

  if (x>(INT_MAX>>1)) {
    x=(INT_MAX>>1)-x;
  }

  do {
   i2p1=2*i+1;
   sum+=(term=fg_mone_to_n[i]*fg_apow(x,i2p1)/fg_factorial[i2p1]);
   i++;
  } while ((i<5) && term);
  sum*=sign;
  return (sum);
}
  
inline FG_INT32 icos(FG_INT32 x)
{
  FG_INT32 term,sum=0;
  FG_INT32 i2,sign,i=0;
 
  x=FG_ABS(x);

  if (x>(INT_MAX>>1)) {
    x=(INT_MAX>>1)-x;
    sign=-1;
  }

  do {
   i2=2*i;
   sum+=(term=fg_mone_to_n[i]*fg_apow(x,i2)/fg_factorial[i2]);
   i++;
  } while ((i<5) && term);
  sum*=sign;
  return (sum);
}

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
 * $Id: fgfixed.h,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $
 */
#define FP_OVF(c) ((FG_INT32)(((c)>INT_MAX) ?       \
                                 (c)-UINT_MAX-1 :   \
                                 (((c)<INT_MIN) ?   \
                                   (c)+UINT_MAX+1 : \
                                   (c))))

#ifdef _USE_FP_ADD_
  #define F_ADD(x,y) FP_OVF((double)(x)+(double)(y))
#else
  #define F_ADD(x,y) ((x)+(y))
#endif

#ifdef _USE_FP_SUB_
  #define F_SUB(x,y)  FP_OVF((double)(x)-(double)(y))
#else
  #define F_SUB(x,y) ((x)-(y))
#endif

#ifdef _USE_FP_MUL
  #define F_MUL(x,y)  (FG_INT64)((double)(x)*(double)(y)/FG_ONE)
#elif  defined(_USE_LONG_MUL)
  #define F_MUL(x,y)  ((FG_INT64)(x)*(y)/FG_ONE)
#else
inline static FG_INT64 F_MUL(FG_INT32 a, FG_INT32 b)
{
  register FG_INT32 xl,xh,yl,yh;
  xl=(a % FG_ONE);
  xh=(a/FG_ONE);
  yl=(b % FG_ONE);
  yh=(b/FG_ONE);
  return ((FG_INT64)(xh*yh)*FG_ONE+(FG_INT32)xh*yl+(FG_INT32)yh*xl
           +(FG_INT32)yl*xl/FG_ONE);


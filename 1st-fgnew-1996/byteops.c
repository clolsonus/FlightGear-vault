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
 * $Id: byteops.c,v 2.1 1996/10/31 18:05:22 korpela Exp korpela $
 */

#include <byteops.h>

inline BYTES4 addBYTES4(BYTES4 a, BYTES4 b)
{
  return (BYTES4)((((a.i & 0xff00ff00)+(b.i & 0xff00ff00)) & 0xff00ff00) +
          (((a.i & 0x00ff00ff)+(b.i & 0x00ff00ff)) & 0x00ff00ff));
}

inline BYTES4 subBYTES4(BYTES4 a, BYTES4 b)
{
  return (BYTES4)((((a.i & 0xff00ff00)-(b.i & 0xff00ff00)) & 0xff00ff00) +
          (((a.i & 0x00ff00ff)-(b.i & 0x00ff00ff)) & 0x00ff00ff));
}

inline BYTES4 mulfracBYTES4(BYTES4 a, BYTES4 b)
{
  register unsigned i;
  register BYTES4 result;
  for (i=0;i<4;i++) {
    result.b[i]=((unsigned int)(a.b[i])*((unsigned int)(b.b[i])+1))>>8;
  }
  return (result);
}

inline BYTES4 addmaxBYTES4(BYTES4 a, BYTES4 b)
{
  register unsigned top=((a.i & 0xff000000)>>8)+((b.i & 0xff000000)>>8);
  register unsigned odd=(a.i & 0x0000ff00)+(b.i & 0x0000ff00);
  register unsigned even=(a.i & 0x00ff00ff)+(b.i & 0x00ff00ff);
  top=((top > 0x00ff0000)*0xff000000 ) | (top<<8);
  odd|=((odd > 0x0000ff00)*0x0000ff00);
  even|=(((even > 0x00ff0000)*0x00ff0000)+(((even & 0x0000ff00)>0)*0x000000ff));
  return (BYTES4)(top | (odd & 0x0000ff00) | (even & 0x00ff00ff));
}

inline BYTES4 subminBYTES4(BYTES4 a, BYTES4 b)
{
  register unsigned top=((a.i & 0xff000000)>>8)-((b.i & 0xff000000)>>8);
  register unsigned odd=(a.i & 0x0000ff00)-(b.i & 0x0000ff00);
  register unsigned even=(0x01000100 | (a.i & 0x00ff00ff))-(b.i & 0x00ff00ff);
  register unsigned test=((even & 0x01000100)*0xff)>>8;
  top=((top < 0x00ff0000)*(top<<8));
  odd&=((odd < 0x0000ff00)*0x0000ff00);
  even&=test;
  return (BYTES4)(top | odd | even ); 
} 


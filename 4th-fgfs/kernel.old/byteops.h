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
 * $Id: byteops.h,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $
 */
typedef union _BYTES4 {
  unsigned int i;
  unsigned char b[4];
} BYTES4;

#define andBYTES4(a,b) (BYTES4)(a.i & b.i)
#define orBYTES4(a,b) (BYTES4)(a.i | b.i)
#define xorBYTES4(a,b) (BYTES4)(a.i ^ b.i)
#define notBYTES4(a) (BYTES4)(~a.i)

inline BYTES4 addBYTES4(BYTES4 a, BYTES4 b);
inline BYTES4 subBYTES4(BYTES4 a, BYTES4 b);
inline BYTES4 mulfracBYTES4(BYTES4 a, BYTES4 b);
inline BYTES4 addmaxBYTES4(BYTES4 a, BYTES4 b);
inline BYTES4 subminBYTES4(BYTES4 a, BYTES4 b);

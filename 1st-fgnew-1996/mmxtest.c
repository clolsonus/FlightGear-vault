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
 * $Id$
 */

#include "mmx.h"

MMXR *a,*b,*c;
FG_INT8 oneb[]={1,1,1,1,1,1,1,1};
FG_INT16 onew[]={1,1,1,1,1,1,1,1};
FG_INT32 oned[]={1,1,1,1,1,1,1,1};
FG_INT64 oneq[]={1,1,1,1,1,1,1,1};
FG_INT8 t0[]={1,-2,3,-4,SCHAR_MAX,SCHAR_MIN,SCHAR_MAX,SCHAR_MIN};
FG_UINT8 ut0[]={1,2,3,4,UCHAR_MAX,UCHAR_MAX,UCHAR_MAX,UCHAR_MAX};
FG_INT16 t1[]={1,-2,3,-4,SCHAR_MAX+1,SCHAR_MIN-1,SCHAR_MAX+2,SCHAR_MIN-2};
FG_UINT16 ut1[]={1,2,3,4,UCHAR_MAX+1,UCHAR_MAX+2,UCHAR_MAX+3,UCHAR_MAX+4};
FG_INT32 t2[]={1,-2,3,-4,SHRT_MAX+1,SHRT_MIN-1,SHRT_MAX+2,SHRT_MIN-2};
FG_UINT32 ut2[]={1,2,3,4,USHRT_MAX+1,USHRT_MAX+2,USHRT_MAX+3,USHRT_MAX+4};
FG_INT64 t3[]={1,-2,3,-4,INT_MAX+1,INT_MIN-1,INT_MAX+2,INT_MIN-2};
FG_UINT64 ut3[]={1,2,3,4,UINT_MAX+1,UINT_MAX+2,UINT_MAX+3,UINT_MAX+4};

int main()
{
  int i;
  a=(MMXR *)malloc(65536);
  b=(MMXR *)malloc(65536);
  c=(MMXR *)malloc(65536);

  memcpy(a,t1,sizeof(t1));
  packsswb((FG_INT16 *)a,(FG_INT8 *)b,8);
  printf("packsswb  ( 1 -2 3 -4 MAX MIN MAX MIN) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].sb[i]);
  printf(")\n");
  memcpy(a,ut1,sizeof(ut1));
  packuswb((FG_UINT16 *)a,(FG_UINT8 *)b,8);
  printf("packuswb  ( 1 2 3 4 MAX MAX MAX MAX) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].ub[i]);
  printf(")\n");

  memcpy(a,t2,sizeof(t2));
  packssdw((FG_INT32 *)a,(FG_INT16 *)b,8);
  printf("packssdw  ( 1 -2 3 -4 MAX MIN MAX MIN) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[i>>2].sw[i%4]);
  printf(")\n");
  memcpy(a,ut2,sizeof(ut2));
  packusdw((FG_UINT32 *)a,(FG_UINT16 *)b,8);
  printf("packusdw  ( 1 2 3 4 MAX MAX MAX MAX) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[i>>2].uw[i%4]);
  printf(")\n");

  memcpy(a,ut0,sizeof(ut0));
  paddb((FG_UINT8 *)a,oneb,(FG_UINT8 *)b,8);
  printf("paddb  ( 2 3 4 5 0 0 0 0) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].ub[i]);
  printf(")\n");
  memcpy(a,ut1,sizeof(ut1));
  paddw((FG_UINT16 *)a,onew,(FG_UINT16 *)b,8);
  printf("paddw  ( 2 3 4 5 257 258 259 260) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[i>>2].uw[i%4]);
  printf(")\n");
  memcpy(a,ut2,sizeof(ut2));
  paddd((FG_UINT32 *)a,oned,(FG_UINT32 *)b,8);
  printf("paddd  ( 2 3 4 5 65537 65538 65539 65540) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[i>>1].udw[i%2]);
  printf(")\n");

  memcpy(a,t0,sizeof(t0));
  paddsb((FG_INT8 *)a,oneb,(FG_INT8 *)b,8);
  printf("paddsb  ( 2 -1 4 -3 MAX MIN+1 MAX MIN+1) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].sb[i]);
  printf(")\n");
  memcpy(a,t1,sizeof(t1));
  paddsw((FG_INT16 *)a,onew,(FG_INT16 *)b,8);
  printf("paddsw  ( 2 -1 4 -3 129 -128 130 -129) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[i>>2].sw[i%4]);
  printf(")\n");

  memcpy(a,ut0,sizeof(ut0));
  paddusb((FG_UINT8 *)a,oneb,(FG_UINT8 *)b,8);
  printf("paddsb  ( 2 3 4 5 MAX MAX MAX MAX) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].ub[i]);
  printf(")\n");
  memcpy(a,ut1,sizeof(ut1));
  paddsw((FG_UINT16 *)a,onew,(FG_UINT16 *)b,8);
  printf("paddsw  ( 2 3 4 5 257 258 259 260) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[i>>2].uw[i%4]);
  printf(")\n");

  memcpy(a,ut0,sizeof(ut0));
  pand(a,oneb,b,sizeof(ut0));
  printf("pand  ( 1 0 1 0 1 1 1 1 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].ub[i]);
  printf(")\n");
 
  memcpy(a,ut0,sizeof(ut0));
  pandn(a,oneb,b,sizeof(ut0));
  printf("pandn  ( 0 1 0 1 0 0 0 0 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].ub[i]);
  printf(")\n");

  memcpy(a,ut0,sizeof(ut0));
  pcmpeqb(a,oneb,b,sizeof(ut0));
  printf("pcmpeqb  ( MAX 0 0 0 0 0 0 0 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].ub[i]);
  printf(")\n");
  memcpy(a,ut1,sizeof(ut1));
  pcmpeqw(a,onew,b,8);
  printf("pcmpeqw  ( MAX 0 0 0 0 0 0 0 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[i>>2].uw[i%4]);
  printf(")\n");
  memcpy(a,ut2,sizeof(ut2));
  pcmpeqd(a,oned,b,8);
  printf("pcmpeqd  ( MAX 0 0 0 0 0 0 0 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%u ",b[i>>1].udw[i%2]);
  printf(")\n");

  memcpy(a,t0,sizeof(t0));
  pcmpgtb(a,oneb,b,sizeof(t0));
  printf("pcmpgtb  ( 0 0 MAX 0 MAX 0 MAX 0 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].ub[i]);
  printf(")\n");
  memcpy(a,t1,sizeof(t1));
  pcmpgtw(a,onew,b,8);
  printf("pcmpgtw  ( 0 0 MAX 0 MAX 0 MAX 0 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[i>>2].uw[i%4]);
  printf(")\n");
  memcpy(a,t2,sizeof(t2));
  pcmpgtd(a,oned,b,8);
  printf("pcmpgtd  ( 0 0 MAX 0 MAX 0 MAX 0 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%u ",b[i>>1].udw[i%2]);
  printf(")\n");
  
  memcpy(a,t1,sizeof(t1));
  pmaddwd(a,a,b,8);
  printf("pmaddwd ( 5 25 33025 33541 ) -> \n ( ");
  for (i=0;i<4;i++) printf("%d ",b[i>>1].sdw[i%2]);
  printf(")\n");

  pmullw(a,a,b,8);
  printf("pmullw ( 1 4 9 16 16384 16641 16641 16900 ) -> \n ( "); 
  for (i=0;i<8;i++) printf("%d ",b[i>>2].sw[i%4]);
  printf(")\n");

  psllw(a,8,a,8);
  pmulhw(a,a,b,8);
  printf("pmulhw ( 1 4 9 16 16384 16129 16129 15876 ) -> \n ( "); 
  for (i=0;i<8;i++) printf("%d ",b[i>>2].sw[i%4]);
  printf(")\n");

  memcpy(a,ut0,sizeof(ut0));
  por(a,oneb,b,sizeof(ut0));
  printf("por  ( 1 3 3 5 MAX MAX MAX MAX ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].ub[i]);
  printf(")\n");

  memcpy(a,ut1,sizeof(ut1));
  psllw(a,7,b,8);
  printf("psllw  ( 128 256 384 512 32768 32896 33024 33152 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].uw[i]);
  printf(")\n");

  memcpy(a,t2,sizeof(t2));
  pslld(a,7,b,8);
  printf("pslld  ( 128 -256 384 -512 4194304 -4194432 4194432 -4194560 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].sdw[i]);
  printf(")\n");

  memcpy(a,t1,sizeof(t1));
  for (i=0;i<8;i++) printf("%d ",a[0].sw[i]);
  psraw(a,1,b,8);
  printf("psraw  ( 0 -1 1 -2, 64, -65, 64, -65 ) -> \n ( ");
  for (i=0;i<8;i++) printf("%d ",b[0].sw[i]);
  printf(")\n");


}

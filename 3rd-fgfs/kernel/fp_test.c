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
 * $Id: fp_test.c,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $
 */



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <limits.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgvars.h"

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000L
#endif

inline FG_INT32 fp_iovf(double c)
{
  c=((c>INT_MAX)? c-UINT_MAX-1 : c);
  c=((c<INT_MIN)? c+UINT_MAX+1 : c);
  return((FG_INT32)c);
}


#include "isqrt.c"

inline FG_INT32 fp_add(FG_INT32 a, FG_INT32 b)
{
  return fp_iovf((double)a+(double)b);
}

inline FG_INT32 i_add(FG_INT32 a, FG_INT32 b)
{
  return (a+b);
}

inline FG_INT32 fp_sub(FG_INT32 a, FG_INT32 b)
{
  return fp_iovf((double)a-(double)b);
}

inline FG_INT32 i_sub(FG_INT32 a, FG_INT32 b)
{
  return (a-b);
}

inline FG_INT64 fp_mul(FG_INT32 a, FG_INT32 b)
{
  return (FG_INT64)(((double)a)/FG_ONE*(double)b);
}

inline FG_INT64 l_mul(FG_INT32 a, FG_INT32 b)
{
  return ((FG_INT64)a*(FG_INT64)b/FG_ONE);
}

inline FG_INT64 i_mul(FG_INT32 a, FG_INT32 b)
{
  register FG_INT32 xl,xh,yl,yh;
  xl=(a % FG_ONE);
  xh=(a/FG_ONE);
  yl=(b % FG_ONE);
  yh=(b/FG_ONE);
  return ((FG_INT64)(xh*yh)*FG_ONE+(FG_INT32)xh*yl+(FG_INT32)yh*xl
           +(FG_INT32)yl*xl/FG_ONE);
}

inline FG_INT32 fp_div(FG_INT64 a, FG_INT32 b)
{
  return (FG_INT32)((double)a/(double)b*(double)FG_ONE);
}

inline FG_INT32 l_div(FG_INT64 a, FG_INT32 b)
{
  register FG_INT64 result=(((FG_INT64)a*(FG_INT64)FG_ONE)/b);
  if (result>INT_MAX) result=INT_MAX;
  if (result<INT_MIN) result=INT_MIN;
  return (FG_INT32)result;
}


int main(void)
{
  int i,errors=0;
  clock_t time1,time2,time3,time4,clock0;
  FG_INT32 a,b,c,d,e,f;
  FG_INT64 aa,cc,dd,ee;

  /* test addition */
  srandom(time(0));
  for (i=0;i<100;i++) {
     a=random()-INT_MAX/2;
     b=random()-INT_MAX/2;
     c=fp_add(a,b);
     d=i_add(a,b);
     if (c != d) {
       errors++;
       printf("/* Error %d + %d = %d and %d*/\n",a,b,c,d);
     }
  }
  printf("/* %d addition errors */\n",errors);
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    c=fp_add(a,b);
  }
  time1=clock()-clock0;
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    c=i_add(a,b);
  }
  time2=clock()-clock0;
  printf("/* fp_add() %f calls/second */\n",1000000./time1*CLOCKS_PER_SEC);
  printf("/* i_add()  %f calls/second */\n",1000000./time2*CLOCKS_PER_SEC);
  if (time2<time1) {
    printf("#define _USE_INT_ADD_\n");
  } else {
    printf("#define _USE_FP_ADD_\n");
  }

  /* test subtraction */
  errors=0;
  srandom(time(0));
  for (i=0;i<100;i++) {
     a=random()-INT_MAX/2;
     b=random()-INT_MAX/2;
     c=fp_sub(a,b);
     d=i_sub(a,b);
     if (c != d) {
       errors++;
       printf("/* Error %d - %d = %d and %d  */\n",a,b,c,d);
     }
  }
  printf("/* %d subtraction errors */\n",errors);
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    c=fp_sub(a,b);
  }
  time1=clock()-clock0;
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    c=i_sub(a,b);
  }
  time2=clock()-clock0;
  printf("/* fp_sub() %f calls/second */\n",1000000./time1*CLOCKS_PER_SEC);
  printf("/* i_sub()  %f calls/second */\n",1000000./time2*CLOCKS_PER_SEC);
  if (time2<time1) {
    printf("#define _USE_INT_SUB_\n");
  } else {
    printf("#define _USE_FP_SUB_\n");
  }

  /* test multiplication */
  errors=0;
  srandom(time(0));
  for (i=0;i<100;i++) {
     a=random()-INT_MAX/2;
     b=random()-INT_MAX/2;
     cc=fp_mul(a,b);
     dd=i_mul(a,b);
     ee=l_mul(a,b);
     if ((cc != dd) || (cc!=ee)) {
       errors++;
       printf("/* Error %f * %f = %f,%f, and %f */\n",
           (double)a/FG_ONE,(double)b/FG_ONE,(double)cc/FG_ONE,
           (double)dd/FG_ONE,(double)ee/FG_ONE);
     }
  }
  printf("/* %d multiplication errors */\n",errors);
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    cc=fp_mul(a,b);
  }
  time1=clock()-clock0;
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    cc=i_mul(a,b);
  }
  time2=clock()-clock0;
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    cc=l_mul(a,b);
  }
  time3=clock()-clock0;
  time4=FG_MIN(time1,FG_MIN(time2,time3));
  printf("/* fp_mul() %f calls/second */\n",1000000./time1*CLOCKS_PER_SEC);
  printf("/* i_mul()  %f calls/second */\n",1000000./time2*CLOCKS_PER_SEC);
  printf("/* l_mul()  %f calls/second */\n",1000000./time3*CLOCKS_PER_SEC);

  if (time4==time1) {
    printf("#define _USE_FP_MUL_\n");
  } else {
    if (time4==time2) {
      printf("#define _USE_INT_MUL_\n");
    } else {
      printf("#define _USE_LONG_MUL_\n");
    }
  }

  /* test division */
  errors=0;
  srandom(time(0));
  for (i=0;i<100;i++) {
     a=random()-INT_MAX/2;
     b=random()-INT_MAX/2;
     aa=fp_mul(a,b);
     b=random()-INT_MAX/2;
     c=fp_div(aa,b);
     e=l_div(aa,b);
     if (c != e)  {
       errors++;
       printf("/* Error %f / %f = %f and %f*/\n",
           (double)aa/FG_ONE,(double)b/FG_ONE,
           (double)c/FG_ONE,(double)e/FG_ONE);
     }
  }
  printf("/* %d division errors */\n",errors);
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    aa=fp_mul(a,b);
    b=random()-INT_MAX/2;
    c=fp_div(aa,b);
  }
  time1=clock()-clock0;
  clock0=clock();
  while (clock0==clock());
  clock0=clock();
  for (i=0;i<1000000;i++) {
    a=random()-INT_MAX/2;
    b=random()-INT_MAX/2;
    aa=fp_mul(a,b);
    b=random()-INT_MAX/2;
    c=l_div(aa,b);
  }
  time3=clock()-clock0;
  printf("/* fp_div() %f calls/second */\n",1000000./time1*CLOCKS_PER_SEC);
  printf("/* l_div()  %f calls/second */\n",1000000./time3*CLOCKS_PER_SEC);

  if (time3>time1) {
    printf("#define _USE_FP_DIV_\n");
  } else {
    printf("#define _USE_LONG_DIV_\n");
  }

  /* test sqrt */
  {
    double nerror=0,ierror=0;
    errors=0;
    srandom(time(0));
    for (i=0;i<100;i++) {
      a=(unsigned)random()>>1;
      b=(unsigned)random()>>1;
      aa=fp_mul(a,b);
      c=fp_isqrt(aa);
      d=i_isqrt(aa);
      e=n_isqrt(aa);
      nerror+=(double)(e-c);
      ierror+=(double)(d-c);
    }
    printf("/*  avg n_isqrt() error is %f */\n",nerror/100/FG_ONE);
    printf("/*  avg i_isqrt() error is %f */\n",ierror/100/FG_ONE);
    clock0=clock();
    while (clock0==clock());
    clock0=clock();
    for (i=0;i<250000;i++) {
      a=(unsigned)random()>>1;
      b=(unsigned)random()>>1;
      aa=fp_mul(a,b);
      c=fp_isqrt(aa);
    }
    time1=clock()-clock0;
    clock0=clock();
    while (clock0==clock());
    clock0=clock();
    for (i=0;i<250000;i++) {
      a=(unsigned)random()>>1;
      b=(unsigned)random()>>1;
      aa=fp_mul(a,b);
      c=n_isqrt(aa);
    }
    time2=clock()-clock0;
    clock0=clock();
    while (clock0==clock());
    clock0=clock();
    for (i=0;i<250000;i++) {
      a=(unsigned)random()>>1;
      b=(unsigned)random()>>1;
      aa=fp_mul(a,b);
      c=i_isqrt(aa);
    }
    time3=clock()-clock0;
    time4=FG_MIN(time1,FG_MIN(time2,time3));
    printf("/* fp_isqrt() %f calls/second */\n",250000./time1*CLOCKS_PER_SEC);
    printf("/* i_isqrt()  %f calls/second */\n",250000./time2*CLOCKS_PER_SEC);
    printf("/* l_isqrt()  %f calls/second */\n",250000./time3*CLOCKS_PER_SEC);
    if (time4==time1) {
      printf("#define _USE_FP_SQRT_\n");
    } else {
      if (time4==time2) {
        printf("#define _USE_N_SQRT_\n");
        printf("#define _SQRT_ERROR_ %d",(FG_INT32)nerror/100);
      } else {
        printf("#define _USE_I_SQRT_\n");
        printf("#define _SQRT_ERROR_ %d",(FG_INT32)ierror/100);
      }
    }
  }
}

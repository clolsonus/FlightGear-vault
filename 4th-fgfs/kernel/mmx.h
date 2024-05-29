#include <limits.h>
#include "fgdefs.h"
#include "fgtypes.h"

typedef union _MMXR {
  FG_UINT32  udw[2];
  FG_INT32   sdw[2];
  FG_UINT64  uqw;
  FG_INT64   sqw;
  FG_UINT16  uw[4];
  FG_INT16   sw[4];
  FG_UINT8   ub[8];
  FG_INT8    sb[8];
} MMXR;

const MMXR ODDB ={{0xff00ff00,0xff00ff00}};
const MMXR EVENB={{0x00ff00ff,0x00ff00ff}};
const MMXR ODDW ={{0xffff0000,0xffff0000}};
const MMXR EVENW={{0x0000ffff,0x0000ffff}};
const MMXR ODDD ={{0xffffffff,0x00000000}};
const MMXR EVEND={{0x00000000,0xffffffff}};
const MMXR SIGNB={{0x80808080,0x80808080}};
const MMXR SIGNW={{0x80008000,0x80008000}};
const MMXR SIGND={{0x80000000,0x80000000}};
const MMXR MANTB={{0x7f7f7f7f,0x7f7f7f7f}};
const MMXR MANTW={{0x7fff7fff,0x7fff7fff}};
const MMXR MANTD={{0x7fffffff,0x7fffffff}};
const MMXR word_shiftsl[16]={{{0xffffffff,0xffffffff}},
                            {{0xfffefffe,0xfffefffe}},
                            {{0xfffcfffc,0xfffcfffc}},
                            {{0xfff8fff8,0xfff8fff8}},
                            {{0xfff0fff0,0xfff0fff0}},
                            {{0xffe0ffe0,0xffe0ffe0}},
                            {{0xffc0ffc0,0xffc0ffc0}},
                            {{0xff80ff80,0xff80ff80}},
                            {{0xff00ff00,0xff00ff00}},
                            {{0xfe00fe00,0xfe00fe00}},
                            {{0xfc00fc00,0xfc00fc00}},
                            {{0xf800f800,0xf800f800}},
                            {{0xf000f000,0xf000f000}},
                            {{0xe000e000,0xe000e000}},
                            {{0xc000c000,0xc000c000}},
                            {{0x80008000,0x80008000}}};
const MMXR word_shiftsr[16]={{{0xffffffff,0xffffffff}},
                            {{0x7fff7fff,0x7fff7fff}},
                            {{0x3fff3fff,0x3fff3fff}},
                            {{0x1fff1fff,0x1fff1fff}},
                            {{0x0fff0fff,0x0fff0fff}},
                            {{0x07ff07ff,0x07ff07ff}},
                            {{0x03ff03ff,0x03ff03ff}},
                            {{0x01ff01ff,0x01ff01ff}},
                            {{0x00ff00ff,0x00ff00ff}},
                            {{0x007f007f,0x007f007f}},
                            {{0x003f003f,0x003f003f}},
                            {{0x001f001f,0x001f001f}},
                            {{0x000f000f,0x000f000f}},
                            {{0x00070007,0x00070007}},
                            {{0x00030003,0x00030003}},
                            {{0x00010001,0x00010001}}};
#ifdef BIGENDIAN

#define HIGHER (-1)
#define LOWER 1
#define DW0 1
#define DW1 0
#define W0 3
#define W1 2
#define W2 1
#define W3 0

#else

#define LOWER (-1)
#define HIGHER 1
#define DW0 0
#define DW1 1
#define W0 0
#define W1 1
#define W2 2
#define W3 3
#endif



inline static void packsswb(FG_INT16 *a, FG_INT8 *b, int n) 
/* Pack n words at a into n bytes at b with signed saturation */
/* WARNING-- n must be evenly divisible by 8                  */
{
#ifndef _HAS_MMX
  int i,j;
  for (j=0; j<n; j+=8) {
    for (i=0; i<8; i++) {
      if (a[j+i] > SCHAR_MAX) {
        b[j+i]=SCHAR_MAX;
      } else {
        if (a[j+i] < SCHAR_MIN) {
          b[j+i]=SCHAR_MIN;
        } else {
          b[j+i]=a[j+i];
        }     
      }
    }
  }
#else
  /* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl 	%3,%0        
        bge 	1f
        movq 	(%1,%3),%%f0
        packsswb 8(%1,%3),%%f0
        movq 	%%f0,(%2,%3)
        addl 	%1,8
        addl    %3,8
        jmp 	0b
1:	emms"
  :: "c" (n), "a" (a), "d" (b), "b" (0)  
  : "f0");
#endif
}           

inline static void packuswb(FG_UINT16 *a, FG_UINT8 *b, int n) 
/* Pack n words at a into n bytes at b with unsigned saturation */
/* WARNING-- n must be evenly divisible by 8                    */
{
#ifndef _HAS_MMX
  int i,j;
  for (j=0; j<n; j+=8) {
    for (i=0; i<8; i++) {
      if (a[j+i] > UCHAR_MAX) {
        b[j+i]=UCHAR_MAX;
      } else {
        b[j+i]=a[j+i];
      }     
    }
  }
#else
  /* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl 	%3,%0        
        bge 	1f
        movq 	(%1,%3),%%f0
        packuswb 8(%1,%3),%%f0
        movq 	%%f0,(%2,%3)
        addl 	%1,8
        addl    %3,8
        jmp 	0b
1:      emms"
  :: "c" (n), "a" (a), "d" (b), "b" (0)  
  : "f0");
#endif
}           

inline static void packssdw(FG_INT32 *a, FG_INT16 *b, int n) 
/* Pack n double words at a into n words at b with signed saturation */
/* WARNING-- n must be evenly divisible by 4                  */
{
#ifndef _HAS_MMX
  int i,j;
  for (j=0; j<n; j+=4) {
    for (i=0; i<4; i++) {
      if (a[j+i] > SHRT_MAX) {
        b[j+i]=SHRT_MAX;
      } else {
        if (a[j+i] < SHRT_MIN) {
          b[j+i]=SHRT_MIN;
        } else {
          b[j+i]=a[j+i];
        }     
      }
    }
  }
#else
  /* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl 	%3,%0        
        bge 	1f
        movq 	(%1,%3),%%f0
        packssdw 8(%1,%3),%%f0
        movq 	%%f0,(%2,%3)
        addl 	%1,8
        addl    %3,8
        jmp	0b
1:	emms"
  :: "c" (n), "a" (a), "d" (b), "b" (0)  
  : "f0");
#endif
}           

inline static void packusdw(FG_UINT32 *a, FG_UINT16 *b, int n) 
/* Pack n double words at a into n words at b with unsigned saturation */
/* WARNING-- n must be evenly divisible by 4                           */
{
#ifndef _HAS_MMX
  int i,j;
  for (j=0; j<n; j+=4) {
    for (i=0; i<4; i++) {
      if (a[j+i] > USHRT_MAX) {
        b[j+i]=USHRT_MAX;
      } else {
        b[j+i]=a[j+i];
      }
    }
  }
#else
  /* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl 	%3,%0        
        bge 	1f
        movq 	(%1,%3),%%f0
        packusdw 8(%1,%3),%%f0
        movq 	%%f0,(%2,%3)
        addl 	%1,8
        addl    %3,8
        jmp	0b
1:	emms"
  :: "c" (n), "a" (a), "d" (b), "b" (0)  
  : "f0");
#endif
}           

inline static void paddb(FG_UINT8 *a, FG_UINT8 *b, FG_UINT8 *result , int n) 
/* Packed add of n bytes with overflow  result[i]=a[i]+b[i]     */
/* WARNING-- n must be divisible by 8!                          */
{
#ifndef _HAS_MMX
  register MMXR *a1=(MMXR *)a;
  register MMXR *b1=(MMXR *)b;
  register MMXR *r1=(MMXR *)result;
  register FG_UINT64 odd,even;
  register int i;
  for (i=0;i<(n>>3);i++) {
    odd=(((a1[i].uqw & ODDB.uqw)+(b1[i].uqw & ODDB.uqw)) & ODDB.uqw);
    even=(((a1[i].uqw & EVENB.uqw)+(b1[i].uqw & EVENB.uqw)) & EVENB.uqw);
    r1[i].uqw=odd | even;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
        cmpl    %3,%0
        bge 	1f
        movq 	(%1,%3,8),%%f0
        paddb 	(%2,%3,8),%%f0
        movq 	%%f0,(%4,%3,8)
        incl    %3
        jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void paddw(FG_UINT16 *a, FG_UINT16 *b, FG_UINT16 *result , int n) 
/* Packed add of n words with overflow  result[i]=a[i]+b[i]     */
/* WARNING-- n must be divisible by 4!                          */
{
#ifndef _HAS_MMX
  register MMXR *a1=(MMXR *)a;
  register MMXR *b1=(MMXR *)b;
  register MMXR *r1=(MMXR *)result;
  register FG_UINT64 odd,even;
  register int i;
  for (i=0;i<(n>>2);i++) {
    odd=(((a1[i].uqw & ODDW.uqw)+(b1[i].uqw & ODDW.uqw)) & ODDW.uqw);
    even=(((a1[i].uqw & EVENW.uqw)+(b1[i].uqw & EVENW.uqw)) & EVENW.uqw);
    r1[i].uqw=odd | even;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
        cmpl    %3,%0
        bge 	1f
        movq 	(%1,%3,8),%%f0
        paddw 	(%2,%3,8),%%f0
        movq 	%%f0,(%4,%3,8)
        incl    %3
        jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void paddd(FG_UINT32 *a, FG_UINT32 *b, FG_UINT32 *result, int n)
{
/* Packed add of n double words  with overflow  *result=*a+*b           */
/* WARNING-- n must be divisible by 2!                          */
#ifndef _HAS_MMX
  register int i;
  for (i=0;i<(n);i+=2) {
    result[i]=a[i]+b[i];
    result[i+1]=a[i+1]+b[i+1];
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	paddq 	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>1), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void paddsb(FG_INT8 *a, FG_INT8 *b, FG_INT8 *result, int n) 
/* Packed add of n signed bytes with saturation  result[i]=a[i]+b[i]  */
/* WARNING-- n must be divisible by 8!                                */
{
#ifndef _HAS_MMX
  register int k,i,j;
  for (k=0;k<n;k+=8) {
    for (i=0;i<8;i++) {
      j=(int)a[k+i]+(int)b[k+i];
      if (j>SCHAR_MAX) {
        result[k+i]=SCHAR_MAX;
      } else {
        if (j<SCHAR_MIN) {
          result[k+i]=SCHAR_MIN;
        } else {
          result[k+i]=j;
        }
      }
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	paddsb	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void paddsw(FG_INT16 *a, FG_INT16 *b, FG_INT16 *result, int n) 
/* Packed add of n signed words with saturation  result[i]=a[i]+b[i]  */
/* WARNING-- n must be divisible by 4!                                */
{
#ifndef _HAS_MMX
  register int k,i,j;
  for (k=0;k<n;k+=4) {
    for (i=0;i<4;i++) {
      j=(int)a[k+i]+(int)b[k+i];
      if (j>SHRT_MAX) {
        result[k+i]=SHRT_MAX;
      } else {
        if (j<SHRT_MIN) {
          result[k+i]=SHRT_MIN;
        } else {
          result[k+i]=j;
        }
      }
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	paddsw	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void paddusb(FG_UINT8 *a, FG_UINT8 *b, FG_UINT8 *result, int n) 
/* Packed add of n unsigned bytes with saturation  result[i]=a[i]+b[i]  */
/* WARNING-- n must be divisible by 8!                                  */
{
#ifndef _HAS_MMX
  register int k,i,j;
  for (k=0;k<n;k+=8) {
    for (i=0;i<8;i++) {
      j=(FG_UINT)a[k+i]+(FG_UINT)b[k+i];
      if (j>UCHAR_MAX) {
        result[k+i]=UCHAR_MAX;
      } else {
        result[k+i]=j;
      }
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	paddusb	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void paddusw(FG_UINT16 *a, FG_UINT16 *b, FG_UINT16 *result, int n) 
/* Packed add of n unsigned words with saturation  result[i]=a[i]+b[i]  */
/* WARNING-- n must be divisible by 4!                                  */
{
#ifndef _HAS_MMX
  register int k,i,j;
  for (k=0;k<n;k+=4) {
    for (i=0;i<4;i++) {
      j=(FG_UINT32)a[k+i]+(FG_UINT32)b[k+i];
      if (j>USHRT_MAX) {
        result[k+i]=USHRT_MAX;
      } else {

        result[k+i]=j;
      }
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	paddusw	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           


inline static void pand(FG_VOID *a, FG_VOID *b, FG_VOID *result, int n) 
/* Packed add of n bytes                                                */
/* WARNING-- n must be divisible by 8!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i;
  for (i=0;i<(n>>3);i++) {
    l->uqw=k->uqw & j->uqw;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pand	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms" 
  :
  : "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void pandn(FG_VOID *a, FG_VOID *b, FG_VOID *result, int n) 
/* Packed add not of n bytes [result=((~a) & b )]                       */
/* WARNING-- n must be divisible by 8!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i;
  for (i=0;i<(n>>3);i++) {
    l->uqw=(~k->uqw) & j->uqw;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pandn	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void pcmpeqb(FG_UINT8 *a, FG_UINT8 *b, FG_UINT8 *result, int n) 
/* Packed bytewise compare of memory ((a==b) ? 0xff : 0x00)             */
/* WARNING-- n must be divisible by 8!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i,m;
  for (i=0;i<(n>>3);i++) {
    l->uqw=(k->uqw) ^ (j->uqw);
    for (m=0;m<8;m++) l->ub[m]=(l->ub[m]==0)*0xff;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pcmpeqb	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void pcmpeqw(FG_UINT16 *a, FG_UINT16 *b, FG_UINT16 *result, int n) 
/* Packed shortwise compare of memory ((a==b) ? 0xffff : 0x0000)         */
/* WARNING-- n must be divisible by 4!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i,m;
  for (i=0;i<(n>>2);i++) {
    l->uqw=(k->uqw) ^ (j->uqw);
    for (m=0;m<4;m++) l->uw[m]=(l->uw[m]==0)*0xffff;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pcmpeqw	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void pcmpeqd(FG_UINT32 *a, FG_UINT32 *b, FG_UINT32 *result, int n) 
/* Packed intwise compare of memory ((a==b) ? 0xffffffff : 0x00000000)  */
/* WARNING-- n must be divisible by 2!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i;
  for (i=0;i<(n>>1);i++) {
    l->udw[0]=(k->udw[0] == j->udw[0])*0xffffffff;
    l->udw[1]=(k->udw[1] == j->udw[1])*0xffffffff;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pcmpeqd	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>1), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           


inline static void pcmpgtb(FG_INT8 *a, FG_INT8 *b, FG_UINT8 *result, int n) 
/* Packed bytewise compare of memory ((a>b) ? 0xff : 0x00)             */
/* WARNING-- n must be divisible by 8!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i,m;
  for (i=0;i<(n>>3);i++) {
    for (m=0;m<8;m++) l->ub[m]=(k->sb[m] > j->sb[m])*0xff;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pcmpgtb	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void pcmpgtw(FG_INT16 *a, FG_INT16 *b, FG_UINT16 *result, int n) 
/* Packed bytewise compare of memory ((a>b) ? 0xffff : 0x0000)          */
/* WARNING-- n must be divisible by 4!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i,m;
  for (i=0;i<(n>>2);i++) {
    for (m=0;m<4;m++) l->uw[m]=(k->sw[m] > j->sw[m])*0xffff;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pcmpgtw	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}          

inline static void pcmpgtd(FG_INT32 *a, FG_INT32 *b, FG_UINT32 *result, int n) 
/* Packed intwise compare of memory (( a>b ) ? 0xffffffff : 0x00000000)  */
/* WARNING-- n must be divisible by 2!                                   */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i;
  for (i=0;i<(n>>1);i++) {
    l->udw[0]=(k->sdw[0] > j->sdw[0])*0xffffffff;
    l->udw[1]=(k->sdw[1] > j->sdw[1])*0xffffffff;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pcmpgtd	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>1), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void pmaddwd(FG_INT16 *a, FG_INT16 *b, FG_INT32 *result, int n)
/* Packed multiply and add words to doubles                   */
/* Look at the intel docs to see what this does               */
{ 
#ifndef _HAS_MMX
  register int i;
  register MMXR *a1=(MMXR *)a;
  register MMXR *b1=(MMXR *)b;
  register MMXR *r1=(MMXR *)result;
  MMXR temp;

  for (i=0;i<(n>>2);i++) {
    temp.sdw[0]=(FG_INT32)a1[i].sw[0]*(FG_INT32)b1[i].sw[0]+
                (FG_INT32)a1[i].sw[1]*(FG_INT32)b1[i].sw[1];
    temp.sdw[1]=(FG_INT32)a1[i].sw[2]*(FG_INT32)b1[i].sw[2]+
                (FG_INT32)a1[i].sw[3]*(FG_INT32)b1[i].sw[3];
    r1[i].sqw=temp.sqw;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pmaddwd	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           


inline static void pmulhw(FG_INT16 *a, FG_INT16 *b, FG_INT16 *result, int n)
/* Packed multiply storing the high word of the result        */
{ 
#ifndef _HAS_MMX
  register int i,j;
  register MMXR *a1=(MMXR *)a;
  register MMXR *b1=(MMXR *)b;
  register MMXR *r1=(MMXR *)result;

  for (i=0;i<(n>>2);i++) {
    for (j=0;j<4;j++) {
      r1[i].sw[j]=((FG_INT32)a1[i].sw[j]*(FG_INT32)b1[i].sw[j])>>16;
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pmulhw	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void pmullw(FG_INT16 *a, FG_INT16 *b, FG_INT16 *result, int n)
/* Packed multiply storing the low word of the result        */
{ 
#ifndef _HAS_MMX
  register int i,j;
  register MMXR *a1=(MMXR *)a;
  register MMXR *b1=(MMXR *)b;
  register MMXR *r1=(MMXR *)result;

  for (i=0;i<(n>>2);i++) {
    for (j=0;j<4;j++) {
      r1[i].sw[j]=((FG_INT32)a1[i].sw[j]*(FG_INT32)b1[i].sw[j]) & 0xffff;
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pmullw	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void por(FG_VOID *a, FG_VOID *b, FG_VOID *result, int n) 
/* Packed or of n bytes                                                 */
/* WARNING-- n must be divisible by 8!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i;
  for (i=0;i<(n>>3);i++) {
    l->uqw=k->uqw | j->uqw;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	por	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psllw(FG_UINT16 *a, int count, FG_UINT16 *result, int n) 
/* Performs a logical shift left on an array of words                   */
/* WARNING-- n must be divisible by 4!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *l=(MMXR *)result;
  register int i;
  if (count < 16) {
    for (i=0;i<(n>>2);i++) {
      l->uqw=(k->uqw<<count) & word_shiftsl[count].uqw;
      k++; l++;
    }
  } else {
    for (i=0;i<(n>>2);i++) {
      l->uqw=0;
      k++; l++;
    }
  }
#else
/* until gas supports mmx, this won't work */
  MMXR temp;
  temp.uqw=count;
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psllw	(%2),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (&temp), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void pslld(FG_UINT32 *a, int count, FG_UINT32 *result, int n) 
/* Performs a logical shift left on an array of double words            */
/* WARNING-- n must be divisible by 2!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *l=(MMXR *)result;
  register int i;
  if (count < 32) {
    for (i=0;i<(n>>1);i++) {
      l->udw[0]=k->udw[0]<<count;
      l->udw[1]=k->udw[1]<<count;
      k++; l++;
    }
  } else {
    for (i=0;i<(n>>1);i++) {
      l->uqw=0;
      k++; l++;
    }
  }
#else
/* until gas supports mmx, this won't work */
  MMXR temp;
  temp.uqw=count;
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pslld	(%2),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>1), "a" (a), "d" (&temp), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psllq(FG_UINT64 *a, int count, FG_UINT64 *result, int n) 
/* Performs a logical shift left on an array of words                   */
{
#ifndef _HAS_MMX
  register int i;
  for (i=0;i<(n);i++) {
    result[i]=a[i] << count;
  }
#else
/* until gas supports mmx, this won't work */
  MMXR temp;
  temp.uqw=count;
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psllq	(%2),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n), "a" (a), "d" (&temp), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psraw(FG_UINT16 *a, int count, FG_UINT16 *result, int n) 
/* Performs an arithmetic shift left on an array of words               */
/* WARNING-- n must be divisible by 4!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *l=(MMXR *)result;
  register int i,m;
  for (i=0;i<(n>>2);i++) {
    for (m=0;m<4;m++) {
      l->sw[m]=(k->sw[m]>>count);
    }
    k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  MMXR temp;
  temp.uqw=count;
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psraw	(%2),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (&temp), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psrad(FG_UINT32 *a, int count, FG_UINT32 *result, int n) 
/* Performs a logical shift left on an array of double words            */
/* WARNING-- n must be divisible by 2!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *l=(MMXR *)result;
  register int i;
  for (i=0;i<(n>>1);i++) {
    l->sdw[0]=k->sdw[0]>>count;
    l->sdw[1]=k->sdw[1]>>count;
    k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  MMXR temp;
  temp.uqw=count;
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psrad	(%2),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>1), "a" (a), "d" (&temp), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psrlw(FG_UINT16 *a, int count, FG_UINT16 *result, int n) 
/* Performs a logical shift left on an array of words                   */
/* WARNING-- n must be divisible by 4!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *l=(MMXR *)result;
  register int i;
  if (count < 16) {
    for (i=0;i<(n>>2);i++) {
      l->uqw=(k->uqw>>count) & word_shiftsr[count].uqw;
      k++; l++;
    }
  } else {
    for (i=0;i<(n>>2);i++) {
      l->uqw=0;
      k++; l++;
    }
  }
#else
/* until gas supports mmx, this won't work */
  MMXR temp;
  temp.uqw=count;
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psrlw	(%2),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (&temp), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psrld(FG_UINT32 *a, int count, FG_UINT32 *result, int n) 
/* Performs a logical shift left on an array of double words            */
/* WARNING-- n must be divisible by 2!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *l=(MMXR *)result;
  register int i;
  if (count < 32) {
    for (i=0;i<(n>>1);i++) {
      l->udw[0]=k->udw[0]>>count;
      l->udw[1]=k->udw[1]>>count;
      k++; l++;
    }
  } else {
    for (i=0;i<(n>>1);i++) {
      l->uqw=0;
      k++; l++;
    }
  }
#else
/* until gas supports mmx, this won't work */
  MMXR temp;
  temp.uqw=count;
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psrld	(%2),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>1), "a" (a), "d" (&temp), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psrlq(FG_UINT64 *a, int count, FG_UINT64 *result, int n) 
/* Performs a logical shift left on an array of words                   */
{
#ifndef _HAS_MMX
  register int i;
  for (i=0;i<(n);i++) {
    result[i]=a[i] >> count;
  }
#else
/* until gas supports mmx, this won't work */
  MMXR temp;
  temp.uqw=count;
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psrlq	(%2),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n), "a" (a), "d" (&temp), "b" (0) , "S" (result) 
  : "f0");
#endif
}           


inline static void psubb(FG_UINT8 *a, FG_UINT8 *b, FG_UINT8 *result , int n) 
/* Packed add of n bytes with overflow  result[i]=a[i]-b[i]     */
/* WARNING-- n must be divisible by 8!                          */
{
#ifndef _HAS_MMX
  register MMXR *a1=(MMXR *)a;
  register MMXR *b1=(MMXR *)b;
  register MMXR *r1=(MMXR *)result;
  register FG_UINT64 odd,even;
  register int i;
  for (i=0;i<(n>>3);i++) {
    odd=(((a1[i].uqw & ODDB.uqw)+((b1[i].uqw * -1) & ODDB.uqw)) & ODDB.uqw);
    even=(((a1[i].uqw & EVENB.uqw)+((b1[i].uqw * -1) & EVENB.uqw)) & EVENB.uqw);
    r1[i].uqw=odd | even;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
        cmpl    %3,%0
        bge 	1f
        movq 	(%1,%3,8),%%f0
        psubb 	(%2,%3,8),%%f0
        movq 	%%f0,(%4,%3,8)
        incl    %3
        jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psubw(FG_UINT16 *a, FG_UINT16 *b, FG_UINT16 *result , int n) 
/* Packed add of n words with overflow  result[i]=a[i]-b[i]     */
/* WARNING-- n must be divisible by 4!                          */
{
#ifndef _HAS_MMX
  register MMXR *a1=(MMXR *)a;
  register MMXR *b1=(MMXR *)b;
  register MMXR *r1=(MMXR *)result;
  register FG_UINT64 odd,even;
  register int i;
  for (i=0;i<(n>>2);i++) {
    odd=(((a1[i].uqw & ODDW.uqw)+((b1[i].uqw * -1) & ODDW.uqw)) & ODDW.uqw);
    even=(((a1[i].uqw & EVENW.uqw)+((b1[i].uqw * -1) & EVENW.uqw)) & EVENW.uqw);
    r1[i].uqw=odd | even;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
        cmpl    %3,%0
        bge 	1f
        movq 	(%1,%3,8),%%f0
        psubw 	(%2,%3,8),%%f0
        movq 	%%f0,(%4,%3,8)
        incl    %3
        jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psubd(FG_UINT32 *a, FG_UINT32 *b, FG_UINT32 *result, int n)
/* Packed add of n double words  with overflow  *result=*a-*b           */
/* WARNING-- n must be divisible by 2!                          */
{
#ifndef _HAS_MMX
  register int i;
  for (i=0;i<(n);i+=2) {
    result[i]=a[i]-b[i];
    result[i+1]=a[i+1]-b[i+1];
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psubd 	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>1), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           


inline static void psubsb(FG_INT8 *a, FG_INT8 *b, FG_INT8 *result, int n) 
/* Packed add of n signed bytes with saturation  result[i]=a[i]+b[i]  */
/* WARNING-- n must be divisible by 8!                                */
{
#ifndef _HAS_MMX
  register int k,i,j;
  for (k=0;k<n;k+=8) {
    for (i=0;i<8;i++) {
      j=(int)a[k+i]-(int)b[k+i];
      if (j>SCHAR_MAX) {
        result[k+i]=SCHAR_MAX;
      } else {
        if (j<SCHAR_MIN) {
          result[k+i]=SCHAR_MIN;
        } else {
          result[k+i]=j;
        }
      }
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psubsb	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psubsw(FG_INT16 *a, FG_INT16 *b, FG_INT16 *result, int n) 
/* Packed add of n signed words with saturation  result[i]=a[i]+b[i]  */
/* WARNING-- n must be divisible by 4!                                */
{
#ifndef _HAS_MMX
  register int k,i,j;
  for (k=0;k<n;k+=4) {
    for (i=0;i<4;i++) {
      j=(int)a[k+i]-(int)b[k+i];
      if (j>SHRT_MAX) {
        result[k+i]=SHRT_MAX;
      } else {
        if (j<SHRT_MIN) {
          result[k+i]=SHRT_MIN;
        } else {
          result[k+i]=j;
        }
      }
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psubsw	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           


inline static void psubusb(FG_UINT8 *a, FG_UINT8 *b, FG_UINT8 *result, int n) 
/* Packed add of n unsigned bytes with saturation  result[i]=a[i]+b[i]  */
/* WARNING-- n must be divisible by 8!                                  */
{
#ifndef _HAS_MMX
  register FG_INT32 k,i,j;
  for (k=0;k<n;k+=8) {
    for (i=0;i<8;i++) {
      j=a[k+i]-b[k+i];
      if (j<0) {
        result[k+i]=0;
      } else {
        result[k+i]=j;
      }
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psubusb	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void psubusw(FG_UINT16 *a, FG_UINT16 *b, FG_UINT16 *result, int n) 
/* Packed add of n unsigned words with saturation  result[i]=a[i]+b[i]  */
/* WARNING-- n must be divisible by 4!                                  */
{
#ifndef _HAS_MMX
  register FG_INT32 k,i,j;
  for (k=0;k<n;k+=4) {
    for (i=0;i<4;i++) {
      j=a[k+i]-b[k+i];
      if (j<0) {
        result[k+i]=0;
      } else {
        result[k+i]=j;
      }
    }
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	psubusw	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms"
  :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

inline static void punpckbw(FG_UINT8 *a, FG_UINT8 *b, FG_UINT16 *result, int n)
/* Unpacks bytes into words in interleaved fashion with b going into the */
/* high order bytes (b=NULL will zero extend)                            */
/* WARNING- n must be divisible by 8!                                    */
{
#ifndef _HAS_MMX
   register int i,j;
   if (b) {
     for (i=0;i<n;i+=8) {
       for (j=0;j<8;j++) {
         result[i+j]=a[i+j]+(b[i+j]<<8);
       }
     }
   } else {
     for (i=0;i<n;i+=8) {
       for (j=0;j<8;j++) {
         result[i+j]=a[i+j];
       } 
     }   
   }
#else
  if (b) {
    __asm__ ("
  0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,4),%%f0
        movq    %%f0,%%f1
	punpckhbw	(%2,%3,4),%%f0
        punpcklbw       (%2,%3,4),%%f1
	movq 	%%f0,8(%4,%3,8)
        movq	%%f1,(%4,%3,8)
	addl    $2,%3
	jmp	0b
  1:	emms"
    :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
    : "f0","f1");
  } else {
    __asm__ ("
        pxor	%%f2,%%f2
  0:    cmpl    %3,%0
        bge     1f
        movq    (%1,%3,4),%%f0
        movq    %%f0,%%f1
        punpckhbw       %%f2,%%f0
        punpcklbw       %%f2,%%f1
        movq    %%f0,8(%4,%3,8)
        movq    %%f1,(%4,%3,8) 
        addl    $2,%3
        jmp     0b
  1:    emms"   
    :: "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result)
    : "f0","f1","f2");
  }
#endif
}           

inline static void punpckwd(FG_UINT16 *a, FG_UINT16 *b, FG_UINT32 *result, int n)
/* Unpacks words into dwords in interleaved fashion with b going into the */
/* high order  word (b=NULL will zero extend)                             */
/* WARNING- n must be divisible by 4!                                     */
{
#ifndef _HAS_MMX
   register int i,j;
   if (b) {
     for (i=0;i<n;i+=4) {
       for (j=0;j<4;j++) {
         result[i+j]=a[i+j]+(b[i+j]<<16);
       }
     }
   } else {
     for (i=0;i<n;i+=4) {
       for (j=0;j<4;j++) {
         result[i+j]=a[i+j];
       } 
     }   
   }
#else
  if (b) {
    __asm__ ("
  0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,4),%%f0
        movq    %%f0,%%f1
	punpckhwd	(%2,%3,4),%%f0
        punpcklwd       (%2,%3,4),%%f1
	movq 	%%f0,8(%4,%3,8)
        movq	%%f1,(%4,%3,8)
	addl    $2,%3
	jmp	0b
  1:	emms"
    :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result) 
    : "f0","f1");
  } else {
    __asm__ ("
        pxor    %%f2,%%f2
  0:    cmpl    %3,%0
        bge     1f
        movq    (%1,%3,4),%%f0
        movq    %%f0,%%f1
        punpckhwd       %%f2,%%f0
        punpcklwd       %%f2,%%f1
        movq    %%f0,8(%4,%3,8)
        movq    %%f1,(%4,%3,8) 
        addl    $2,%3
        jmp     0b
  1:    emms"   
    :: "c" (n>>2), "a" (a), "d" (b), "b" (0) , "S" (result)
    : "f0","f1","f2");
  }
#endif
}

inline static void punpckdq(FG_UINT32 *a, FG_UINT32 *b, MMXR *result, int n)
/* Unpacks words into dwords in interleaved fashion with b going into the */
/* high order  word (b=NULL will zero extend)                             */
/* WARNING- n must be divisible by 2!                                     */
{
#ifndef _HAS_MMX
   register int i,j;
   if (b) {
     for (i=0;i<n;i+=2) {
         result[i].udw[DW0]=a[i];
         result[i].udw[DW1]=b[i];
         result[i+1].udw[DW0]=a[i+1];
         result[i+1].udw[DW1]=b[i+1];
     }
   } else {
     for (i=0;i<n;i+=2) {
       for (j=0;j<2;j++) {
         result[i+j].uqw=(FG_UINT64)a[i+j];
       } 
     }   
   }
#else
  if (b) {
    __asm__ ("
  0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,4),%%f0
        movq    %%f0,%%f1
	punpckhdq	(%2,%3,4),%%f0
        punpckldq       (%2,%3,4),%%f1
	movq 	%%f0,8(%4,%3,8)
        movq	%%f1,(%4,%3,8)
	addl    $2,%3
	jmp	0b
  1:	emms"
    :: "c" (n>>1), "a" (a), "d" (b), "b" (0) , "S" (result) 
    : "f0","f1");
  } else {
    __asm__ ("
        pxor    %%f2,%%f2
  0:    cmpl    %3,%0
        bge     1f
        movq    (%1,%3,4),%%f0
        movq    %%f0,%%f1
        punpckhdq       %%f2,%%f0
        punpckldq       %%f2,%%f1
        movq    %%f0,8(%4,%3,8)
        movq    %%f1,(%4,%3,8) 
        addl    $2,%3
        jmp     0b
  1:    emms"   
    :: "c" (n>>1), "a" (a), "d" (b), "b" (0) , "S" (result)
    : "f0","f1","f2");
  }
#endif
}

inline static void pxor(FG_VOID *a, FG_VOID *b, FG_VOID *result, int n) 
/* Packed add of n bytes                                                */
/* WARNING-- n must be divisible by 8!                                  */
{
#ifndef _HAS_MMX
  register MMXR *k=(MMXR *)a;
  register MMXR *j=(MMXR *)b;
  register MMXR *l=(MMXR *)result;
  register int i;
  for (i=0;i<(n>>3);i++) {
    l->uqw=k->uqw ^ j->uqw;
    j++; k++; l++;
  }
#else
/* until gas supports mmx, this won't work */
  __asm__ ("
0:	cmpl    %3,%0
	bge 	1f
	movq 	(%1,%3,8),%%f0
	pxor	(%2,%3,8),%%f0
	movq 	%%f0,(%4,%3,8)
	incl    %3
	jmp	0b
1:	emms" 
  :
  : "c" (n>>3), "a" (a), "d" (b), "b" (0) , "S" (result) 
  : "f0");
#endif
}           

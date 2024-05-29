#include "fgtypes.h"


#include "longlong.h"


#if 0
#ifdef BIGENDIAN   
  #define HIGHER   (-1)
  #define LOWER      1 
  #define L0         1 
  #define L1         0 
  #define S0         3
  #define S1         2
  #define S2         1
  #define S3         0
#else
  #define LOWER    (-1)
  #define HIGHER     1 
  #define L0         0 
  #define L1         1 
  #define S0         0 
  #define S1         1 
  #define S2         2
  #define S3         3
#endif
#endif


static const FG_UINT32 __clz_tab[] =
{
  0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
};


static inline FG_UINT64 shr64(FG_INT64 a, int b)
{
  FG_UINT64 result;
  result.l[L0]=(a.l[L0]>>b) | (a.l[L1]<<(32-b));
  result.l[L1]=(a.l[L1]>>b);
  return(result);
}

static inline FG_UINT64 sar64(FG_INT64 a, int b)
{
  FG_UINT64 result;
  result.l[L0]=(a.l[L0]>>b) | (a.l[L1]<<(32-b));
  result.l[L1]=((FG_INT32)(a.l[L1])>>b);
  return(result);
}

static inline FG_INT64 shl64(FG_INT64 a, int b)
{
  FG_INT64 result;
  result.l[L0]=(a.l[L0]<<b);
  result.l[L1]=(a.l[L1]<<b) | (a.l[L0]>>(32-b));
  return(result);
}

static inline FG_UINT64 add64(FG_UINT64 a, FG_UINT64 b)
{
  FG_INT64 result;
  register FG_UINT32 __x;	
  __x = a.l[L0] + b.l[L0];	
  result.l[L1] = a.l[L1] + b.l[L1] + (__x < a.l[L0]);
  result.l[L0] = __x;	
  return (result);
}

static inline FG_UINT64 sub64(FG_UINT64 a, FG_UINT64 b)
{
  FG_INT64 result;
  register FG_UINT32 __x;
  __x = a.l[L0] - b.l[L0];	
  result.l[L1] = a.l[L1] - b.l[L1] - (__x > a.l[L0]);	
  result.l[L0] = __x;	
  return result;
}

static inline FG_UINT64 umul_qqs(FG_UINT64 a, FG_UINT16 b)
{
  FG_UINT64 result;
  register int j;
  result.l[L0]=(FG_UINT32)a.s[S0]*b+(((FG_UINT32)a.s[S1]*b)<<16);
  result.l[L1]=((FG_UINT32)a.s[S1]*b)>>16
                 +(FG_UINT32)a.s[S2]*b
                 +(((FG_UINT32)a.s[S3]*b)<<16);
  return (result);
}

static inline FG_INT64 umul_qll(FG_UINT32 a, FG_UINT32 b)
{
  FG_UINT64 result;
  umul_ppmm(result.l[L1],result.l[L0],a.l[L1],a.l[L0],b.l[L1],b.l[L0]);
  return (result);
}

static inline FG_INT32 udiv64by32(FG_UINT64 a, FG_UINT32 b)
{
  FG_UINT32 result,dummy;
  udiv_qrnnd(result,dummy,a.l[L1],a.l[L0],b);
  return (result);
}  

static inline FG_INT32 umod64by32(FG_UINT64 a, FG_UINT32 b)
{
  FG_UINT32 result,dummy;
  udiv_qrnnd(dummy,result,a.l[L1],a.l[L0],b);
  return (result);
}  

static inline FG_INT32 sdiv64by32(FG_UINT64 a, FG_UINT32 b)
{
  FG_UINT32 result,dummy;
  sdiv_qrnnd(result,dummy,a.l[L1],a.l[L0],b);
  return (result);
}  

static inline FG_INT32 smod64by32(FG_UINT64 a, FG_UINT32 b)
{
  FG_UINT32 result,dummy;
  sdiv_qrnnd(dummy,result,a.l[L1],a.l[L0],b);
  return (result);
}  


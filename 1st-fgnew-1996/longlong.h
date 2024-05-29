/* longlong.h -- definitions for mixed size 32/64 bit arithmetic.
   Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.

   This definition file is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   This definition file is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef SI_TYPE_SIZE
#define SI_TYPE_SIZE 32
#endif

#define __BITS4 (SI_TYPE_SIZE / 4)
#define __ll_B (1L << (SI_TYPE_SIZE / 2))
#define __ll_lowpart(t) ((FG_UINT32) (t) % __ll_B)
#define __ll_highpart(t) ((FG_UINT32) (t) / __ll_B)

/* Define auxiliary asm macros.

   1) umul_ppmm(high_prod, low_prod, multipler, multiplicand)
   multiplies two FG_UINT32 integers MULTIPLER and MULTIPLICAND,
   and generates a two-part FG_UINT32 product in HIGH_PROD and
   LOW_PROD.

   2) __umulsidi3(a,b) multiplies two FG_UINT32 integers A and B,
   and returns a FG_UINT64 product.  This is just a variant of umul_ppmm.

   3) udiv_qrnnd(quotient, remainder, high_numerator, low_numerator,
   denominator) divides a two-word unsigned integer, composed by the
   integers HIGH_NUMERATOR and LOW_NUMERATOR, by DENOMINATOR and
   places the quotient in QUOTIENT and the remainder in REMAINDER.
   HIGH_NUMERATOR must be less than DENOMINATOR for correct operation.
   If, in addition, the most significant bit of DENOMINATOR must be 1,
   then the pre-processor symbol UDIV_NEEDS_NORMALIZATION is defined to 1.

   4) sdiv_qrnnd(quotient, remainder, high_numerator, low_numerator,
   denominator).  Like udiv_qrnnd but the numbers are signed.  The
   quotient is rounded towards 0.

   5) count_leading_zeros(count, x) counts the number of zero-bits from
   the msb to the first non-zero bit.  This is the number of steps X
   needs to be shifted left to set the msb.  Undefined for X == 0.

   6) add_ssaaaa(high_sum, low_sum, high_addend_1, low_addend_1,
   high_addend_2, low_addend_2) adds two two-word unsigned integers,
   composed by HIGH_ADDEND_1 and LOW_ADDEND_1, and HIGH_ADDEND_2 and
   LOW_ADDEND_2 respectively.  The result is placed in HIGH_SUM and
   LOW_SUM.  Overflow (i.e. carry out) is not stored anywhere, and is
   lost.

   7) sub_ddmmss(high_difference, low_difference, high_minuend,
   low_minuend, high_subtrahend, low_subtrahend) subtracts two
   two-word unsigned integers, composed by HIGH_MINUEND_1 and
   LOW_MINUEND_1, and HIGH_SUBTRAHEND_2 and LOW_SUBTRAHEND_2
   respectively.  The result is placed in HIGH_DIFFERENCE and
   LOW_DIFFERENCE.  Overflow (i.e. carry out) is not stored anywhere,
   and is lost.

   If any of these macros are left undefined for a particular CPU,
   C macros are used.  */

/* The CPUs come in alphabetical order below.

   Please add support for more CPUs here, or improve the current support
   for the CPUs below!
   (E.g. WE32100, IBM360.)  */

#if defined (__GNUC__) && !defined (NO_ASM)

/* We sometimes need to clobber "cc" with gcc2, but that would not be
   understood by gcc1.  Use cpp to avoid major code duplication.  */
#if __GNUC__ < 2
#define __CLOBBER_CC
#define __AND_CLOBBER_CC
#else /* __GNUC__ >= 2 */
#define __CLOBBER_CC : "cc"
#define __AND_CLOBBER_CC , "cc"
#endif /* __GNUC__ < 2 */

#if defined (__a29k__) || defined (_AM29K)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("add %1,%4,%5
	addc %0,%2,%3"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	    "=&r" ((FG_UINT32)(sl))					\
	   : "%r" ((FG_UINT32)(ah)),					\
	     "rI" ((FG_UINT32)(bh)),					\
	     "%r" ((FG_UINT32)(al)),					\
	     "rI" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("sub %1,%4,%5
	subc %0,%2,%3"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "r" ((FG_UINT32)(ah)),					\
	     "rI" ((FG_UINT32)(bh)),					\
	     "r" ((FG_UINT32)(al)),					\
	     "rI" ((FG_UINT32)(bl)))
#define umul_ppmm(xh, xl, m0, m1) \
  do {									\
    FG_UINT32 __m0 = (m0), __m1 = (m1);					\
    __asm__ ("multiplu %0,%1,%2"					\
	     : "=r" ((FG_UINT32)(xl))					\
	     : "r" (__m0),						\
	       "r" (__m1));						\
    __asm__ ("multmu %0,%1,%2"						\
	     : "=r" ((FG_UINT32)(xh))					\
	     : "r" (__m0),						\
	       "r" (__m1));						\
  } while (0)
#define udiv_qrnnd(q, r, n1, n0, d) \
  __asm__ ("dividu %0,%3,%4"						\
	   : "=r" ((FG_UINT32)(q)),					\
	     "=q" ((FG_UINT32)(r))					\
	   : "1" ((FG_UINT32)(n1)),					\
	     "r" ((FG_UINT32)(n0)),					\
	     "r" ((FG_UINT32)(d)))
#define count_leading_zeros(count, x) \
    __asm__ ("clz %0,%1"						\
	     : "=r" ((FG_UINT32)(count))					\
	     : "r" ((FG_UINT32)(x)))
#endif /* __a29k__ */

#if defined (__arm__)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("adds	%1, %4, %5
	adc	%0, %2, %3"						\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "%r" ((FG_UINT32)(ah)),					\
	     "rI" ((FG_UINT32)(bh)),					\
	     "%r" ((FG_UINT32)(al)),					\
	     "rI" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("subs	%1, %4, %5
	sbc	%0, %2, %3"						\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "r" ((FG_UINT32)(ah)),					\
	     "rI" ((FG_UINT32)(bh)),					\
	     "r" ((FG_UINT32)(al)),					\
	     "rI" ((FG_UINT32)(bl)))
#define umul_ppmm(xh, xl, a, b) \
{register FG_UINT32 __t0, __t1, __t2;					\
  __asm__ ("%@ Inlined umul_ppmm
	mov	%2, %5, lsr #16
	mov	%0, %6, lsr #16
	bic	%3, %5, %2, lsl #16
	bic	%4, %6, %0, lsl #16
	mul	%1, %3, %4
	mul	%4, %2, %4
	mul	%3, %0, %3
	mul	%0, %2, %0
	adds	%3, %4, %3
	addcs	%0, %0, #65536
	adds	%1, %1, %3, lsl #16
	adc	%0, %0, %3, lsr #16"					\
	   : "=&r" ((FG_UINT32)(xh)),					\
	     "=r" ((FG_UINT32)(xl)),					\
	     "=&r" (__t0), "=&r" (__t1), "=r" (__t2)			\
	   : "r" ((FG_UINT32)(a)),					\
	     "r" ((FG_UINT32)(b)));}
#define UMUL_TIME 20
#define UDIV_TIME 100
#endif /* __arm__ */

#if defined (__clipper__)
#define umul_ppmm(w1, w0, u, v) \
  ({union {FG_UINT64 __ll;						\
	   struct {FG_UINT32 __l, __h;} __i;				\
	  } __xx;							\
  __asm__ ("mulwux %2,%0"						\
	   : "=r" (__xx.__ll)						\
	   : "%0" ((FG_UINT32)(u)),					\
	     "r" ((FG_UINT32)(v)));					\
  (w1) = __xx.__i.__h; (w0) = __xx.__i.__l;})
#define smul_ppmm(w1, w0, u, v) \
  ({union {FG_INT64 __ll;							\
	   struct {FG_INT32 __l, __h;} __i;				\
	  } __xx;							\
  __asm__ ("mulwx %2,%0"						\
	   : "=r" (__xx.__ll)						\
	   : "%0" ((FG_INT32)(u)),					\
	     "r" ((FG_INT32)(v)));					\
  (w1) = __xx.__i.__h; (w0) = __xx.__i.__l;})
#define __umulsidi3(u, v) \
  ({FG_UINT64 __w;							\
    __asm__ ("mulwux %2,%0"						\
	     : "=r" (__w)						\
	     : "%0" ((FG_UINT32)(u)),					\
	       "r" ((FG_UINT32)(v)));					\
    __w; })
#endif /* __clipper__ */

#if defined (__gmicro__)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("add.w %5,%1
	addx %3,%0"							\
	   : "=g" ((FG_UINT32)(sh)),					\
	     "=&g" ((FG_UINT32)(sl))					\
	   : "%0" ((FG_UINT32)(ah)),					\
	     "g" ((FG_UINT32)(bh)),					\
	     "%1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("sub.w %5,%1
	subx %3,%0"							\
	   : "=g" ((FG_UINT32)(sh)),					\
	     "=&g" ((FG_UINT32)(sl))					\
	   : "0" ((FG_UINT32)(ah)),					\
	     "g" ((FG_UINT32)(bh)),					\
	     "1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#define umul_ppmm(ph, pl, m0, m1) \
  __asm__ ("mulx %3,%0,%1"						\
	   : "=g" ((FG_UINT32)(ph)),					\
	     "=r" ((FG_UINT32)(pl))					\
	   : "%0" ((FG_UINT32)(m0)),					\
	     "g" ((FG_UINT32)(m1)))
#define udiv_qrnnd(q, r, nh, nl, d) \
  __asm__ ("divx %4,%0,%1"						\
	   : "=g" ((FG_UINT32)(q)),					\
	     "=r" ((FG_UINT32)(r))					\
	   : "1" ((FG_UINT32)(nh)),					\
	     "0" ((FG_UINT32)(nl)),					\
	     "g" ((FG_UINT32)(d)))
#define count_leading_zeros(count, x) \
  __asm__ ("bsch/1 %1,%0"						\
	   : "=g" (count)						\
	   : "g" ((FG_UINT32)(x)),					\
	     "0" ((FG_UINT32)0))
#endif

#if defined (__hppa)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("add %4,%5,%1
	addc %2,%3,%0"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "%rM" ((FG_UINT32)(ah)),					\
	     "rM" ((FG_UINT32)(bh)),					\
	     "%rM" ((FG_UINT32)(al)),					\
	     "rM" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("sub %4,%5,%1
	subb %2,%3,%0"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "rM" ((FG_UINT32)(ah)),					\
	     "rM" ((FG_UINT32)(bh)),					\
	     "rM" ((FG_UINT32)(al)),					\
	     "rM" ((FG_UINT32)(bl)))
#if defined (_PA_RISC1_1)
#define umul_ppmm(w1, w0, u, v) \
  do {									\
    union								\
      {									\
	FG_UINT64 __f;							\
	struct {FG_UINT32 __w1, __w0;} __w1w0;				\
      } __t;								\
    __asm__ ("xmpyu %1,%2,%0"						\
	     : "=x" (__t.__f)						\
	     : "x" ((FG_UINT32)(u)),					\
	       "x" ((FG_UINT32)(v)));					\
    (w1) = __t.__w1w0.__w1;						\
    (w0) = __t.__w1w0.__w0;						\
     } while (0)
#define UMUL_TIME 8
#else
#define UMUL_TIME 30
#endif
#define UDIV_TIME 40
#define count_leading_zeros(count, x) \
  do {									\
    FG_UINT32 __tmp;							\
    __asm__ (								\
       "ldi		1,%0
	extru,=		%1,15,16,%%r0		; Bits 31..16 zero?
	extru,tr	%1,15,16,%1		; No.  Shift down, skip add.
	ldo		16(%0),%0		; Yes.  Perform add.
	extru,=		%1,23,8,%%r0		; Bits 15..8 zero?
	extru,tr	%1,23,8,%1		; No.  Shift down, skip add.
	ldo		8(%0),%0		; Yes.  Perform add.
	extru,=		%1,27,4,%%r0		; Bits 7..4 zero?
	extru,tr	%1,27,4,%1		; No.  Shift down, skip add.
	ldo		4(%0),%0		; Yes.  Perform add.
	extru,=		%1,29,2,%%r0		; Bits 3..2 zero?
	extru,tr	%1,29,2,%1		; No.  Shift down, skip add.
	ldo		2(%0),%0		; Yes.  Perform add.
	extru		%1,30,1,%1		; Extract bit 1.
	sub		%0,%1,%0		; Subtract it.
	" : "=r" (count), "=r" (__tmp) : "1" (x));			\
  } while (0)
#endif

#if defined (__i386__) || defined (__i486__)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("addl %5,%1
	adcl %3,%0"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "%0" ((FG_UINT32)(ah)),					\
	     "g" ((FG_UINT32)(bh)),					\
	     "%1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("subl %5,%1
	sbbl %3,%0"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "0" ((FG_UINT32)(ah)),					\
	     "g" ((FG_UINT32)(bh)),					\
	     "1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("mull %3"							\
	   : "=a" ((FG_UINT32)(w0)),					\
	     "=d" ((FG_UINT32)(w1))					\
	   : "%0" ((FG_UINT32)(u)),					\
	     "rm" ((FG_UINT32)(v)))
#define udiv_qrnnd(q, r, n1, n0, d) \
  __asm__ ("divl %4"							\
	   : "=a" ((FG_UINT32)(q)),					\
	     "=d" ((FG_UINT32)(r))					\
	   : "0" ((FG_UINT32)(n0)),					\
	     "1" ((FG_UINT32)(n1)),					\
	     "rm" ((FG_UINT32)(d)))
#define count_leading_zeros(count, x) \
  do {									\
    FG_UINT32 __cbtmp;							\
    __asm__ ("bsrl %1,%0"						\
	     : "=r" (__cbtmp) : "rm" ((FG_UINT32)(x)));			\
    (count) = __cbtmp ^ 31;						\
  } while (0)
#define UMUL_TIME 40
#define UDIV_TIME 40
#endif /* 80x86 */

#if defined (__i860__)
#if 0
/* Make sure these patterns really improve the code before
   switching them on.  */
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  do {									\
    union								\
      {									\
	FG_INT64 __ll;							\
	struct {FG_UINT32 __l, __h;} __i;					\
      }  __a, __b, __s;							\
    __a.__i.__l = (al);							\
    __a.__i.__h = (ah);							\
    __b.__i.__l = (bl);							\
    __b.__i.__h = (bh);							\
    __asm__ ("fiadd.dd %1,%2,%0"					\
	     : "=f" (__s.__ll)						\
	     : "%f" (__a.__ll), "f" (__b.__ll));			\
    (sh) = __s.__i.__h;							\
    (sl) = __s.__i.__l;							\
    } while (0)
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  do {									\
    union								\
      {									\
	FG_INT64 __ll;							\
	struct {FG_UINT32 __l, __h;} __i;					\
      }  __a, __b, __s;							\
    __a.__i.__l = (al);							\
    __a.__i.__h = (ah);							\
    __b.__i.__l = (bl);							\
    __b.__i.__h = (bh);							\
    __asm__ ("fisub.dd %1,%2,%0"					\
	     : "=f" (__s.__ll)						\
	     : "%f" (__a.__ll), "f" (__b.__ll));			\
    (sh) = __s.__i.__h;							\
    (sl) = __s.__i.__l;							\
    } while (0)
#endif
#endif /* __i860__ */

#if defined (__i960__)
#define umul_ppmm(w1, w0, u, v) \
  ({union {FG_UINT64 __ll;						\
	   struct {FG_UINT32 __l, __h;} __i;				\
	  } __xx;							\
  __asm__ ("emul	%2,%1,%0"					\
	   : "=d" (__xx.__ll)						\
	   : "%dI" ((FG_UINT32)(u)),					\
	     "dI" ((FG_UINT32)(v)));					\
  (w1) = __xx.__i.__h; (w0) = __xx.__i.__l;})
#define __umulsidi3(u, v) \
  ({FG_UINT64 __w;							\
    __asm__ ("emul	%2,%1,%0"					\
	     : "=d" (__w)						\
	     : "%dI" ((FG_UINT32)(u)),					\
	       "dI" ((FG_UINT32)(v)));					\
    __w; })  
#endif /* __i960__ */

#if defined (__mc68000__)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("add%.l %5,%1
	addx%.l %3,%0"							\
	   : "=d" ((FG_UINT32)(sh)),					\
	     "=&d" ((FG_UINT32)(sl))					\
	   : "%0" ((FG_UINT32)(ah)),					\
	     "d" ((FG_UINT32)(bh)),					\
	     "%1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("sub%.l %5,%1
	subx%.l %3,%0"							\
	   : "=d" ((FG_UINT32)(sh)),					\
	     "=&d" ((FG_UINT32)(sl))					\
	   : "0" ((FG_UINT32)(ah)),					\
	     "d" ((FG_UINT32)(bh)),					\
	     "1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#if defined (__mc68020__) || defined (__NeXT__) || defined(mc68020)
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("mulu%.l %3,%1:%0"						\
	   : "=d" ((FG_UINT32)(w0)),					\
	     "=d" ((FG_UINT32)(w1))					\
	   : "%0" ((FG_UINT32)(u)),					\
	     "dmi" ((FG_UINT32)(v)))
#define UMUL_TIME 45
#define udiv_qrnnd(q, r, n1, n0, d) \
  __asm__ ("divu%.l %4,%1:%0"						\
	   : "=d" ((FG_UINT32)(q)),					\
	     "=d" ((FG_UINT32)(r))					\
	   : "0" ((FG_UINT32)(n0)),					\
	     "1" ((FG_UINT32)(n1)),					\
	     "dmi" ((FG_UINT32)(d)))
#define UDIV_TIME 90
#define sdiv_qrnnd(q, r, n1, n0, d) \
  __asm__ ("divs%.l %4,%1:%0"						\
	   : "=d" ((FG_UINT32)(q)),					\
	     "=d" ((FG_UINT32)(r))					\
	   : "0" ((FG_UINT32)(n0)),					\
	     "1" ((FG_UINT32)(n1)),					\
	     "dmi" ((FG_UINT32)(d)))
#define count_leading_zeros(count, x) \
  __asm__ ("bfffo %1{%b2:%b2},%0"					\
	   : "=d" ((FG_UINT32)(count))					\
	   : "od" ((FG_UINT32)(x)), "n" (0))
#else /* not mc68020 */
/* %/ inserts REGISTER_PREFIX, %# inserts IMMEDIATE_PREFIX.  */
#define umul_ppmm(xh, xl, a, b) \
  __asm__ ("| Inlined umul_ppmm
	move%.l	%2,%/d0
	move%.l	%3,%/d1
	move%.l	%/d0,%/d2
	swap	%/d0
	move%.l	%/d1,%/d3
	swap	%/d1
	move%.w	%/d2,%/d4
	mulu	%/d3,%/d4
	mulu	%/d1,%/d2
	mulu	%/d0,%/d3
	mulu	%/d0,%/d1
	move%.l	%/d4,%/d0
	eor%.w	%/d0,%/d0
	swap	%/d0
	add%.l	%/d0,%/d2
	add%.l	%/d3,%/d2
	jcc	1f
	add%.l	%#65536,%/d1
1:	swap	%/d2
	moveq	%#0,%/d0
	move%.w	%/d2,%/d0
	move%.w	%/d4,%/d2
	move%.l	%/d2,%1
	add%.l	%/d1,%/d0
	move%.l	%/d0,%0"						\
	   : "=g" ((FG_UINT32)(xh)),					\
	     "=g" ((FG_UINT32)(xl))					\
	   : "g" ((FG_UINT32)(a)),					\
	     "g" ((FG_UINT32)(b))						\
	   : "d0", "d1", "d2", "d3", "d4")
#define UMUL_TIME 100
#define UDIV_TIME 400
#endif /* not mc68020 */
#endif /* mc68000 */

#if defined (__m88000__)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("addu.co %1,%r4,%r5
	addu.ci %0,%r2,%r3"						\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "%rJ" ((FG_UINT32)(ah)),					\
	     "rJ" ((FG_UINT32)(bh)),					\
	     "%rJ" ((FG_UINT32)(al)),					\
	     "rJ" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("subu.co %1,%r4,%r5
	subu.ci %0,%r2,%r3"						\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "rJ" ((FG_UINT32)(ah)),					\
	     "rJ" ((FG_UINT32)(bh)),					\
	     "rJ" ((FG_UINT32)(al)),					\
	     "rJ" ((FG_UINT32)(bl)))
#define count_leading_zeros(count, x) \
  do {									\
    FG_UINT32 __cbtmp;							\
    __asm__ ("ff1 %0,%1"						\
	     : "=r" (__cbtmp)						\
	     : "r" ((FG_UINT32)(x)));					\
    (count) = __cbtmp ^ 31;						\
  } while (0)
#if defined (__mc88110__)
#define umul_ppmm(wh, wl, u, v) \
  do {									\
    union {FG_UINT64 __ll;						\
	   struct {FG_UINT32 __h, __l;} __i;				\
	  } __xx;							\
    __asm__ ("mulu.d	%0,%1,%2"					\
	     : "=r" (__xx.__ll)						\
	     : "r" ((FG_UINT32)(u)),					\
	       "r" ((FG_UINT32)(v)));					\
    (wh) = __xx.__i.__h;						\
    (wl) = __xx.__i.__l;						\
  } while (0)
#define udiv_qrnnd(q, r, n1, n0, d) \
  ({union {FG_UINT64 __ll;						\
	   struct {FG_UINT32 __h, __l;} __i;				\
	  } __xx;							\
  FG_UINT32 __q;								\
  __xx.__i.__h = (n1); __xx.__i.__l = (n0);				\
  __asm__ ("divu.d %0,%1,%2"						\
	   : "=r" (__q)							\
	   : "r" (__xx.__ll),						\
	     "r" ((FG_UINT32)(d)));					\
  (r) = (n0) - __q * (d); (q) = __q; })
#define UMUL_TIME 5
#define UDIV_TIME 25
#else
#define UMUL_TIME 17
#define UDIV_TIME 150
#endif /* __mc88110__ */
#endif /* __m88000__ */

#if defined (__mips__)
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("multu %2,%3
	mflo %0
	mfhi %1"							\
	   : "=d" ((FG_UINT32)(w0)),					\
	     "=d" ((FG_UINT32)(w1))					\
	   : "d" ((FG_UINT32)(u)),					\
	     "d" ((FG_UINT32)(v)))
#define UMUL_TIME 10
#define UDIV_TIME 100
#endif /* __mips__ */

#if defined (__ns32000__)
#define umul_ppmm(w1, w0, u, v) \
  ({union {FG_UINT64 __ll;						\
	   struct {FG_UINT32 __l, __h;} __i;				\
	  } __xx;							\
  __asm__ ("meid %2,%0"							\
	   : "=g" (__xx.__ll)						\
	   : "%0" ((FG_UINT32)(u)),					\
	     "g" ((FG_UINT32)(v)));					\
  (w1) = __xx.__i.__h; (w0) = __xx.__i.__l;})
#define __umulsidi3(u, v) \
  ({FG_UINT64 __w;							\
    __asm__ ("meid %2,%0"						\
	     : "=g" (__w)						\
	     : "%0" ((FG_UINT32)(u)),					\
	       "g" ((FG_UINT32)(v)));					\
    __w; })
#define udiv_qrnnd(q, r, n1, n0, d) \
  ({union {FG_UINT64 __ll;						\
	   struct {FG_UINT32 __l, __h;} __i;				\
	  } __xx;							\
  __xx.__i.__h = (n1); __xx.__i.__l = (n0);				\
  __asm__ ("deid %2,%0"							\
	   : "=g" (__xx.__ll)						\
	   : "0" (__xx.__ll),						\
	     "g" ((FG_UINT32)(d)));					\
  (r) = __xx.__i.__l; (q) = __xx.__i.__h; })
#endif /* __ns32000__ */

#if (defined (_ARCH_PPC) || defined (_IBMR2)) && W_TYPE_SIZE == 32
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  do {									\
    if (__builtin_constant_p (bh) && (bh) == 0)				\
      __asm__ ("{a%I4|add%I4c} %1,%3,%4\n\t{aze|addze} %0,%2"		\
	     : "=r" ((FG_UINT32)(sh)),					\
	       "=&r" ((FG_UINT32)(sl))					\
	     : "%r" ((FG_UINT32)(ah)),					\
	       "%r" ((FG_UINT32)(al)),					\
	       "rI" ((FG_UINT32)(bl)));					\
    else if (__builtin_constant_p (bh) && (bh) ==~(FG_UINT32) 0)		\
      __asm__ ("{a%I4|add%I4c} %1,%3,%4\n\t{ame|addme} %0,%2"		\
	     : "=r" ((FG_UINT32)(sh)),					\
	       "=&r" ((FG_UINT32)(sl))					\
	     : "%r" ((FG_UINT32)(ah)),					\
	       "%r" ((FG_UINT32)(al)),					\
	       "rI" ((FG_UINT32)(bl)));					\
    else								\
      __asm__ ("{a%I5|add%I5c} %1,%4,%5\n\t{ae|adde} %0,%2,%3"		\
	     : "=r" ((FG_UINT32)(sh)),					\
	       "=&r" ((FG_UINT32)(sl))					\
	     : "%r" ((FG_UINT32)(ah)),					\
	       "r" ((FG_UINT32)(bh)),					\
	       "%r" ((FG_UINT32)(al)),					\
	       "rI" ((FG_UINT32)(bl)));					\
  } while (0)
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  do {									\
    if (__builtin_constant_p (ah) && (ah) == 0)				\
      __asm__ ("{sf%I3|subf%I3c} %1,%4,%3\n\t{sfze|subfze} %0,%2"	\
	       : "=r" ((FG_UINT32)(sh)),					\
		 "=&r" ((FG_UINT32)(sl))					\
	       : "r" ((FG_UINT32)(bh)),					\
		 "rI" ((FG_UINT32)(al)),					\
		 "r" ((FG_UINT32)(bl)));					\
    else if (__builtin_constant_p (ah) && (ah) ==~(FG_UINT32) 0)		\
      __asm__ ("{sf%I3|subf%I3c} %1,%4,%3\n\t{sfme|subfme} %0,%2"	\
	       : "=r" ((FG_UINT32)(sh)),					\
		 "=&r" ((FG_UINT32)(sl))					\
	       : "r" ((FG_UINT32)(bh)),					\
		 "rI" ((FG_UINT32)(al)),					\
		 "r" ((FG_UINT32)(bl)));					\
    else if (__builtin_constant_p (bh) && (bh) == 0)			\
      __asm__ ("{sf%I3|subf%I3c} %1,%4,%3\n\t{ame|addme} %0,%2"		\
	       : "=r" ((FG_UINT32)(sh)),					\
		 "=&r" ((FG_UINT32)(sl))					\
	       : "r" ((FG_UINT32)(ah)),					\
		 "rI" ((FG_UINT32)(al)),					\
		 "r" ((FG_UINT32)(bl)));					\
    else if (__builtin_constant_p (bh) && (bh) ==~(FG_UINT32) 0)		\
      __asm__ ("{sf%I3|subf%I3c} %1,%4,%3\n\t{aze|addze} %0,%2"		\
	       : "=r" ((FG_UINT32)(sh)),					\
		 "=&r" ((FG_UINT32)(sl))					\
	       : "r" ((FG_UINT32)(ah)),					\
		 "rI" ((FG_UINT32)(al)),					\
		 "r" ((FG_UINT32)(bl)));					\
    else								\
      __asm__ ("{sf%I4|subf%I4c} %1,%5,%4\n\t{sfe|subfe} %0,%3,%2"	\
	       : "=r" ((FG_UINT32)(sh)),					\
		 "=&r" ((FG_UINT32)(sl))					\
	       : "r" ((FG_UINT32)(ah)),					\
		 "r" ((FG_UINT32)(bh)),					\
		 "rI" ((FG_UINT32)(al)),					\
		 "r" ((FG_UINT32)(bl)));					\
  } while (0)
#define count_leading_zeros(count, x) \
  __asm__ ("{cntlz|cntlzw} %0,%1"					\
	   : "=r" ((FG_UINT32)(count))					\
	   : "r" ((FG_UINT32)(x)))
#if defined (_ARCH_PPC)
#define umul_ppmm(ph, pl, m0, m1) \
  do {									\
    FG_UINT32 __m0 = (m0), __m1 = (m1);					\
    __asm__ ("mulhwu %0,%1,%2"						\
	     : "=r" ((FG_UINT32) ph)					\
	     : "%r" (__m0),						\
	       "r" (__m1));						\
    (pl) = __m0 * __m1;							\
  } while (0)
#define UMUL_TIME 15
#define smul_ppmm(ph, pl, m0, m1) \
  do {									\
    FG_INT32 __m0 = (m0), __m1 = (m1);					\
    __asm__ ("mulhw %0,%1,%2"						\
	     : "=r" ((FG_INT32) ph)					\
	     : "%r" (__m0),						\
	       "r" (__m1));						\
    (pl) = __m0 * __m1;							\
  } while (0)
#define SMUL_TIME 14
#define UDIV_TIME 120
#else
#define umul_ppmm(xh, xl, m0, m1) \
  do {									\
    FG_UINT32 __m0 = (m0), __m1 = (m1);					\
    __asm__ ("mul %0,%2,%3"						\
	     : "=r" ((FG_UINT32)(xh)),					\
	       "=q" ((FG_UINT32)(xl))					\
	     : "r" (__m0),						\
	       "r" (__m1));						\
    (xh) += ((((FG_INT32) __m0 >> 31) & __m1)				\
	     + (((FG_INT32) __m1 >> 31) & __m0));				\
  } while (0)
#define UMUL_TIME 8
#define smul_ppmm(xh, xl, m0, m1) \
  __asm__ ("mul %0,%2,%3"						\
	   : "=r" ((FG_INT32)(xh)),					\
	     "=q" ((FG_INT32)(xl))					\
	   : "r" (m0),							\
	     "r" (m1))
#define SMUL_TIME 4
#define sdiv_qrnnd(q, r, nh, nl, d) \
  __asm__ ("div %0,%2,%4"						\
	   : "=r" ((FG_INT32)(q)), "=q" ((FG_INT32)(r))			\
	   : "r" ((FG_INT32)(nh)), "1" ((FG_INT32)(nl)), "r" ((FG_INT32)(d)))
#define UDIV_TIME 100
#endif
#endif /* Power architecture variants.  */

#if defined (__pyr__)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("addw	%5,%1
	addwc	%3,%0"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "%0" ((FG_UINT32)(ah)),					\
	     "g" ((FG_UINT32)(bh)),					\
	     "%1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("subw	%5,%1
	subwb	%3,%0"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "0" ((FG_UINT32)(ah)),					\
	     "g" ((FG_UINT32)(bh)),					\
	     "1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
/* This insn doesn't work on ancient pyramids.  */
#define umul_ppmm(w1, w0, u, v) \
  ({union {								\
	FG_UINT64 __ll;							\
	struct {FG_UINT32 __h, __l;} __i;					\
     } __xx;								\
  __xx.__i.__l = u;							\
  __asm__ ("uemul %3,%0"						\
	   : "=r" (__xx.__i.__h),					\
	     "=r" (__xx.__i.__l)					\
	   : "1" (__xx.__i.__l),					\
	     "g" ((FG_UINT32)(v)));					\
  (w1) = __xx.__i.__h;							\
  (w0) = __xx.__i.__l;})
#endif /* __pyr__ */

#if defined (__ibm032__) /* RT/ROMP */
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("a %1,%5
	ae %0,%3"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "%0" ((FG_UINT32)(ah)),					\
	     "r" ((FG_UINT32)(bh)),					\
	     "%1" ((FG_UINT32)(al)),					\
	     "r" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("s %1,%5
	se %0,%3"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "0" ((FG_UINT32)(ah)),					\
	     "r" ((FG_UINT32)(bh)),					\
	     "1" ((FG_UINT32)(al)),					\
	     "r" ((FG_UINT32)(bl)))
#define umul_ppmm(ph, pl, m0, m1) \
  do {									\
    FG_UINT32 __m0 = (m0), __m1 = (m1);					\
    __asm__ (								\
       "s	r2,r2
	mts	r10,%2
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	cas	%0,r2,r0
	mfs	r10,%1"							\
	     : "=r" ((FG_UINT32)(ph)),					\
	       "=r" ((FG_UINT32)(pl))					\
	     : "%r" (__m0),						\
		"r" (__m1)						\
	     : "r2");							\
    (ph) += ((((FG_INT32) __m0 >> 31) & __m1)				\
	     + (((FG_INT32) __m1 >> 31) & __m0));				\
  } while (0)
#define UMUL_TIME 20
#define UDIV_TIME 200
#define count_leading_zeros(count, x) \
  do {									\
    if ((x) >= 0x10000)							\
      __asm__ ("clz	%0,%1"						\
	       : "=r" ((FG_UINT32)(count))				\
	       : "r" ((FG_UINT32)(x) >> 16));				\
    else								\
      {									\
	__asm__ ("clz	%0,%1"						\
		 : "=r" ((FG_UINT32)(count))				\
		 : "r" ((FG_UINT32)(x)));					\
	(count) += 16;							\
      }									\
  } while (0)
#endif

#if defined (__sparc__)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("addcc %r4,%5,%1
	addx %r2,%3,%0"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "%rJ" ((FG_UINT32)(ah)),					\
	     "rI" ((FG_UINT32)(bh)),					\
	     "%rJ" ((FG_UINT32)(al)),					\
	     "rI" ((FG_UINT32)(bl))					\
	   __CLOBBER_CC)
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("subcc %r4,%5,%1
	subx %r2,%3,%0"							\
	   : "=r" ((FG_UINT32)(sh)),					\
	     "=&r" ((FG_UINT32)(sl))					\
	   : "rJ" ((FG_UINT32)(ah)),					\
	     "rI" ((FG_UINT32)(bh)),					\
	     "rJ" ((FG_UINT32)(al)),					\
	     "rI" ((FG_UINT32)(bl))					\
	   __CLOBBER_CC)
#if defined (__sparc_v8__)
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("umul %2,%3,%1;rd %%y,%0"					\
	   : "=r" ((FG_UINT32)(w1)),					\
	     "=r" ((FG_UINT32)(w0))					\
	   : "r" ((FG_UINT32)(u)),					\
	     "r" ((FG_UINT32)(v)))
#define udiv_qrnnd(q, r, n1, n0, d) \
  __asm__ ("mov %2,%%y;nop;nop;nop;udiv %3,%4,%0;umul %0,%4,%1;sub %3,%1,%1"\
	   : "=&r" ((FG_UINT32)(q)),					\
	     "=&r" ((FG_UINT32)(r))					\
	   : "r" ((FG_UINT32)(n1)),					\
	     "r" ((FG_UINT32)(n0)),					\
	     "r" ((FG_UINT32)(d)))
#else
#if defined (__sparclite__)
/* This has hardware multiply but not divide.  It also has two additional
   instructions scan (ffs from high bit) and divscc.  */
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("umul %2,%3,%1;rd %%y,%0"					\
	   : "=r" ((FG_UINT32)(w1)),					\
	     "=r" ((FG_UINT32)(w0))					\
	   : "r" ((FG_UINT32)(u)),					\
	     "r" ((FG_UINT32)(v)))
#define udiv_qrnnd(q, r, n1, n0, d) \
  __asm__ ("! Inlined udiv_qrnnd
	wr	%%g0,%2,%%y	! Not a delayed write for sparclite
	tst	%%g0
	divscc	%3,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%%g1
	divscc	%%g1,%4,%0
	rd	%%y,%1
	bl,a 1f
	add	%1,%4,%1
1:	! End of inline udiv_qrnnd"					\
	   : "=r" ((FG_UINT32)(q)),					\
	     "=r" ((FG_UINT32)(r))					\
	   : "r" ((FG_UINT32)(n1)),					\
	     "r" ((FG_UINT32)(n0)),					\
	     "rI" ((FG_UINT32)(d))					\
	   : "%g1" __AND_CLOBBER_CC)
#define UDIV_TIME 37
#define count_leading_zeros(count, x) \
  __asm__ ("scan %1,0,%0"						\
	   : "=r" ((FG_UINT32)(x))					\
	   : "r" ((FG_UINT32)(count)))
#else
/* SPARC without integer multiplication and divide instructions.
   (i.e. at least Sun4/20,40,60,65,75,110,260,280,330,360,380,470,490) */
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("! Inlined umul_ppmm
	wr	%%g0,%2,%%y	! SPARC has 0-3 delay insn after a wr
	sra	%3,31,%%g2	! Don't move this insn
	and	%2,%%g2,%%g2	! Don't move this insn
	andcc	%%g0,0,%%g1	! Don't move this insn
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,0,%%g1
	add	%%g1,%%g2,%0
	rd	%%y,%1"							\
	   : "=r" ((FG_UINT32)(w1)),					\
	     "=r" ((FG_UINT32)(w0))					\
	   : "%rI" ((FG_UINT32)(u)),					\
	     "r" ((FG_UINT32)(v))						\
	   : "%g1", "%g2" __AND_CLOBBER_CC)
#define UMUL_TIME 39		/* 39 instructions */
/* It's quite necessary to add this much assembler for the sparc.
   The default udiv_qrnnd (in C) is more than 10 times slower!  */
#define udiv_qrnnd(q, r, n1, n0, d) \
  __asm__ ("! Inlined udiv_qrnnd
	mov	32,%%g1
	subcc	%1,%2,%%g0
1:	bcs	5f
	 addxcc %0,%0,%0	! shift n1n0 and a q-bit in lsb
	sub	%1,%2,%1	! this kills msb of n
	addx	%1,%1,%1	! so this can't give carry
	subcc	%%g1,1,%%g1
2:	bne	1b
	 subcc	%1,%2,%%g0
	bcs	3f
	 addxcc %0,%0,%0	! shift n1n0 and a q-bit in lsb
	b	3f
	 sub	%1,%2,%1	! this kills msb of n
4:	sub	%1,%2,%1
5:	addxcc	%1,%1,%1
	bcc	2b
	 subcc	%%g1,1,%%g1
! Got carry from n.  Subtract next step to cancel this carry.
	bne	4b
	 addcc	%0,%0,%0	! shift n1n0 and a 0-bit in lsb
	sub	%1,%2,%1
3:	xnor	%0,0,%0
	! End of inline udiv_qrnnd"					\
	   : "=&r" ((FG_UINT32)(q)),					\
	     "=&r" ((FG_UINT32)(r))					\
	   : "r" ((FG_UINT32)(d)),					\
	     "1" ((FG_UINT32)(n1)),					\
	     "0" ((FG_UINT32)(n0)) : "%g1" __AND_CLOBBER_CC)
#define UDIV_TIME (3+7*32)	/* 7 instructions/iteration. 32 iterations. */
#endif /* __sparclite__ */
#endif /* __sparc_v8__ */
#endif /* __sparc__ */

#if defined (__vax__)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  __asm__ ("addl2 %5,%1
	adwc %3,%0"							\
	   : "=g" ((FG_UINT32)(sh)),					\
	     "=&g" ((FG_UINT32)(sl))					\
	   : "%0" ((FG_UINT32)(ah)),					\
	     "g" ((FG_UINT32)(bh)),					\
	     "%1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  __asm__ ("subl2 %5,%1
	sbwc %3,%0"							\
	   : "=g" ((FG_UINT32)(sh)),					\
	     "=&g" ((FG_UINT32)(sl))					\
	   : "0" ((FG_UINT32)(ah)),					\
	     "g" ((FG_UINT32)(bh)),					\
	     "1" ((FG_UINT32)(al)),					\
	     "g" ((FG_UINT32)(bl)))
#define umul_ppmm(xh, xl, m0, m1) \
  do {									\
    union {								\
	FG_UINT64 __ll;							\
	struct {FG_UINT32 __l, __h;} __i;					\
      } __xx;								\
    FG_UINT32 __m0 = (m0), __m1 = (m1);					\
    __asm__ ("emul %1,%2,$0,%0"						\
	     : "=r" (__xx.__ll)						\
	     : "g" (__m0),						\
	       "g" (__m1));						\
    (xh) = __xx.__i.__h;						\
    (xl) = __xx.__i.__l;						\
    (xh) += ((((FG_INT32) __m0 >> 31) & __m1)				\
	     + (((FG_INT32) __m1 >> 31) & __m0));				\
  } while (0)
#define sdiv_qrnnd(q, r, n1, n0, d) \
  do {									\
    union {FG_INT64 __ll;							\
	   struct {FG_INT32 __l, __h;} __i;				\
	  } __xx;							\
    __xx.__i.__h = n1; __xx.__i.__l = n0;				\
    __asm__ ("ediv %3,%2,%0,%1"						\
	     : "=g" (q), "=g" (r)					\
	     : "g" (__xx.__ll), "g" (d));				\
  } while (0)
#endif /* __vax__ */

#endif /* __GNUC__ */

/* If this machine has no inline assembler, use C macros.  */

#if !defined (add_ssaaaa)
#define add_ssaaaa(sh, sl, ah, al, bh, bl) \
  do {									\
    FG_UINT32 __x;							\
    __x = (al) + (bl);							\
    (sh) = (ah) + (bh) + (__x < (al));					\
    (sl) = __x;								\
  } while (0)
#endif

#if !defined (sub_ddmmss)
#define sub_ddmmss(sh, sl, ah, al, bh, bl) \
  do {									\
    FG_UINT32 __x;							\
    __x = (al) - (bl);							\
    (sh) = (ah) - (bh) - (__x > (al));					\
    (sl) = __x;								\
  } while (0)
#endif

#if !defined (umul_ppmm)
#define umul_ppmm(w1, w0, u, v)						\
  do {									\
    FG_UINT32 __x0, __x1, __x2, __x3;					\
    FG_UINT32 __ul, __vl, __uh, __vh;					\
									\
    __ul = __ll_lowpart (u);						\
    __uh = __ll_highpart (u);						\
    __vl = __ll_lowpart (v);						\
    __vh = __ll_highpart (v);						\
									\
    __x0 = (FG_UINT32) __ul * __vl;					\
    __x1 = (FG_UINT32) __ul * __vh;					\
    __x2 = (FG_UINT32) __uh * __vl;					\
    __x3 = (FG_UINT32) __uh * __vh;					\
									\
    __x1 += __ll_highpart (__x0);/* this can't give carry */		\
    __x1 += __x2;		/* but this indeed can */		\
    if (__x1 < __x2)		/* did we get it? */			\
      __x3 += __ll_B;		/* yes, add it in the proper pos. */	\
									\
    (w1) = __x3 + __ll_highpart (__x1);					\
    (w0) = __ll_lowpart (__x1) * __ll_B + __ll_lowpart (__x0);		\
  } while (0)
#endif

#if !defined (__umulsidi3)
#define __umulsidi3(u, v) \
  ({DIunion __w;							\
    umul_ppmm (__w.s.high, __w.s.low, u, v);				\
    __w.ll; })
#endif

/* Define this unconditionally, so it can be used for debugging.  */
#define __udiv_qrnnd_c(q, r, n1, n0, d) \
  do {									\
    FG_UINT32 __d1, __d0, __q1, __q0;					\
    FG_UINT32 __r1, __r0, __m;						\
    __d1 = __ll_highpart (d);						\
    __d0 = __ll_lowpart (d);						\
									\
    __r1 = (n1) % __d1;							\
    __q1 = (n1) / __d1;							\
    __m = (FG_UINT32) __q1 * __d0;					\
    __r1 = __r1 * __ll_B | __ll_highpart (n0);				\
    if (__r1 < __m)							\
      {									\
	__q1--, __r1 += (d);						\
	if (__r1 >= (d)) /* i.e. we didn't get carry when adding to __r1 */\
	  if (__r1 < __m)						\
	    __q1--, __r1 += (d);					\
      }									\
    __r1 -= __m;							\
									\
    __r0 = __r1 % __d1;							\
    __q0 = __r1 / __d1;							\
    __m = (FG_UINT32) __q0 * __d0;					\
    __r0 = __r0 * __ll_B | __ll_lowpart (n0);				\
    if (__r0 < __m)							\
      {									\
	__q0--, __r0 += (d);						\
	if (__r0 >= (d))						\
	  if (__r0 < __m)						\
	    __q0--, __r0 += (d);					\
      }									\
    __r0 -= __m;							\
									\
    (q) = (FG_UINT32) __q1 * __ll_B | __q0;				\
    (r) = __r0;								\
  } while (0)

/* If the processor has no udiv_qrnnd but sdiv_qrnnd, go through
   __udiv_w_sdiv (defined in libgcc or elsewhere).  */
#if !defined (udiv_qrnnd) && defined (sdiv_qrnnd)
#define udiv_qrnnd(q, r, nh, nl, d) \
  do {									\
    FG_UINT32 __r;							\
    (q) = __udiv_w_sdiv (&__r, nh, nl, d);				\
    (r) = __r;								\
  } while (0)
#endif

/* If udiv_qrnnd was not defined for this processor, use __udiv_qrnnd_c.  */
#if !defined (udiv_qrnnd)
#define UDIV_NEEDS_NORMALIZATION 1
#define udiv_qrnnd __udiv_qrnnd_c
#endif

#if !defined (count_leading_zeros)
extern const FG_UINT32 __clz_tab[];
#define count_leading_zeros(count, x) \
  do {									\
    FG_UINT32 __xr = (x);							\
    FG_UINT32 __a;							\
									\
    if (SI_TYPE_SIZE <= 32)						\
      {									\
	__a = __xr < (1<<2*__BITS4)					\
	  ? (__xr < (1<<__BITS4) ? 0 : __BITS4)				\
	  : (__xr < (1<<3*__BITS4) ?  2*__BITS4 : 3*__BITS4);		\
      }									\
    else								\
      {									\
	for (__a = SI_TYPE_SIZE - 8; __a > 0; __a -= 8)		\
	  if (((__xr >> __a) & 0xff) != 0)				\
	    break;							\
      }									\
									\
    (count) = SI_TYPE_SIZE - (__clz_tab[__xr >> __a] + __a);		\
  } while (0)
#endif

#ifndef UDIV_NEEDS_NORMALIZATION
#define UDIV_NEEDS_NORMALIZATION 0
#endif

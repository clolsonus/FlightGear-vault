/** FALLOC : fallocdefs.h
**/

/* $Id: fallocdefs.h,v 1.3 90/09/05 18:05:36 dfs Exp Locker: dfs $ */


/* Define an alignment based on a type or an absolute size for any new machine
** or it will default to sizeof(double), at some small cost in space.
*/

#ifdef vax
#define ALIGN_WORST 2
#endif

#ifdef apollo
#define ALIGN_WORST 2
#endif

#ifndef ALIGN_WORST
#define ALIGN_WORST (sizeof(double))
#endif

#define ALIGN_SIZE (ALIGN_WORST - 1)
#define ALIGN_MASK (~ALIGN_SIZE)

#define MALLOC_FATAL(P,T,N,M)                                           \
        {                                                               \
           if ((P = (T *) malloc((unsigned)(N)*sizeof(T))) == NULL) {   \
             exit(fprintf(stderr, "falloc: MALLOC RETURNED NULL: %s\n", M)); \
           }                                                            \
        }

#define MALLOC_RET(P,T,N,R)                                             \
        {                                                               \
           if ((P = (T *) malloc((unsigned)(N)*sizeof(T))) == NULL) {   \
              return R;                                                 \
           }                                                            \
        }

#define REALLOC_RET(P,T,N,R)                                            \
        {                                                               \
           if ((P = (T *) realloc((char *)P,                            \
             (unsigned)(N)*sizeof(T))) == NULL) {                       \
              return R;                                                 \
           }                                                            \
        }

#define FREE(P) free((char *) P)

#define SWAP(A, B, T) ((T) = (A), (A) = (B), (B) = (T));
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define MAGIC ((int) 0xbad1)

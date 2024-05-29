#include "Vlib.h"

#if !defined(__GNUC__) || !defined(__STDC__) || defined(_NO_INLINE)

double VDotProd (a, b)
VPoint	*a, *b; {

	return a->x * b->x + a->y * b->y + a->z * b->z;
}
#endif

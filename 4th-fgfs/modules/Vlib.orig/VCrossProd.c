#include "Vlib.h"

#if !defined(__GNUC__) || !defined(__STDC__) || defined(_NO_INLINE)

void
VCrossProd (a, b, r)
VPoint	*a, *b, *r;
{
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->z * b->x - a->x * b->z;
	r->z = a->x * b->y - a->y * b->x;
}

#endif

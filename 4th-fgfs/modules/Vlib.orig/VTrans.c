#include "Vlib.h"

#if !defined(__GNUC__) || !defined(__STDC__) || defined(_NO_INLINE)

/*
 * VTransform: transform a point from one coordinate system to another.
 */

void
VTransform (pt, mt, newPt)
VPoint	*pt;
VMatrix *mt;
VPoint	*newPt;
{

  newPt->x = pt->x * mt->m[0][0] + pt->y * mt->m[0][1]
		+ pt->z * mt->m[0][2] + mt->m[0][3];

  newPt->y = pt->x * mt->m[1][0] + pt->y * mt->m[1][1]
  		+ pt->z * mt->m[1][2] + mt->m[1][3];

  newPt->z = pt->x * mt->m[2][0] + pt->y * mt->m[2][1]
  		+ pt->z * mt->m[2][2] + mt->m[2][3];
}

void
VTransform_ (pt, mt, newPt)
VPoint	*pt;
VMatrix *mt;
VPoint	*newPt;
{

  newPt->x = pt->x * mt->m[0][0] + pt->y * mt->m[0][1]
		+ pt->z * mt->m[0][2];

  newPt->y = pt->x * mt->m[1][0] + pt->y * mt->m[1][1]
  		+ pt->z * mt->m[1][2];

  newPt->z = pt->x * mt->m[2][0] + pt->y * mt->m[2][1]
  		+ pt->z * mt->m[2][2];
}

#endif


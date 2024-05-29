#include "Vlib.h"
#include <math.h>

void
VComputeClipNormals (v)
Viewport *v; {

	VPoint	*p, *q;
	int	i, max;
	double  mag;

	p = v->clipPoly->vertex;
	q = v->clipNormals;

/*
 *  We only reserved space for four clipping plane normals in clipNormals
 */

	max = (v->clipPoly->numVtces > 4) ? 4 : v->clipPoly->numVtces;

/*
 *  Compute the unit-normal vectors corresponding to each clipping plane
 */

	for (i=0; i<max; ++i) {
		mag = sqrt (p->x * p->x + p->y * p->y + p->z * p->z);
		q->x = p->x / mag;
		q->y = p->y / mag;
		q->z = p->z / mag;
		q++;
		p++;
	}

}

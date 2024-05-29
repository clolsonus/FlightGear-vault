#include "Vlib.h"

void VDrawSegments (v, seg, nseg, color)
Viewport	*v;
Segment		*seg;
int		nseg;
Color		color; {

	register ZInfo	*z;

	if (v->ztop == v->zsize) {
		printf ("Z-information pool overflow\n");
		return;
	}

	z = &(v->zpool[(v->ztop)++]);
	z->depth = --v->depth;
	z->color = color;

	for ( ; nseg > 0; -- nseg) {
		DrawLine (v->w, seg->x1, seg->y1, seg->x2, seg->y2, z);
		++ seg;
	}
}

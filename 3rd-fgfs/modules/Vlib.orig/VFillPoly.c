#include "Vlib.h"

#define COLLAPSEUNUSEDPOINTS
/*ARGSUSED*/
void VFillPolygon(v, win, gc, poly)
Viewport *v;
Window	 win;
GC	 gc;
VPolygon *poly; {

	VPoint 	 *p;
	XPoint	 xpt[VmaxVP], *lastpt;
	register int i, k;
	Drawable d;
	ZInfo	 *z;

	if (v->ztop == v->zsize) {
		fprintf (stderr, "Z-information pool overflow\n");
		return;
	}

	z = &(v->zpool[(v->ztop)++]);

	d = (v->flags & VPPixmap) ? (Drawable) v->monoPixmap : (Drawable) win;

	if (poly == (VPolygon *) NULL)
		return;

	k = 0;
	lastpt = &xpt[0];

/*
 */

	z->depth = -- v->depth;
	z->color = VComputePolygonColor(v, poly);

	for ((i=0, p=poly->vertex); i<poly->numVtces; (++i, ++p)) {

	    if (v->flags & VPPerspective && p->z != 0.0) {
		xpt[k].x = (v->Middl.x + (int) (v->Scale.x * p->x / p->z)) >> 2;
		xpt[k].y = (v->Middl.y - (int) (v->Scale.y * p->y / p->z)) >> 2;
	    }
	    else {
		xpt[k].x = (v->Middl.x + (int) (v->Scale.x * p->x)) >> 2;
		xpt[k].y = (v->Middl.y - (int) (v->Scale.y * p->y)) >> 2;
	    }

/*
 *  This hack insures that improperly 3-D clipped polygons will not
 *  induce a core dump.  This problem appears when the height is an even value.
 */

	    if (xpt[k].x >= v->width)
		xpt[k].x = v->width - 1;
	    if (xpt[k].y >= v->height)
		xpt[k].y = v->height - 1;

#ifdef COLLAPSEUNUSEDPOINTS
	    if (k == 0 || !(xpt[k].x == lastpt->x && xpt[k].y == lastpt->y))
		lastpt = &xpt[k++];
#else
	    ++k;
#endif

	}

	if (k > 0) {
#ifdef COLLAPSEUNUSEDPOINTS
		if (k == 1)
			DrawPoint (v->w, xpt[0].x, xpt[0].y, z);
		else if (k == 2)
			DrawLine (v->w, xpt[0].x, xpt[0].y,
				xpt[1].x, xpt[1].y, z);
		else
#endif
		FillPolygon (v->w, xpt, k, z);
	}

}

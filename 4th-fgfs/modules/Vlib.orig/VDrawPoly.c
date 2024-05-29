#include "Vlib.h"

void VDrawPolygon(v, win, gc, poly)
Viewport *v;
Window	 win;
GC	 gc;
VPolygon *poly; {

	VPoint 	*p;
	XPoint	xpt[VmaxVP];
	int	i;
	Drawable d;

	d = (v->flags & VPPixmap) ? (Drawable) v->monoPixmap : (Drawable) win;

	if (poly == (VPolygon *) NULL)
		return;

	for ((i=0, p=poly->vertex); i<poly->numVtces; (++i, ++p)) {
	    if (v->flags & VPPerspective) {
		xpt[i].x = (v->Middl.x + (int) (v->Scale.x * p->x / p->z)) >> 2;
		xpt[i].y = (v->Middl.y - (int) (v->Scale.y * p->y / p->z)) >> 2;
	    }
	    else {
		xpt[i].x = (v->Middl.x + (int) (v->Scale.x * p->x)) >> 2;
		xpt[i].y = (v->Middl.y - (int) (v->Scale.y * p->y)) >> 2;
	    }
	}

	if (i > 0) {
		xpt[i] = xpt[0];
		XDrawLines (v->dpy, d, gc, xpt, i+1, CoordModeOrigin);
	}

}
